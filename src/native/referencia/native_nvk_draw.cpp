// SPDX-License-Identifier: GPL-3.0-or-later

#include "native_nvk_draw.h"

#include <algorithm>
#include <array>
#include <bit>

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ui/vulkan/api.h>

#if COD5_DIAGNOSTICS
REXCVAR_DEFINE_UINT32(cod5_native_nvk_measure, 0, "COD5",
                      "Switch NVK diagnostics: measure one of every N draw-path calls; "
                      "0 disables measurement and N is rounded down to a power of two")
    .range(0, 4096)
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_nvk_measure_failures, false, "COD5",
                    "Switch NVK diagnostics: additionally touch pipeline/shader data before "
                    "measurement; this deliberately adds work")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
#endif
REXCVAR_DEFINE_BOOL(cod5_native_nvk_emission, true, "COD5",
                    "Switch NVK: emit each draw command as one identical byte sequence")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_nvk_cbufs, true, "COD5",
                    "Switch NVK: skip repeated constant-buffer work when no slot is dirty")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_nvk_dynamic, true, "COD5",
                    "Switch NVK: emit only dirty dynamic-state groups")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_nvk_preload, true, "COD5",
                    "Switch NVK: prefetch selected graphics pipelines and shaders")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_nvk_set4_delta, true, "COD5",
                    "Switch NVK: update dynamic UBO set 4 by exact differences")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

#if REX_PLATFORM_SWITCH
// These variables are exported by Mesa with unmangled C names. Keep their
// declarations outside every C++ namespace, especially an anonymous namespace:
// an anonymous-namespace variable keeps internal C++ linkage even when written
// inside an extern "C" block and therefore cannot bind Mesa's global symbol.
extern "C" {
extern cod5::native::NvkSwitchDraw nvk_switch_draw __attribute__((weak));
extern cod5::native::NvkSwitchSet4 nvk_switch_set4 __attribute__((weak));
void vk_switch_preload_pipeline(VkPipeline pipeline) __attribute__((weak));
}
#endif

namespace cod5::native {
namespace {

enum NvkImprovement : size_t {
  kEmission = 0,
  kCbufs = 1,
  kDynamic = 2,
  kSet4Fast = 3,
  kPreload = 4,
};

bool g_preload_requested = false;

template <typename T>
T AtomicLoad(const T* value) {
  return __atomic_load_n(value, __ATOMIC_RELAXED);
}

template <typename T>
void AtomicStore(T* value, T desired) {
  __atomic_store_n(value, desired, __ATOMIC_RELAXED);
}

uint32_t MeasurementInterval() {
#if COD5_DIAGNOSTICS
  const uint32_t requested = REXCVAR_GET(cod5_native_nvk_measure);
  return requested == 0 ? 0 : std::bit_floor(requested);
#else
  return 0;
#endif
}

bool MeasureFailuresRequested() {
#if COD5_DIAGNOSTICS
  return REXCVAR_GET(cod5_native_nvk_measure_failures);
#else
  return false;
#endif
}

#if REX_PLATFORM_SWITCH
NvkSwitchDraw* DrawContract() {
  NvkSwitchDraw* contract = &::nvk_switch_draw;
  return contract != nullptr && AtomicLoad(&contract->version) == 1
             ? contract
             : nullptr;
}

NvkSwitchSet4* Set4Contract() {
  NvkSwitchSet4* contract = &::nvk_switch_set4;
  return contract != nullptr && AtomicLoad(&contract->version) == 1
             ? contract
             : nullptr;
}
#else
NvkSwitchDraw* DrawContract() { return nullptr; }
NvkSwitchSet4* Set4Contract() { return nullptr; }
#endif

#if COD5_DIAGNOSTICS
void CaptureDraw(NvkSwitchDraw& output, const NvkSwitchDraw& input) {
  output.version = AtomicLoad(&input.version);
  output.measure = AtomicLoad(&input.measure);
  output.measure_failures = AtomicLoad(&input.measure_failures);
  output.environment = AtomicLoad(&input.environment);
  output.ticks_per_second = AtomicLoad(&input.ticks_per_second);
  for (size_t i = 0; i < std::size(output.parts); ++i) {
    output.parts[i].times = AtomicLoad(&input.parts[i].times);
    output.parts[i].ticks = AtomicLoad(&input.parts[i].ticks);
  }
  for (size_t i = 0; i < std::size(output.counts); ++i) {
    output.counts[i] = AtomicLoad(&input.counts[i]);
  }
  for (size_t i = 0; i < std::size(output.improvements); ++i) {
    output.improvements[i].request = AtomicLoad(&input.improvements[i].request);
    output.improvements[i].off = AtomicLoad(&input.improvements[i].off);
    output.improvements[i].uses = AtomicLoad(&input.improvements[i].uses);
    output.improvements[i].validated = AtomicLoad(&input.improvements[i].validated);
    output.improvements[i].differences = AtomicLoad(&input.improvements[i].differences);
    output.improvements[i].no_check = AtomicLoad(&input.improvements[i].no_check);
  }
}

void CaptureSet4(NvkSwitchSet4& output, const NvkSwitchSet4& input) {
  output.version = AtomicLoad(&input.version);
  output.request = AtomicLoad(&input.request);
  output.environment = AtomicLoad(&input.environment);
  output.off = AtomicLoad(&input.off);
  output.total.bindings_difference = AtomicLoad(&input.total.bindings_difference);
  output.total.complete_bindings = AtomicLoad(&input.total.complete_bindings);
  output.total.writes_root = AtomicLoad(&input.total.writes_root);
  output.total.dwords_root = AtomicLoad(&input.total.dwords_root);
  output.total.cbufs_dirty = AtomicLoad(&input.total.cbufs_dirty);
  output.total.cbufs_saved = AtomicLoad(&input.total.cbufs_saved);
  output.total.draws = AtomicLoad(&input.total.draws);
  output.total.checks = AtomicLoad(&input.total.checks);
  output.total.differences = AtomicLoad(&input.total.differences);
}

double MicrosecondsPerCall(const NvkSwitchDrawPart& part, uint64_t frequency) {
  if (part.times == 0 || frequency == 0) return 0.0;
  return double(part.ticks) * 1000000.0 /
         (double(frequency) * double(part.times));
}
#endif

}  // namespace

void ConfigureNvkDrawDriver() {
  NvkSwitchDraw* draw = DrawContract();
  NvkSwitchSet4* set4 = Set4Contract();
  const uint32_t measure = MeasurementInterval();
  const bool measure_failures = MeasureFailuresRequested();
  const std::array<int32_t, 5> requests = {
      REXCVAR_GET(cod5_native_nvk_emission) ? 1 : 0,
      REXCVAR_GET(cod5_native_nvk_cbufs) ? 1 : 0,
      REXCVAR_GET(cod5_native_nvk_dynamic) ? 1 : 0,
      0,  // NFS never enabled the separate, unaccepted set4-fast path.
      REXCVAR_GET(cod5_native_nvk_preload) ? 1 : 0,
  };
  g_preload_requested = draw != nullptr && requests[kPreload] != 0;

  if (draw != nullptr) {
    AtomicStore(&draw->measure, int32_t(measure));
    AtomicStore(&draw->measure_failures, measure_failures ? 1 : 0);
    for (size_t i = 0; i < requests.size(); ++i) {
      AtomicStore(&draw->improvements[i].request, requests[i]);
    }
  }
  if (set4 != nullptr) {
    AtomicStore(&set4->request,
                REXCVAR_GET(cod5_native_nvk_set4_delta) ? 1 : 0);
  }

#if COD5_DIAGNOSTICS
  REXLOG_INFO(
      "[nvk-draw-config] schema=1 draw_contract={} set4_contract={} measure={} "
      "measure_failures={} emission={} cbufs={} dynamic={} set4_fast=0 preload={} "
      "set4_delta={} init_only=1",
      draw != nullptr, set4 != nullptr, measure, measure_failures, requests[kEmission],
      requests[kCbufs], requests[kDynamic], requests[kPreload],
      REXCVAR_GET(cod5_native_nvk_set4_delta));
#endif
}

void PreloadNvkGraphicsPipeline(void* pipeline) {
#if REX_PLATFORM_SWITCH
  if (g_preload_requested && pipeline != nullptr &&
      vk_switch_preload_pipeline != nullptr) {
    vk_switch_preload_pipeline(static_cast<VkPipeline>(pipeline));
  }
#else
  (void)pipeline;
#endif
}

#if COD5_DIAGNOSTICS
void CaptureNvkDriverReport(NvkDriverReportSnapshot& snapshot) {
  if (NvkSwitchDraw* draw = DrawContract()) {
    snapshot.draw_contract = true;
    CaptureDraw(snapshot.draw, *draw);
  }
  if (NvkSwitchSet4* set4 = Set4Contract()) {
    snapshot.set4_contract = true;
    CaptureSet4(snapshot.set4, *set4);
  }
}

void FormatNvkDriverReport(const NvkDriverReportSnapshot& snapshot) {
  if (!snapshot.draw_contract && !snapshot.set4_contract) {
    REXLOG_INFO("[nvk-draw] schema=1 draw_contract=0 set4_contract=0");
    return;
  }
  if (snapshot.draw_contract) {
    const auto& d = snapshot.draw;
    REXLOG_INFO(
        "[nvk-draw] schema=1 environment={} measure={} failures={} hz={} "
        "draw_us={:.3f} dynamic_us={:.3f} shaders_us={:.3f} cbufs_us={:.3f} "
        "emit_us={:.3f} pipeline_copy_us={:.3f} sets_us={:.3f} root_us={:.3f} "
        "measured_draws={} dwords={} dynamic_with_work={} dynamic_bits={} "
        "shaders_dirty={} cbufs_relinked={} pipelines={} pipelines_no_effect={} "
        "sets={} root_uploads={} root_dwords={} chunks={} cumulative=1",
        d.environment, d.measure, d.measure_failures, d.ticks_per_second,
        MicrosecondsPerCall(d.parts[0], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[2], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[4], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[5], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[6], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[9], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[10], d.ticks_per_second),
        MicrosecondsPerCall(d.parts[13], d.ticks_per_second), d.counts[0],
        d.counts[1], d.counts[2], d.counts[3], d.counts[4], d.counts[5],
        d.counts[6], d.counts[7], d.counts[9], d.counts[10], d.counts[11],
        d.counts[12]);
    static constexpr const char* names[5] = {
        "emission", "cbufs", "dynamic", "set4-fast", "preload"};
    for (size_t i = 0; i < std::size(d.improvements); ++i) {
      const auto& improvement = d.improvements[i];
      REXLOG_INFO(
          "[nvk-draw-improvement] schema=1 name={} request={} off={} uses={} "
          "validated={} differences={} no_check={} cumulative=1",
          names[i], improvement.request, improvement.off, improvement.uses,
          improvement.validated, improvement.differences,
          improvement.no_check);
    }
  }
  if (snapshot.set4_contract) {
    const auto& s = snapshot.set4;
    REXLOG_INFO(
        "[nvk-set4] schema=1 request={} environment={} off={} delta_bindings={} "
        "complete_bindings={} root_writes={} root_dwords={} cbufs_dirty={} "
        "cbufs_saved={} draws={} checks={} differences={} cumulative=1",
        s.request, s.environment, s.off, s.total.bindings_difference,
        s.total.complete_bindings, s.total.writes_root, s.total.dwords_root,
        s.total.cbufs_dirty, s.total.cbufs_saved, s.total.draws,
        s.total.checks, s.total.differences);
  }
}
#endif

}  // namespace cod5::native
