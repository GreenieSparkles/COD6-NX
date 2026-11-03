// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

#include "cod6_diagnostics.h"

namespace cod6::native {

// ABI v1 exported by the pinned mesa-switch NVK patch. Keep these definitions
// independent from Mesa headers: the application links the symbols weakly and
// must also keep working with an unpatched SDK.
struct NvkSwitchSet4Counts {
  uint64_t bindings_difference;
  uint64_t complete_bindings;
  uint64_t writes_root;
  uint64_t dwords_root;
  uint64_t cbufs_dirty;
  uint64_t cbufs_saved;
  uint64_t draws;
  uint64_t checks;
  uint64_t differences;
};

struct NvkSwitchSet4 {
  int32_t version;
  int32_t request;
  int32_t environment;
  int32_t off;
  NvkSwitchSet4Counts total;
};

struct NvkSwitchDrawPart {
  uint64_t times;
  uint64_t ticks;
};

struct NvkSwitchDrawImprovement {
  int32_t request;
  int32_t off;
  uint64_t uses;
  uint64_t validated;
  uint64_t differences;
  uint64_t no_check;
};

struct NvkSwitchDraw {
  int32_t version;
  int32_t measure;
  int32_t measure_failures;
  int32_t environment;
  uint64_t ticks_per_second;
  NvkSwitchDrawPart parts[16];
  uint64_t counts[13];
  NvkSwitchDrawImprovement improvements[5];
};

static_assert(sizeof(NvkSwitchSet4) == 88,
              "NVK set4 ABI must match mesa-switch v1");
static_assert(sizeof(NvkSwitchDraw) == 584,
              "NVK draw ABI must match mesa-switch v1");

// Called before the renderer starts recording command buffers. It makes the
// formerly implicit Mesa defaults explicit and fail-closes on an ABI mismatch.
void ConfigureNvkDrawDriver();

// NFS hands a selected graphics pipeline to Mesa several microseconds before
// vkCmdBindPipeline. This is a cache prefetch only and is a no-op without the
// pinned Mesa export or when the preload candidate is disabled.
void PreloadNvkGraphicsPipeline(void* pipeline);

#if COD6_DIAGNOSTICS
struct NvkDriverReportSnapshot {
  bool draw_contract = false;
  bool set4_contract = false;
  NvkSwitchDraw draw{};
  NvkSwitchSet4 set4{};
};

// Reads every driver-owned scalar atomically. Formatting happens later on the
// report consumer, never on the renderer/ring thread.
void CaptureNvkDriverReport(NvkDriverReportSnapshot& snapshot);
void FormatNvkDriverReport(const NvkDriverReportSnapshot& snapshot);
#endif

}  // namespace cod6::native
