// SPDX-License-Identifier: GPL-3.0-or-later
//
// Native renderer for the Switch port, stage A: the graphics system.
//
// Adapted from nfsmw-nx (StevensND/nfsmw-nx, app/src/nfsmw_nativo_sistema.cpp,
// GPL-3.0): the MMIO registers, the command ring consumer, the answers the game
// waits for and the vblank thread follow its design and, in places, its code.
// Its Need for Speed specific parts (the offline shader library, the FlushState
// markers, pairing draws with Direct3D calls) are not carried over.
//
// The imported renderer consumes the ring, records Vulkan draws/resolves and
// presents the resolved frontbuffer. COD5 compatibility is still being tested.
// Initialization errors must fail setup, rather than presenting a test colour
// that could be mistaken for a working game frame. Occlusion queries are still
// approximated, and unknown packets are reported for compatibility review.

#include "cod5_native_system.h"
#include "native_renderer.h"
#include "cod5_render_hooks.h"
#include "cod5_skinning_hooks.h"
#include "cod5_lightgrid_hooks.h"
#include "cod5_wait_diagnostics.h"
#include "cod5_worker_hooks.h"
#include "cod5_handoff_diagnostics.h"
#include "cod5_main_wait.h"
#include "cod5_page_writes.h"
#include "cod5_fill_hooks.h"
#include "cod5_file_writes.h"
#include "cod5_optimization_evidence.h"
#include "native_report_queue.h"
#include "native_slow_frame_samples.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <exception>
#include <mutex>
#include <thread>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <rex/cvar.h>
#include <rex/graphics/register_file.h>
#include <rex/graphics/registers.h>
#include <rex/graphics/xenos.h>
#include <rex/kernel/xboxkrnl/video.h>
#include <rex/logging.h>
#include <rex/memory/utils.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>
#include <rex/system/xthread.h>
#include <rex/system/xvideo.h>
#include <rex/thread.h>
#include <rex/ui/presenter.h>
#include <rex/ui/vulkan/device.h>
#include <rex/ui/vulkan/presenter.h>
#include <rex/ui/vulkan/provider.h>
#include <rex/ui/windowed_app_context.h>

// project8-sw pins the ring thread to its SDK's switch_cp_core; our SDK has no such setting, so it is defined here.
REXCVAR_DEFINE_INT32(cod5_native_ring_core, -1, "COD5",
                     "Native renderer: pin the ring thread to this Switch core (0-2; -1 = let the system choose)");
REXCVAR_DEFINE_BOOL(cod5_native_ring_ready_fast_path, true, "COD5",
                    "Bypass the ring wait mutex when its acquire-loaded wake predicate is already true; "
                    "idle waits retain the original locked recheck and notification protocol")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod5_native_diagnostics, false, "COD5",
                    "Bounded frame-supply, interrupt and depth/shadow state diagnostics");
REXCVAR_DEFINE_BOOL(cod5_auto_cpu_samples, false, "COD5",
                    "Automatic 20 ms CPU stack samples; diagnostic timing is affected")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

extern "C" bool Cod5AutomaticCpuSamplesEnabled(void) {
  return cod5::diagnostics::kEnabled && REXCVAR_GET(cod5_auto_cpu_samples);
}

// The game starts its next frame as soon as the swap-complete interrupt of the
// previous one arrives (its frame limiter waits for a counter that interrupt
// bumps). With the native renderer that interrupt came back immediately, so
// frames were paced only by how long they took: 16.7, 33 or 50 ms in turn,
// which feels choppy even at a good average. Holding the interrupt to vblank
// boundaries, at least this many vblanks after the previous one, gives the
// even cadence a console gets from vsync; 2 is a steady 30.
REXCVAR_DEFINE_BOOL(cod5_pace_before_present, false, "COD5",
                    "Hold a frame for the frame rate cap before presenting it rather than "
                    "after, so frames go on screen evenly spaced");
REXCVAR_DEFINE_UINT32(cod5_min_frame_vblanks, 3, "COD5",
                      "Native renderer: minimum 60 Hz vblanks between frames (1 = the game's "
                      "own pacing, 2 = an even 30 fps)");

// cod5-nx debug: every N presented frames, the guest output as the presenter has it is written to
// <cod5_debug_capture_dir>/frame_<swap>.rgbx (8-byte header: width, height, little endian; then R8G8B8X8 rows).
// tools/rgbx_to_png.py converts them. Off (0) by default: each capture waits for the GPU.
REXCVAR_DEFINE_UINT32(cod5_debug_capture_every, 0, "COD5",
                      "Debug: save every Nth presented frame to cod5_debug_capture_dir (0 = off)");
REXCVAR_DEFINE_STRING(cod5_debug_capture_dir, "", "COD5", "Debug: folder for cod5_debug_capture_every");

REXCVAR_DEFINE_STRING(cod5_renderer, "xenos", "COD5",
                      "xenos = the SDK's emulated Xbox 360 GPU; native = the native renderer "
                      "(in development: COD5 rendering correctness is unverified)")
    .allowed({"xenos", "native"})
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

#if defined(__SWITCH__)
extern "C" {
#include <switch/kernel/svc.h>
}
extern "C" void RexGmSetReadOnlyMapped(bool enabled);
// The frame counter the console FPS overlay and the profiler read (the
// emulated path counts its swaps there too).
extern "C" void RexSwitchPerfCount(unsigned id);
#endif

namespace cod5::native {
namespace {

namespace xenos = rex::graphics::xenos;
using rex::X_STATUS;
using Clock = std::chrono::steady_clock;

// Sleeps until at least the deadline (project8-sw added rex::thread::SleepUntil to its SDK for this; ours lacks it).
void SleepUntil(Clock::time_point deadline) {
  const auto now = Clock::now();
  if (deadline > now) {
    rex::thread::Sleep(std::chrono::duration_cast<std::chrono::microseconds>(deadline - now));
  }
}

constexpr uint32_t kMmioBase = 0x7FC80000;
constexpr uint32_t kMmioMask = 0xFFFF0000;
constexpr uint32_t kMmioSize = 0x0000FFFF;
constexpr uint32_t kRegisterCount = 0x5003;  // RegisterFile::kRegisterCount

// Registers with a fixed value or an effect, as in the SDK's graphics_system.cpp.
constexpr uint32_t kRegCpRbWptr = 0x01C5;
constexpr uint32_t kRegRbEdramTiming = 0x0F00;
constexpr uint32_t kRegRbBcControl = 0x0F01;
constexpr uint32_t kRegD1ModeVCounter = 0x194C;
constexpr uint32_t kRegD1ModeVblankVlineStatus = 0x1951;
constexpr uint32_t kRegD1ModeViewportSize = 0x1961;

constexpr uint32_t kFakeOcclusionSamples = 1000;  // query_occlusion_fake_sample_count
constexpr uint16_t kMaxExtent = 2048 >> 3;        // xenos::kTexture2DCubeMaxWidthHeight >> 3
constexpr int kMaxIndirectDepth = 4;
// Without a GPU some waits can never be satisfied; they are cut short so the
// ring cannot hang.
constexpr auto kWaitRegMemMax = std::chrono::milliseconds(200);
constexpr uint32_t kOutputWidth = 1280;
constexpr uint32_t kOutputHeight = 720;

std::atomic<uint64_t> g_swaps_presented{0};
std::atomic<uint32_t> g_last_frame_draws{0};

// Big-endian words of a command buffer. In the ring the position wraps around
// (its size is a power of two); in an indirect buffer it does not.
struct Reader {
  const uint8_t* base = nullptr;
  uint32_t mask = 0;  // words - 1 in the ring; 0 in a linear buffer
  uint32_t pos = 0;
  uint32_t end = 0;

  uint32_t Pending() const { return mask ? ((end - pos) & mask) : (end - pos); }
  uint32_t Peek() const { return rex::memory::load_and_swap<uint32_t>(base + size_t(pos) * 4); }
  uint32_t Read() {
    const uint32_t value = Peek();
    Advance(1);
    return value;
  }
  void Advance(uint32_t words) { pos = mask ? ((pos + words) & mask) : (pos + words); }
};

#if COD5_DIAGNOSTICS
struct NativeReportSnapshot {
  struct InterruptInterval {
    uint64_t count = 0, ns = 0, maximum = 0;
  };
  struct SlowFrame {
    uint64_t frame_ns = 0, game_wait_ns = 0, ring_idle_ns = 0, hold_ns = 0;
    uint64_t fence_wait_ns = 0, stream_wait_ns = 0, upload_wait_ns = 0;
    uint64_t drain_wait_ns = 0, texture_ns = 0;
    uint64_t pipelines = 0, shaders = 0, textures = 0, created = 0, draws = 0;
  };
  static constexpr size_t kSlowFrameCapacity = 4;
  uint64_t sample = 0;
  uint64_t interval_ns = 0;
  uint64_t dropped_reports = 0;
  uint64_t report_calls = 0, report_ns = 0, report_max_ns = 0;
  uint64_t packets = 0, draws = 0, copies = 0, swaps = 0, waits_cut_short = 0;
  uint64_t waits = 0, wait_ns = 0, ring_idle_ns = 0, hold_ns = 0;
  uint64_t ring_ready_bypasses = 0, ring_cv_paths = 0;
  uint64_t indirect_limit_skips = 0, wptr_updates = 0;
  bool renderer_ready = false, diagnostics = false, ring_ready_fast_path = false;
  bool writes_enabled = false, writes_healthy = false;
  uint64_t active_write_pages = 0;
  RendererStats renderer;
  RendererReportSnapshot renderer_report;
  cod5::game::DrawSurfCounters drawsurfs;
  cod5::skinning::Counters skinning;
  cod5::lightgrid::Counters lightgrid;
  cod5::game::LodCounters lod;
  cod5::threading::ZeroWaitCounters zero_wait;
  cod5::workers::Counters workers;
  cod5::fill::Counters fill;
  cod5::writes::FileCounters file_writes;
  InterruptInterval interrupts[2];
  SlowFrame slow_frames[kSlowFrameCapacity];
  uint32_t slow_frame_count = 0;
};
static_assert(std::is_trivially_copyable_v<NativeReportSnapshot>);
#endif

bool Compare(uint32_t info, uint32_t value, uint32_t reference) {
  switch (info & 0x7) {
    case 0x0:
      return false;
    case 0x1:
      return value < reference;
    case 0x2:
      return value <= reference;
    case 0x3:
      return value == reference;
    case 0x4:
      return value != reference;
    case 0x5:
      return value >= reference;
    case 0x6:
      return value > reference;
    default:
      return true;
  }
}

class NativeGraphicsSystem final : public rex::system::IGraphicsSystem {
 public:
  NativeGraphicsSystem()
      : register_file_(std::make_unique<rex::graphics::RegisterFile>()),
        registers_(register_file_->values) {
#if defined(__SWITCH__)
    // Nothing watches guest pages for writes without the SDK's shared memory,
    // so the pages the game makes read-only can stay mapped. Otherwise every
    // read of them is an emulated fault: 200,000 a second at the menu, which
    // held it at 8 fps. The emulated path gets this from SharedMemory.
    RexGmSetReadOnlyMapped(true);
#endif
    std::memset(registers_, 0, sizeof(register_file_->values));
    // The linear 256-entry gamma ramp the SDK starts with.
    for (uint32_t i = 0; i < 256; ++i) {
      uint32_t value = i * 0x3FF / 0xFF;
      gamma_ramp_[i] = value | (value << 10) | (value << 20);
    }
  }
  ~NativeGraphicsSystem() override { Shutdown(); }

  X_STATUS SetupPresentation(rex::ui::WindowedAppContext* app_context) override {
    if (presenter_) {
      return X_STATUS_SUCCESS;
    }
    app_context_ = app_context;
    if (!provider_) {
      provider_ = rex::ui::vulkan::VulkanProvider::Create(true, true);
      if (!provider_) {
        REXLOG_ERROR("[native] Could not create the Vulkan device");
        return X_STATUS_UNSUCCESSFUL;
      }
    }
    auto create = [this]() { presenter_ = provider_->CreatePresenter(); };
    if (app_context_) {
      app_context_->CallInUIThreadSynchronous(create);
    } else {
      create();
    }
    if (!presenter_) {
      REXLOG_ERROR("[native] Could not create the presenter");
      return X_STATUS_UNSUCCESSFUL;
    }
    REXLOG_INFO("[native] COD5 native Vulkan selected: command-ring renderer, Xenos backend not created");
    return X_STATUS_SUCCESS;
  }

  X_STATUS SetupGuestGpu(rex::runtime::FunctionDispatcher* dispatcher,
                         rex::system::KernelState* kernel_state) override {
    dispatcher_ = dispatcher;
    kernel_state_ = kernel_state;
    memory_ = dispatcher->memory();
    diagnostics_ = cod5::diagnostics::kEnabled && REXCVAR_GET(cod5_native_diagnostics);
    REXLOG_INFO("[native-diag] bounded frame/depth diagnostics: {}", diagnostics_ ? "enabled" : "disabled");
    if (!provider_ || !presenter_) {
      REXLOG_ERROR("[native] Presentation must be initialized before the native GPU");
      return X_STATUS_UNSUCCESSFUL;
    }
    if (!renderer_ready_ && provider_) {
      renderer_ready_ = renderer_.Initialize(provider_.get(), memory_, register_file_.get());
      if (!renderer_ready_) {
        REXLOG_ERROR("[native] Native Vulkan renderer initialization failed; guest launch aborted");
        return X_STATUS_UNSUCCESSFUL;
      }
    }
    if (!mmio_registered_) {
      mmio_registered_ = memory_->AddVirtualMappedRange(kMmioBase, kMmioMask, kMmioSize, this,
                                                        &ReadMmioThunk, &WriteMmioThunk);
      if (!mmio_registered_) {
        REXLOG_ERROR("[native] Could not register the GPU MMIO range");
        return X_STATUS_UNSUCCESSFUL;
      }
    }
    if (running_.exchange(true)) {
      return X_STATUS_SUCCESS;
    }
#if COD5_DIAGNOSTICS
    report_stop_.store(false, std::memory_order_release);
    report_thread_ = std::thread([this]() { ReportLoop(); });
#endif
    vblank_thread_ = rex::system::object_ref<rex::system::XHostThread>(
        new rex::system::XHostThread(kernel_state_, 128 * 1024, 0, [this]() { return VblankLoop(); }));
    vblank_thread_->set_name("GPU VSync native");
    vblank_thread_->Create();
    ring_thread_ = rex::system::object_ref<rex::system::XHostThread>(
        new rex::system::XHostThread(kernel_state_, 128 * 1024, 0, [this]() { return RingLoop(); }));
    ring_thread_->set_name("GPU ring native");
    ring_thread_->Create();
    return X_STATUS_SUCCESS;
  }

  bool has_presentation() const override { return presenter_ != nullptr; }
  rex::ui::GraphicsProvider* provider() const override { return provider_.get(); }
  rex::ui::Presenter* presenter() const override { return presenter_.get(); }

  void SetInterruptCallback(uint32_t callback, uint32_t user_data) override {
    callback_data_.store(user_data, std::memory_order_release);
    callback_.store(callback, std::memory_order_release);
    REXLOG_INFO("[native] Interrupt callback {:08X} ({:08X})", callback, user_data);
  }

  void InitializeRingBuffer(uint32_t ptr, uint32_t size_log2) override {
    // As CommandProcessor::InitializeRingBuffer: (1 << (size_log2 + 3)) bytes.
    ring_words_.store(uint32_t(1) << (size_log2 + 1), std::memory_order_release);
    ring_base_.store(ptr, std::memory_order_release);
    ring_generation_.fetch_add(1, std::memory_order_acq_rel);
    REXLOG_INFO("[native] Ring at {:08X}, {} words", ptr, uint32_t(1) << (size_log2 + 1));
  }

  void EnableReadPointerWriteBack(uint32_t ptr, uint32_t block_size_log2) override {
    (void)block_size_log2;
    read_pointer_writeback_.store(ptr, std::memory_order_release);
  }

  void InitializeShaderStorage(const std::filesystem::path& cache_root, uint32_t title_id,
                               bool blocking) override {
    (void)blocking;
    if (renderer_ready_) {
      renderer_.InitializeDriverCache(cache_root, title_id);
    }
  }

  void Shutdown() override {
    if (running_.exchange(false)) {
      {
        std::lock_guard<std::mutex> lock(ring_mutex_);
      }
      ring_cv_.notify_all();
      if (ring_thread_) {
        ring_thread_->Wait(0, 0, 0, nullptr);
        ring_thread_.reset();
      }
      if (vblank_thread_) {
        vblank_thread_->Wait(0, 0, 0, nullptr);
        vblank_thread_.reset();
      }
    }
#if COD5_DIAGNOSTICS
    if (report_thread_.joinable()) {
      report_stop_.store(true, std::memory_order_release);
      report_cv_.notify_one();
      report_thread_.join();
    }
#endif
    if (renderer_ready_) {
      renderer_.Shutdown();
      renderer_ready_ = false;
    }
    DestroyVulkan();
    if (presenter_) {
      if (app_context_) {
        app_context_->CallInUIThreadSynchronous([this]() { presenter_.reset(); });
      }
      presenter_.reset();
    }
    provider_.reset();
  }

 private:
  using OutputContext =
      rex::ui::vulkan::VulkanPresenter::VulkanGuestOutputRefreshContext;

  // --- MMIO registers ----------------------------------------------------------

  static uint32_t ReadMmioThunk(void* ppc_context, void* context, uint32_t address) {
    (void)ppc_context;
    return static_cast<NativeGraphicsSystem*>(context)->ReadMmio(address);
  }

  static void WriteMmioThunk(void* ppc_context, void* context, uint32_t address, uint32_t value) {
    (void)ppc_context;
    static_cast<NativeGraphicsSystem*>(context)->WriteMmio(address, value);
  }

  uint32_t ReadMmio(uint32_t address) {
    const uint32_t index = (address & 0xFFFF) / 4;
    switch (index) {
      case kRegRbEdramTiming:
        return 0x08100748;
      case kRegRbBcControl:
        return 0x0000200E;
      case kRegD1ModeVCounter: {
        rex::system::X_VIDEO_MODE mode;
        rex::kernel::xboxkrnl::VdQueryVideoMode(&mode);
        return std::min(uint32_t(mode.display_height), uint32_t(0x0FFF));
      }
      case kRegD1ModeVblankVlineStatus:
        return 1;  // in vblank
      case kRegD1ModeViewportSize: {
        rex::system::X_VIDEO_MODE mode;
        rex::kernel::xboxkrnl::VdQueryVideoMode(&mode);
        const uint32_t width = std::min(uint32_t(mode.display_width), uint32_t(0x0FFF));
        const uint32_t height = std::min(uint32_t(mode.display_height), uint32_t(0x0FFF));
        return (width << 16) | height;
      }
      default:
        break;
    }
    return Register(index);
  }

  void WriteMmio(uint32_t address, uint32_t value) {
    const uint32_t index = (address & 0xFFFF) / 4;
    if (index == kRegCpRbWptr) {
      if (diagnostics_) wptr_writes_.fetch_add(1, std::memory_order_relaxed);
      write_pointer_.store(value, std::memory_order_release);
      // Taking the lock for an instant keeps the wakeup from being lost when
      // the ring thread is between checking and going to sleep.
      {
        std::lock_guard<std::mutex> lock(ring_mutex_);
      }
      ring_cv_.notify_one();
    }
    // No side effects through MMIO (graphics_system.cpp).
    if (index < kRegisterCount && registers_[index] != value) {
      const uint32_t old_value = registers_[index];
      registers_[index] = value;
      renderer_.RegisterWritten(index, old_value, value);
    }
  }

  uint32_t Register(uint32_t index) const {
    return index < kRegisterCount ? registers_[index] : 0;
  }

  // A register write from the ring, with the side effects it has when the GPU
  // processes it (CommandProcessor::WriteRegister). The Direct3D writes a
  // SCRATCH_REG and then waits with WAIT_REG_MEM until it appears at
  // SCRATCH_ADDR, so those are copied to memory; COHER_STATUS_HOST stays
  // pending until polled.
  // Consecutive registers from the stream. Blocks of shader constants, most of
  // the register traffic (every draw sets its matrices), skip the per-register
  // call and switch: a changed one only bumps a counter in the renderer.
  void WriteRegisters(uint32_t index, uint32_t count, Reader& data) {
    if (index >= 0x4000 && index + count <= 0x4928) {
      bool vs_float = false, ps_float = false, bool_loop = false;
      uint32_t fetch_slots = 0;
      for (uint32_t i = 0; i < count; ++i) {
        const uint32_t r = index + i;
        const uint32_t v = data.Read();
        if (registers_[r] == v) continue;
        registers_[r] = v;
        if (r < 0x4400) {
          vs_float = true;
        } else if (r < 0x4800) {
          ps_float = true;
        } else if (r < 0x48C0) {
          fetch_slots |= 1u << ((r - 0x4800) / 6);
        } else {
          bool_loop = true;
        }
      }
      renderer_.ConstantsWritten(vs_float, ps_float, fetch_slots, bool_loop);
      return;
    }
    for (uint32_t i = 0; i < count; ++i) {
      WriteRegister(index + i, data.Read());
    }
  }

  void WriteRegister(uint32_t index, uint32_t value) {
    if (index >= kRegisterCount) {
      return;
    }
    if (index == rex::graphics::XE_GPU_REG_COHER_STATUS_HOST) {
      value |= UINT32_C(0x80000000);
    }
    // Caches in the renderer depend on register values, and the game rewrites
    // most state with the same value before every draw: only a change counts.
    if (registers_[index] != value) {
      const uint32_t old_value = registers_[index];
      registers_[index] = value;
      renderer_.RegisterWritten(index, old_value, value);
    }
    switch (index) {
      case rex::graphics::XE_GPU_REG_DC_LUT_RW_INDEX:
        gamma_ramp_component_ = 0;
        return;
      case rex::graphics::XE_GPU_REG_DC_LUT_SEQ_COLOR: {
        // CommandProcessor::WriteRegister: red, green, blue in sequence, the
        // write enable mask in blue, green, red order.
        uint32_t entry = registers_[rex::graphics::XE_GPU_REG_DC_LUT_RW_INDEX] & 0xFF;
        if (registers_[rex::graphics::XE_GPU_REG_DC_LUT_WRITE_EN_MASK] &
            (1u << (2 - gamma_ramp_component_))) {
          uint32_t color = (value & 0xFFFF) >> 6;
          uint32_t shift = 20 - 10 * gamma_ramp_component_;  // red is bits 20:29
          gamma_ramp_[entry] = (gamma_ramp_[entry] & ~(0x3FFu << shift)) | (color << shift);
        }
        if (++gamma_ramp_component_ >= 3) {
          gamma_ramp_component_ = 0;
          registers_[rex::graphics::XE_GPU_REG_DC_LUT_RW_INDEX] =
              (registers_[rex::graphics::XE_GPU_REG_DC_LUT_RW_INDEX] & ~0xFFu) |
              ((entry + 1) & 0xFF);
        }
        renderer_.SetGammaRamp256(gamma_ramp_);
        return;
      }
      case rex::graphics::XE_GPU_REG_DC_LUT_30_COLOR: {
        uint32_t entry = registers_[rex::graphics::XE_GPU_REG_DC_LUT_RW_INDEX] & 0xFF;
        uint32_t enable = registers_[rex::graphics::XE_GPU_REG_DC_LUT_WRITE_EN_MASK] & 0b111;
        uint32_t mask = ((enable & 0b001) ? 0x3FFu : 0) | ((enable & 0b010) ? 0x3FFu << 10 : 0) |
                        ((enable & 0b100) ? 0x3FFu << 20 : 0);
        gamma_ramp_[entry] = (gamma_ramp_[entry] & ~mask) | (value & mask);
        renderer_.SetGammaRamp256(gamma_ramp_);
        return;
      }
      default:
        break;
    }
    if (index >= rex::graphics::XE_GPU_REG_SCRATCH_REG0 &&
        index <= rex::graphics::XE_GPU_REG_SCRATCH_REG7) {
      const uint32_t n = index - rex::graphics::XE_GPU_REG_SCRATCH_REG0;
      if ((UINT32_C(1) << n) & registers_[rex::graphics::XE_GPU_REG_SCRATCH_UMSK]) {
        const uint32_t address = registers_[rex::graphics::XE_GPU_REG_SCRATCH_ADDR] + n * 4;
        rex::memory::store_and_swap<uint32_t>(memory_->TranslatePhysical(address), value);
      }
    }
  }

  void WriteMemory(uint32_t address, uint32_t value) {
    const uint32_t swapped = xenos::GpuSwap(value, static_cast<xenos::Endian>(address & 0x3));
    std::memcpy(memory_->TranslatePhysical(address & ~uint32_t(0x3)), &swapped, sizeof(swapped));
  }

  uint32_t ReadMemory(uint32_t address) {
    uint32_t value;
    std::memcpy(&value, memory_->TranslatePhysical(address & ~uint32_t(0x3)), sizeof(value));
    return xenos::GpuSwap(value, static_cast<xenos::Endian>(address & 0x3));
  }

  // --- Threads -------------------------------------------------------------------

  void Interrupt(uint32_t source, uint32_t cpu) {
    const uint32_t callback = callback_.load(std::memory_order_acquire);
    if (!callback || !dispatcher_) {
      return;
    }
    auto* thread = rex::system::XThread::GetCurrentThread();
    if (!thread) {
      return;
    }
    thread->SetActiveCpu(uint8_t(cpu));
    uint64_t args[] = {source, callback_data_.load(std::memory_order_acquire)};
    const auto started = diagnostics_ ? Clock::now() : Clock::time_point{};
    dispatcher_->ExecuteInterrupt(thread->thread_state(), callback, args, 2);
    // The fixed COD5 pending counter may advance immediately in the command
    // callback or later in the vblank callback. Notify only after guest code
    // returns, when its internal spinlock has been released. This does not
    // claim that the physical GPU completed and does not alter guest state.
    cod5::main_wait::Notify(memory_->virtual_membase(),
                            cod5::main_wait::Reason::RingProgress);
    if (diagnostics_ && source < interrupt_timings_.size()) {
      const uint64_t ns = uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                      Clock::now() - started).count());
      auto& timing = interrupt_timings_[source];
      timing.count.fetch_add(1, std::memory_order_relaxed);
      timing.ns.fetch_add(ns, std::memory_order_relaxed);
      uint64_t maximum = timing.maximum.load(std::memory_order_relaxed);
      while (ns > maximum && !timing.maximum.compare_exchange_weak(
                                 maximum, ns, std::memory_order_relaxed)) {}
    }
  }

  int VblankLoop() {
    rex::system::X_VIDEO_MODE mode;
    rex::kernel::xboxkrnl::VdQueryVideoMode(&mode);
    const double hz = std::max(1.0, double(float(mode.refresh_rate)));
    const auto interval =
        std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / hz));
    // The SDK chooses this host thread's priority at creation. Do not raise
    // it here: the callback executes guest code and may wait on guest locks.
    auto next = Clock::now() + interval;
    while (running_.load(std::memory_order_acquire)) {
      // To the deadline, not in 1 ms steps: those made every vblank up to a
      // millisecond or more late.
      SleepUntil(next);
      const auto now = Clock::now();
      // After a long pause (loading, a debugger) hundreds of vblanks are not
      // replayed.
      if (now - next > std::chrono::milliseconds(250)) {
        next = now;
      }
      while (now >= next) {
        vblanks_.fetch_add(1, std::memory_order_release);
        counter_.fetch_add(1, std::memory_order_relaxed);
        Interrupt(0, 2);
        next += interval;
      }
    }
    return 0;
  }

  int RingLoop() {
    if(renderer_ready_)renderer_.AdoptRecorderProducer();
#if defined(__SWITCH__)
    // The same core the emulated path's command processor is pinned to: on a
    // core of its own it is not migrated around the game's threads.
    {
      const int32_t core = REXCVAR_GET(cod5_native_ring_core);
      if (core >= 0 && core <= 2) {
        const Result result = svcSetThreadCoreMask(CUR_THREAD_HANDLE, core, 1u << core);
        REXLOG_INFO("[native] Ring thread on core {}: result {:08X}", core, uint32_t(result));
      }
    }
#endif
    uint32_t read = 0;
    uint32_t generation = 0;
    ring_ready_fast_path_ = REXCVAR_GET(cod5_native_ring_ready_fast_path);
    REXLOG_INFO("[ring-wait] schema=1 enabled={} point=NativeGraphicsSystem::RingLoop "
                "original_cv_fallback=1 wait_bound_us=4000", ring_ready_fast_path_);
    const auto wake_requested = [&] {
      return !running_.load(std::memory_order_acquire) ||
             write_pointer_.load(std::memory_order_acquire) != read ||
             ring_generation_.load(std::memory_order_acquire) != generation;
    };
    auto last_report = Clock::now();
#if COD5_DIAGNOSTICS
    report_capture_baseline_ = last_report;
#endif
    while (running_.load(std::memory_order_acquire)) {
      if (renderer_ready_) renderer_.KickRecorder();
      if (ring_ready_fast_path_ && wake_requested()) {
        // No sleeping on this path. The release-published pointer remains the
        // synchronization source; the producer's mutex/notify is unchanged.
#if COD5_DIAGNOSTICS
        if (diagnostics_) ++ring_ready_bypasses_;
#endif
      } else {
#if COD5_DIAGNOSTICS
        if (diagnostics_) ++ring_cv_paths_;
#endif
        const auto idle_start = Clock::now();
        std::unique_lock<std::mutex> lock(ring_mutex_);
        // Recheck under the original mutex even if the unlocked probe was
        // false: publication between that probe and this lock must not be lost.
        ring_cv_.wait_for(lock, std::chrono::milliseconds(4), wake_requested);
        ring_idle_ns_ += uint64_t(
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - idle_start)
                .count());
      }
      if (!running_.load(std::memory_order_acquire)) {
        break;
      }
      if (Clock::now() - last_report > std::chrono::seconds(10)) {
        last_report = Clock::now();
#if COD5_DIAGNOSTICS
        const auto report_started = diagnostics_ ? Clock::now() : Clock::time_point{};
#endif
        Report();
#if COD5_DIAGNOSTICS
        if (diagnostics_) {
          const uint64_t report_ns = uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                  Clock::now() - report_started)
                                                  .count());
          ++report_calls_;
          report_ns_ += report_ns;
          report_max_ns_ = std::max(report_max_ns_, report_ns);
        }
#endif
      }
      const uint32_t gen = ring_generation_.load(std::memory_order_acquire);
      if (gen != generation) {
        generation = gen;
        read = 0;  // InitializeRingBuffer leaves the read pointer at zero
      }
      const uint32_t base = ring_base_.load(std::memory_order_acquire);
      const uint32_t words = ring_words_.load(std::memory_order_acquire);
      if (!base || !words) {
        continue;
      }
      const uint32_t written = write_pointer_.load(std::memory_order_acquire) & (words - 1);
      if (written == read) {
        continue;
      }
      Reader reader;
      reader.base = memory_->TranslatePhysical(base);
      reader.mask = words - 1;
      reader.pos = read;
      reader.end = written;
      while (reader.Pending()) {
        if (!Packet(reader, 0)) {
          // A partial packet: the rest arrives with the next CP_RB_WPTR.
          break;
        }
      }
      read = reader.pos;
      const uint32_t writeback = read_pointer_writeback_.load(std::memory_order_acquire);
      if (writeback) {
        rex::memory::store_and_swap<uint32_t>(memory_->TranslatePhysical(writeback), read);
      }
    }
    return 0;
  }

  void Report() {
#if COD5_DIAGNOSTICS
    NativeReportSnapshot* snapshot = report_queue_.TryReserve();
    if (!snapshot) return;
    const auto captured_at = Clock::now();
    snapshot->interval_ns = uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                         captured_at - report_capture_baseline_).count());
    snapshot->report_calls = report_calls_;
    snapshot->report_ns = report_ns_;
    snapshot->report_max_ns = report_max_ns_;
    snapshot->dropped_reports = report_queue_.dropped();
    snapshot->packets = packets_;
    snapshot->draws = draws_;
    snapshot->copies = copies_;
    snapshot->swaps = swaps_;
    snapshot->waits_cut_short = waits_cut_short_;
    snapshot->waits = waits_;
    snapshot->wait_ns = wait_ns_;
    snapshot->ring_idle_ns = ring_idle_ns_;
    snapshot->hold_ns = hold_ns_;
    snapshot->ring_ready_bypasses = ring_ready_bypasses_;
    snapshot->ring_cv_paths = ring_cv_paths_;
    snapshot->ring_ready_fast_path = ring_ready_fast_path_;
    snapshot->indirect_limit_skips = indirect_limit_skips_;
    snapshot->renderer_ready = renderer_ready_;
    snapshot->diagnostics = diagnostics_;
    snapshot->drawsurfs = cod5::game::ReadDrawSurfCounters();
    snapshot->skinning = cod5::skinning::ReadCounters();
    snapshot->lightgrid = cod5::lightgrid::ReadCounters();
    snapshot->lod = cod5::game::ReadLodCounters();
    snapshot->workers = cod5::workers::ReadCounters();
    snapshot->fill = cod5::fill::ReadCounters();
    snapshot->file_writes = cod5::writes::ReadFileCounters();
#if defined(COD5_SWITCH_ZERO_WAIT_OVERLAY)
    snapshot->zero_wait = cod5::threading::ReadZeroWaitCounters();
#else
    snapshot->zero_wait = {};
#endif
    snapshot->writes_enabled = cod5::writes::Enabled();
    snapshot->writes_healthy = cod5::writes::g_healthy.load(std::memory_order_relaxed);
    snapshot->active_write_pages = cod5::writes::g_pages.Active();
    if (renderer_ready_) {
      snapshot->sample = ++evidence_sample_;
      snapshot->renderer = renderer_.stats();
      renderer_.CaptureReportSnapshot(snapshot->renderer_report);
    } else {
      snapshot->sample = evidence_sample_;
      snapshot->renderer = {};
    }
    snapshot->slow_frame_count = slow_frame_samples_.count();
    for (uint32_t i = 0; i < slow_frame_samples_.count(); ++i) {
      snapshot->slow_frames[i] = slow_frame_samples_[i];
    }
    slow_frame_samples_.Reset();
    if (diagnostics_) {
      snapshot->wptr_updates = wptr_writes_.exchange(0, std::memory_order_relaxed);
      for (size_t source = 0; source < interrupt_timings_.size(); ++source) {
        auto& timing = interrupt_timings_[source];
        snapshot->interrupts[source].count = timing.count.exchange(0, std::memory_order_relaxed);
        snapshot->interrupts[source].ns = timing.ns.exchange(0, std::memory_order_relaxed);
        snapshot->interrupts[source].maximum =
            timing.maximum.exchange(0, std::memory_order_relaxed);
      }
    } else {
      snapshot->wptr_updates = 0;
      snapshot->interrupts[0] = {};
      snapshot->interrupts[1] = {};
    }
    report_queue_.Publish();
    report_capture_baseline_ = captured_at;
    report_cv_.notify_one();
#endif
  }

#if COD5_DIAGNOSTICS
  void ReportLoop() {
    for (;;) {
      if (const NativeReportSnapshot* snapshot = report_queue_.TryAcquire()) {
        try {
          FormatReport(*snapshot);
        } catch (const std::exception& error) {
          REXLOG_ERROR("[native] asynchronous diagnostic report failed: {}", error.what());
        } catch (...) {
          REXLOG_ERROR("[native] asynchronous diagnostic report failed with an unknown exception");
        }
        report_queue_.Release();
        continue;
      }
      if (report_stop_.load(std::memory_order_acquire) &&
          report_queue_.consumed() == report_queue_.published()) {
        break;
      }
      std::unique_lock<std::mutex> lock(report_mutex_);
      report_cv_.wait_for(lock, std::chrono::milliseconds(50), [this] {
        return report_stop_.load(std::memory_order_acquire) ||
               report_queue_.consumed() != report_queue_.published();
      });
    }
  }

  void FormatReport(const NativeReportSnapshot& snapshot) {
    const uint64_t packets_ = snapshot.packets;
    const uint64_t draws_ = snapshot.draws;
    const uint64_t copies_ = snapshot.copies;
    const uint64_t swaps_ = snapshot.swaps;
    const uint64_t waits_cut_short_ = snapshot.waits_cut_short;
    const uint64_t waits_ = snapshot.waits;
    const uint64_t wait_ns_ = snapshot.wait_ns;
    const uint64_t ring_idle_ns_ = snapshot.ring_idle_ns;
    const uint64_t hold_ns_ = snapshot.hold_ns;
    const uint64_t indirect_limit_skips_ = snapshot.indirect_limit_skips;
    const bool renderer_ready_ = snapshot.renderer_ready;
    const bool diagnostics_ = snapshot.diagnostics;
    const bool ring_ready_fast_path_ = snapshot.ring_ready_fast_path;
    const uint64_t ring_ready_bypasses_ = snapshot.ring_ready_bypasses;
    const uint64_t ring_cv_paths_ = snapshot.ring_cv_paths;
    if (diagnostics_) {
      REXLOG_INFO("[report-cost] schema=1 calls={} ns={} max_ns={} cumulative=1 "
                  "point=NativeGraphicsSystem::Report action=async_snapshot dropped={}",
                  snapshot.report_calls, snapshot.report_ns, snapshot.report_max_ns,
                  snapshot.dropped_reports);
    }
    REXLOG_INFO("[native] report interval {:.1f} s: {} packets, {} draws, {} copies, {} swaps, {} waits cut short",
                double(snapshot.interval_ns) / 1e9, packets_ - reported_packets_,
                draws_ - reported_draws_, copies_ - reported_copies_,
                swaps_ - reported_swaps_, waits_cut_short_);
    reported_packets_ = packets_;
    reported_draws_ = draws_;
    reported_copies_ = copies_;
    reported_swaps_before_ = reported_swaps_;
    reported_swaps_ = swaps_;
    if (renderer_ready_) {
      const RendererStats& s = snapshot.renderer;
      cod5::evidence::Report(snapshot.sample, swaps_, snapshot.skinning,
          snapshot.lightgrid, snapshot.fill, snapshot.workers, snapshot.lod, s,
          snapshot.writes_healthy, snapshot.file_writes, snapshot.zero_wait);
      cod5::handoff::Report(snapshot.sample,swaps_);
      REXLOG_INFO(
          "[native] renderer: {} drawn, {} skipped, {} resolves, {} passes, {} submissions, "
          "{} shaders ({} cached), {} pipelines, {} textures ({} MB), {} MB vertices",
          s.draws - reported_render_.draws, s.draws_skipped - reported_render_.draws_skipped,
          s.resolves - reported_render_.resolves, s.render_passes - reported_render_.render_passes,
          s.submissions - reported_render_.submissions, s.shaders_translated,
          s.shaders_from_cache, s.pipelines_created, s.textures_uploaded - reported_render_.textures_uploaded,
          (s.texture_bytes_uploaded - reported_render_.texture_bytes_uploaded) >> 20,
          (s.vertex_bytes_copied - reported_render_.vertex_bytes_copied) >> 20);
      REXLOG_INFO(
          "[native] textures: {} made now, {} streamed in, {} replaced after use, {} lookups "
          "answered empty (streaming or not handled)",
          s.textures_created - reported_render_.textures_created,
          s.textures_streamed - reported_render_.textures_streamed,
          s.textures_replaced - reported_render_.textures_replaced,
          s.textures_pending - reported_render_.textures_pending);
      REXLOG_INFO("[native] texture memory: {} MB, {} freed by the budget since start; {} lookups answered with a "
                  "predecessor while building",
                  s.texture_memory_bytes >> 20, s.texture_budget_evictions,
                  s.texture_placeholders - reported_render_.texture_placeholders);
#if COD5_DIAGNOSTICS
      if (diagnostics_) {
        REXLOG_INFO(
            "[texture-evict] schema=1 checks={} over_budget={} entries={} eligible={} sorted={} "
            "budget_evictions={} budget_bytes={} age_evictions={} age_bytes={} "
            "scan_ns={} candidate_ns={} sort_ns={} release_ns={} age_scan_ns={} "
            "pool_dedicated={} pool_no_fit={} pool_slabs_failed={} cumulative=1 action=record_only",
            s.texture_eviction_checks, s.texture_eviction_over_budget,
            s.texture_eviction_entries_scanned, s.texture_eviction_eligible,
            s.texture_eviction_sorted, s.texture_budget_evictions,
            s.texture_budget_evicted_bytes, s.texture_age_evictions,
            s.texture_age_evicted_bytes, s.texture_eviction_scan_ns,
            s.texture_eviction_candidate_ns, s.texture_eviction_sort_ns,
            s.texture_eviction_release_ns, s.texture_eviction_age_scan_ns,
            s.texture_pool_dedicated, s.texture_pool_no_fit,
            s.texture_pool_slabs_failed);
        REXLOG_INFO(
            "[change-detector] schema=1 texture_trusted={} texture_sample_checks={} texture_sample_hits={} "
            "texture_full={} texture_initial={} texture_unchanged={} texture_changed={} "
            "texture_sample_bytes={} texture_full_bytes={} vertex_trusted={} vertex_sample_checks={} "
            "vertex_sample_hits={} vertex_full={} vertex_initial={} vertex_unchanged={} vertex_changed={} "
            "vertex_sample_bytes={} vertex_full_bytes={} cumulative=1 action=record_only",
            s.texture_change_paths[0], s.texture_change_paths[1], s.texture_change_paths[2],
            s.texture_change_paths[3], s.texture_change_paths[4], s.texture_change_paths[5],
            s.texture_change_paths[6], s.texture_change_paths[7], s.texture_change_paths[8],
            s.vertex_change_paths[0],
            s.vertex_change_paths[1], s.vertex_change_paths[2], s.vertex_change_paths[3],
            s.vertex_change_paths[4], s.vertex_change_paths[5], s.vertex_change_paths[6],
            s.vertex_change_paths[7], s.vertex_change_paths[8]);
        REXLOG_INFO(
            "[view-alive] schema=1 epoch_fast={} batches={} queries={} failures={} revalidated={} "
            "oversized={} probes={} probe_max={} cumulative=1 action=record_only",
            s.view_epoch_fast_hits, s.view_alive_batches, s.view_alive_queries,
            s.view_alive_failures, s.view_alive_revalidated, s.view_alive_oversized,
            s.view_alive_probes, s.view_alive_probe_max);
        REXLOG_INFO(
            "[flat-clear] schema=1 "
            "tex_checked={}/{}/{}/{} tex_memo={}/{}/{}/{} tex_views={}/{}/{}/{} "
            "setup={}/{}/{}/{} index={}/{}/{}/{} vertex_ranges={}/{}/{}/{} "
            "vertex_hashes={}/{}/{}/{} bindings={}/{}/{}/{} "
            "fields=count,entries,capacity,ns cumulative=1 action=record_only",
            s.texture_flat_clear_count[0], s.texture_flat_clear_entries[0],
            s.texture_flat_clear_capacity[0], s.texture_flat_clear_ns[0],
            s.texture_flat_clear_count[1], s.texture_flat_clear_entries[1],
            s.texture_flat_clear_capacity[1], s.texture_flat_clear_ns[1],
            s.texture_flat_clear_count[2], s.texture_flat_clear_entries[2],
            s.texture_flat_clear_capacity[2], s.texture_flat_clear_ns[2],
            s.flat_clear_count[0], s.flat_clear_entries[0], s.flat_clear_capacity[0],
            s.flat_clear_ns[0], s.flat_clear_count[1], s.flat_clear_entries[1],
            s.flat_clear_capacity[1], s.flat_clear_ns[1], s.flat_clear_count[2],
            s.flat_clear_entries[2], s.flat_clear_capacity[2], s.flat_clear_ns[2],
            s.flat_clear_count[3], s.flat_clear_entries[3], s.flat_clear_capacity[3],
            s.flat_clear_ns[3], s.flat_clear_count[4], s.flat_clear_entries[4],
            s.flat_clear_capacity[4], s.flat_clear_ns[4]);
      }
#endif
      Renderer::FormatReportSnapshot(snapshot.renderer_report);
      for (uint32_t i = 0; i < snapshot.slow_frame_count; ++i) {
        const auto& slow = snapshot.slow_frames[i];
        REXLOG_INFO(
            "[native-slow] schema=1 rank={} frame_ms={:.1f} ring_idle_ms={:.1f} "
            "pacing_hold_ms={:.1f} game_wait_ms={:.1f} gpu_wait_ms={:.1f} "
            "stream_wait_ms={:.1f} untiling_wait_ms={:.1f} draw_thread_wait_ms={:.1f} "
            "texture_ms={:.1f} draws={} uploads={} created={} shaders={} pipelines={} "
            "interval=1 action=async_snapshot",
            i + 1, double(slow.frame_ns) / 1e6, double(slow.ring_idle_ns) / 1e6,
            double(slow.hold_ns) / 1e6, double(slow.game_wait_ns) / 1e6,
            double(slow.fence_wait_ns) / 1e6, double(slow.stream_wait_ns) / 1e6,
            double(slow.upload_wait_ns) / 1e6, double(slow.drain_wait_ns) / 1e6,
            double(slow.texture_ns) / 1e6, slow.draws, slow.textures, slow.created,
            slow.shaders, slow.pipelines);
      }
      uint64_t frames = std::max<uint64_t>(swaps_ - reported_swaps_before_, 1);
      if (diagnostics_) {
        const auto& skinning = snapshot.skinning;
        if (skinning.calls != reported_skinning_.calls || skinning.fallbacks != reported_skinning_.fallbacks) {
          REXLOG_INFO("[skinning] interval: {} calls, {} vertices (1/2/3/4 bone: {}/{}/{}/{}); "
                      "compared {}, native-fast {}, fallback {}, mismatch {}",
                      skinning.calls - reported_skinning_.calls,
                      skinning.vertices - reported_skinning_.vertices,
                      skinning.vertices_by_bones[0] - reported_skinning_.vertices_by_bones[0],
                      skinning.vertices_by_bones[1] - reported_skinning_.vertices_by_bones[1],
                      skinning.vertices_by_bones[2] - reported_skinning_.vertices_by_bones[2],
                      skinning.vertices_by_bones[3] - reported_skinning_.vertices_by_bones[3],
                      skinning.compared_vertices - reported_skinning_.compared_vertices,
                      skinning.fast_vertices - reported_skinning_.fast_vertices,
                      skinning.fallbacks - reported_skinning_.fallbacks,
                      skinning.mismatches - reported_skinning_.mismatches);
          reported_skinning_ = skinning;
        }
        const auto& lightgrid=snapshot.lightgrid;
        if(lightgrid.calls!=reported_lightgrid_.calls) {
          REXLOG_INFO("[lightgrid] interval: {} calls, compared {}, native-fast {}, fallback {}, mismatch {}; "
                      "last source {:08X}, destination {:08X} (one writer, not verified volume mapping)",
                      lightgrid.calls-reported_lightgrid_.calls,lightgrid.compared-reported_lightgrid_.compared,
                      lightgrid.fast-reported_lightgrid_.fast,lightgrid.fallbacks-reported_lightgrid_.fallbacks,
                      lightgrid.mismatches-reported_lightgrid_.mismatches,lightgrid.last_source,lightgrid.last_destination);
          reported_lightgrid_=lightgrid;
        }
        const auto& drawsurfs = snapshot.drawsurfs;
        if (drawsurfs.lists != reported_drawsurfs_.lists) {
          std::string techniques;
          for (size_t i=0; i<drawsurfs.lists_by_technique.size(); ++i) {
            const auto count = drawsurfs.lists_by_technique[i] - reported_drawsurfs_.lists_by_technique[i];
            const auto surfaces = drawsurfs.surfaces_by_technique[i] - reported_drawsurfs_.surfaces_by_technique[i];
            if (count) techniques += fmt::format(" {}:{}/{}", i, count, surfaces);
          }
          REXLOG_INFO("[game-opt] draw-surfs interval: {} lists, {} surfaces; suppressed {} lists / {} surfaces; "
                      "technique lists/surfaces:{}",
                      drawsurfs.lists - reported_drawsurfs_.lists,
                      drawsurfs.surfaces - reported_drawsurfs_.surfaces,
                      drawsurfs.suppressed_lists - reported_drawsurfs_.suppressed_lists,
                      drawsurfs.suppressed_surfaces - reported_drawsurfs_.suppressed_surfaces, techniques);
          reported_drawsurfs_ = drawsurfs;
        }
        const auto& lod=snapshot.lod;
        if(lod.calls!=reported_lod_.calls) {
          REXLOG_INFO("[game-opt] LOD interval: {} setups, rigid changed {}, skinned changed {}, invalid {}, unsupported {}",
                      lod.calls-reported_lod_.calls,lod.rigid_changed-reported_lod_.rigid_changed,
                      lod.skinned_changed-reported_lod_.skinned_changed,lod.invalid-reported_lod_.invalid,
                      lod.unsupported-reported_lod_.unsupported);
          reported_lod_=lod;
        }
        if(s.volume_uploads!=reported_render_.volume_uploads || s.volume_retries!=reported_render_.volume_retries) {
          REXLOG_INFO("[native-diag] CPU volume interval: {} uploads, {:.2f} MB, conversion {:.3f} ms, "
                      "layout build {:.3f} ms, {} retries; hash/snapshot included in texture totals; polling 30 frames",
                      s.volume_uploads-reported_render_.volume_uploads,
                      double(s.volume_bytes-reported_render_.volume_bytes)/1048576.0,
                      double(s.volume_conversion_ns-reported_render_.volume_conversion_ns)/1e6,
                      double(s.volume_plan_ns-reported_render_.volume_plan_ns)/1e6,
                      s.volume_retries-reported_render_.volume_retries);
        }
        const auto& worker=snapshot.workers;
        const auto& fill=snapshot.fill;
        if(fill.observed_fill!=reported_fill_.observed_fill || fill.observed_block!=reported_fill_.observed_block ||
           fill.calls!=reported_fill_.calls || fill.fallbacks!=reported_fill_.fallbacks) {
          REXLOG_INFO("[fill] interval: {} fill, {} block-fill observed; {} zero candidates ({} bytes), "
                      "{} compared, {} native-fast, {} fallback, {} mismatch",
                      fill.observed_fill-reported_fill_.observed_fill,fill.observed_block-reported_fill_.observed_block,
                      fill.calls-reported_fill_.calls,fill.bytes-reported_fill_.bytes,fill.compared-reported_fill_.compared,
                      fill.fast-reported_fill_.fast,fill.fallbacks-reported_fill_.fallbacks,fill.mismatches-reported_fill_.mismatches);
          reported_fill_=fill;
        }
        if(snapshot.writes_enabled) {
          const auto& file_writes=snapshot.file_writes;
          REXLOG_INFO("[writes] file-read interval: {} physical reads, {} requested bytes, {} invalid ranges; "
                      "scope closed before callbacks/completion; partial/failed reads conservatively marked",
                      file_writes.calls-reported_file_writes_.calls,file_writes.bytes-reported_file_writes_.bytes,
                      file_writes.invalid-reported_file_writes_.invalid);
          reported_file_writes_=file_writes;
          REXLOG_INFO("[writes] vertex/index interval: {} reused hashes, {} compared, {} unmarked changes, {} busy/changed checks; "
                      "healthy {}, active writers {} (coverage incomplete)",
                      s.vertex_write_hits-reported_render_.vertex_write_hits,
                      s.vertex_write_compared-reported_render_.vertex_write_compared,
                      s.vertex_write_mismatches-reported_render_.vertex_write_mismatches,
                      s.vertex_write_busy-reported_render_.vertex_write_busy,
                      snapshot.writes_healthy,snapshot.active_write_pages);
        }
        if(worker.scans!=reported_workers_.scans &&
           (worker.inflight_scans || worker.available_scans || worker.negative_scans || worker.stalled_parks)) {
          REXLOG_INFO("[workers] stalled classification: {} in-flight-only, {} positive available, {} negative claims; "
                      "{} bounded stalled parks (observations, not runnable-job counts)",
                      worker.inflight_scans-reported_workers_.inflight_scans,
                      worker.available_scans-reported_workers_.available_scans,
                      worker.negative_scans-reported_workers_.negative_scans,
                      worker.stalled_parks-reported_workers_.stalled_parks);
        }
        if(worker.scans!=reported_workers_.scans || worker.errors!=reported_workers_.errors) {
          REXLOG_INFO("[workers] interval: {} scans, {} parks, {} changed, {} quiet returns; {} queued, {} unsafe; "
                      "{} notifications, {} wake words; slept {:.1f} ms, {} late, {} errors (last {:08X})",
                      worker.scans-reported_workers_.scans,worker.parks-reported_workers_.parks,
                      worker.changed-reported_workers_.changed,worker.quiet_returns-reported_workers_.quiet_returns,
                      worker.queued-reported_workers_.queued,worker.unsafe-reported_workers_.unsafe,
                      worker.notifications-reported_workers_.notifications,worker.wakes-reported_workers_.wakes,
                      double(worker.sleep_ns-reported_workers_.sleep_ns)/1e6,worker.late-reported_workers_.late,
                      worker.errors-reported_workers_.errors,worker.last_error);
          reported_workers_=worker;
        }
#if defined(COD5_SWITCH_ZERO_WAIT_OVERLAY)
        const auto& zero_wait=snapshot.zero_wait;
        REXLOG_INFO("[native-diag] zero-timeout probes: fast empty {}, legacy empty {}, ready/interrupt {}, contended {} "
                    "(initial predicate disposition; guest timeout yield preserved)",
                    zero_wait.fast_empty-reported_zero_wait_.fast_empty,
                    zero_wait.legacy_empty-reported_zero_wait_.legacy_empty,
                    zero_wait.ready-reported_zero_wait_.ready,
                    zero_wait.contended-reported_zero_wait_.contended);
        reported_zero_wait_=zero_wait;
#endif
        REXLOG_INFO("[native-diag] per frame supply: ring idle {:.1f} ms, pacing hold {:.1f} ms, "
                    "write-pointer updates {:.1f}; guest vblank-wait hook is not instrumented",
                    double(ring_idle_ns_ - reported_idle_ns_) / 1e6 / double(frames),
                    double(hold_ns_ - reported_hold_ns_) / 1e6 / double(frames),
                    double(snapshot.wptr_updates) / double(frames));
#if COD5_DIAGNOSTICS
        REXLOG_INFO("[ring-wait] schema=1 sample={} frames={} enabled={} ready_bypass={} cv_path={} cumulative=1",
                    snapshot.sample, swaps_, ring_ready_fast_path_, ring_ready_bypasses_, ring_cv_paths_);
#endif
        reported_idle_ns_ = ring_idle_ns_;
        reported_hold_ns_ = hold_ns_;
        REXLOG_INFO("[native-diag] stream rollovers while preparing draws: {} (lifetime audit counter)",
                    s.stream_rollovers_during_draw - reported_render_.stream_rollovers_during_draw);
        if(s.record_queued!=reported_render_.record_queued) {
          REXLOG_INFO("[native-diag] recorder interval: {} draws queued, {} executed; {} publications, {} batches, "
                      "{} wakes, {} capacity waits, {} drains; recording wall {:.1f} ms, capacity wait {:.1f} ms, "
                      "drain wait {:.1f} ms (includes scheduling, overlaps ring preparation)",
                      s.record_queued-reported_render_.record_queued,s.record_executed-reported_render_.record_executed,
                      s.record_publications-reported_render_.record_publications,s.record_batches-reported_render_.record_batches,
                      s.record_wakes-reported_render_.record_wakes,s.record_capacity_waits-reported_render_.record_capacity_waits,
                      s.record_drains-reported_render_.record_drains,
                      double(s.record_execute_ns-reported_render_.record_execute_ns)/1e6,
                      double(s.record_capacity_wait_ns-reported_render_.record_capacity_wait_ns)/1e6,
                      double(s.record_drain_wait_ns-reported_render_.record_drain_wait_ns)/1e6);
        }
        REXLOG_INFO("[native-diag] guest cache syncs: {}, vertex/index content rechecks: {}",
                    s.guest_cache_syncs - reported_render_.guest_cache_syncs,
                    s.vertex_sync_rechecks - reported_render_.vertex_sync_rechecks);
        REXLOG_INFO("[native-diag] resolve channel-map changes: {} since start; "
                    "GetResolveDestination invalidates memo/slot/binding answers, retains old views",
                    s.resolve_swizzle_changes);
        REXLOG_INFO("[native-diag] scoped resolve answers since start: checks={} eligible={} fallbacks={}; "
                    "CanReuseAnswers on binding/slot epoch mismatch; eligibility only, later guards still apply",
                    s.resolve_answer_checks, s.resolve_answer_reuses, s.resolve_answer_fallbacks);
#if COD5_DIAGNOSTICS
        REXLOG_INFO(
            "[resolve-repeat] schema=1 observations={} candidates={} collisions={} "
            "paths=downsample:{}/{} depth_direct:{}/{} depth_scratch:{}/{} "
            "color_msaa:{}/{} color_compute:{}/{} color_transfer:{}/{} "
            "cumulative=1 action=record_only",
            s.resolve_repeat_observations, s.resolve_repeat_candidates,
            s.resolve_repeat_collisions,
            s.resolve_repeat_path_candidates[0], s.resolve_repeat_path_observations[0],
            s.resolve_repeat_path_candidates[1], s.resolve_repeat_path_observations[1],
            s.resolve_repeat_path_candidates[2], s.resolve_repeat_path_observations[2],
            s.resolve_repeat_path_candidates[3], s.resolve_repeat_path_observations[3],
            s.resolve_repeat_path_candidates[4], s.resolve_repeat_path_observations[4],
            s.resolve_repeat_path_candidates[5], s.resolve_repeat_path_observations[5]);
#endif
        REXLOG_INFO("[native-diag] indirect buffers skipped at nesting limit: {} since start",
                    indirect_limit_skips_);
        for (size_t source = 0; source < std::size(snapshot.interrupts); ++source) {
          const auto& t = snapshot.interrupts[source];
          REXLOG_INFO("[native-diag] {} callback: {} completions, wall {:.1f} ms, longest {:.1f} ms "
                      "(includes scheduling and lock waits)",
                      source == 0 ? "vblank" : "command",
                      t.count, double(t.ns) / 1e6, double(t.maximum) / 1e6);
        }
      }
      uint64_t gpu_count =
          std::max<uint64_t>(s.gpu_submissions - reported_render_.gpu_submissions, 1);
      REXLOG_INFO(
          "[native] per frame: renderer CPU {:.1f} ms, GPU {:.1f} ms over {:.1f} submissions "
          "(GPU time as the driver reports it)",
          double(s.cpu_ns - reported_render_.cpu_ns) / 1e6 / double(frames),
          double(s.gpu_ns - reported_render_.gpu_ns) / 1e6 / double(frames),
          double(s.gpu_submissions - reported_render_.gpu_submissions) / double(frames));
      const uint64_t renderer_wall = s.cpu_ns - reported_render_.cpu_ns;
      const uint64_t presentation_wall = s.presentation_call_ns - reported_render_.presentation_call_ns;
      REXLOG_INFO("[native-diag] per frame wall-time split: presentation call {:.1f} ms "
                  "(included in renderer CPU above), outside presentation {:.1f} ms "
                  "(includes other waits; not scheduled CPU execution time)",
                  double(presentation_wall) / 1e6 / double(frames),
                  double(renderer_wall - std::min(renderer_wall, presentation_wall)) / 1e6 / double(frames));
      (void)gpu_count;
      const uint64_t vertex_bytes = s.vertex_bytes_copied - reported_render_.vertex_bytes_copied;
      const uint64_t whole_bytes = s.vertex_bytes_whole - reported_render_.vertex_bytes_whole;
      REXLOG_INFO("[native] per frame: vertex copies {:.1f} MB (whole {:.1f}, ranged {:.1f}), "
                  "copying {:.1f} ms, vertex/index hashing {:.1f} ms",
                  double(vertex_bytes) / double(1u << 20) / double(frames),
                  double(whole_bytes) / double(1u << 20) / double(frames),
                  double(vertex_bytes - whole_bytes) / double(1u << 20) / double(frames),
                  double(s.vertex_copy_ns - reported_render_.vertex_copy_ns) / 1e6 / double(frames),
                  double(s.vertex_hash_ns - reported_render_.vertex_hash_ns) / 1e6 / double(frames));
      REXLOG_INFO("[native] per frame: {:.0f} draws, {:.0f} state rebuilds, {:.0f} texture "
                  "binding rebuilds",
                  double(s.draws - reported_render_.draws) / double(frames),
                  double(s.setups_built - reported_render_.setups_built) / double(frames),
                  double(s.texture_bindings_built - reported_render_.texture_bindings_built) /
                      double(frames));
      REXLOG_INFO(
          "[scene-zcull] schema=1 targets={} growth_fallbacks={} pass_clears={} "
          "interval=1 point=RenderTargetClear",
          s.scene_zcull_targets,
          s.scene_zcull_growth_fallbacks,
          s.scene_zcull_pass_clears - reported_render_.scene_zcull_pass_clears);
      REXLOG_INFO("[binding-key] schema=1 enabled={} queries={} memo_hits={} hashes={} "
                  "slot_mask_scans={} collisions={} interval=1 point=TextureBindings",
                  bool(s.binding_key_enabled),
                  s.binding_key_queries - reported_render_.binding_key_queries,
                  s.binding_key_memo_hits - reported_render_.binding_key_memo_hits,
                  s.binding_key_hashes - reported_render_.binding_key_hashes,
                  s.binding_key_slot_mask_scans - reported_render_.binding_key_slot_mask_scans,
                  s.binding_key_collisions - reported_render_.binding_key_collisions);
      REXLOG_INFO("[system-constants] schema=1 stable_setup_identity={} queries={} input_hits={} "
                  "recomputes={} content_hits={} uploads={} interval=1 point=DrawSystemConstants",
                  bool(s.system_constant_stable_setup),
                  s.system_constant_queries - reported_render_.system_constant_queries,
                  s.system_constant_input_hits - reported_render_.system_constant_input_hits,
                  s.system_constant_recomputes - reported_render_.system_constant_recomputes,
                  s.system_constant_content_hits - reported_render_.system_constant_content_hits,
                  s.system_constant_uploads - reported_render_.system_constant_uploads);
#if COD5_DIAGNOSTICS
      REXLOG_INFO(
          "[instance-candidates] schema=1 mode=record_only hash_based=1 "
          "draws={} eligible={} groups={} reducible={} max_group={} "
          "instance_changed={} instance_same={} "
          "breaks=context:{} no_instance:{} pipeline_state:{} geometry:{} "
          "textures:{} fixed_constants:{} dynamic_state:{} interval=1 "
          "point=DrawSubmission c4_c8=vertex dynamic_vs=excluded",
          s.instance_candidate_draws - reported_render_.instance_candidate_draws,
          s.instance_candidate_eligible - reported_render_.instance_candidate_eligible,
          s.instance_candidate_groups - reported_render_.instance_candidate_groups,
          s.instance_candidate_reducible - reported_render_.instance_candidate_reducible,
          s.instance_candidate_max_group,
          s.instance_candidate_changed - reported_render_.instance_candidate_changed,
          s.instance_candidate_same - reported_render_.instance_candidate_same,
          s.instance_break_context - reported_render_.instance_break_context,
          s.instance_break_no_instance - reported_render_.instance_break_no_instance,
          s.instance_break_pipeline_state - reported_render_.instance_break_pipeline_state,
          s.instance_break_geometry - reported_render_.instance_break_geometry,
          s.instance_break_textures - reported_render_.instance_break_textures,
          s.instance_break_fixed_constants - reported_render_.instance_break_fixed_constants,
          s.instance_break_dynamic_state - reported_render_.instance_break_dynamic_state);
      REXLOG_INFO(
          "[instance-batching] schema=1 enabled={} exact=1 reordered=0 guest_draws={} "
          "host_draws={} removed={} groups={} singles={} max_group={} interval=1 "
          "point=DrawSubmission c4_c8=vertex_input",
          bool(s.instance_batching_enabled),
          s.instance_runtime_guest_draws - reported_render_.instance_runtime_guest_draws,
          s.instance_runtime_host_draws - reported_render_.instance_runtime_host_draws,
          (s.instance_runtime_guest_draws - reported_render_.instance_runtime_guest_draws) -
              (s.instance_runtime_host_draws - reported_render_.instance_runtime_host_draws),
          s.instance_runtime_groups - reported_render_.instance_runtime_groups,
          s.instance_runtime_singles - reported_render_.instance_runtime_singles,
          s.instance_runtime_max_group);
#endif
      REXLOG_INFO("[native] per frame: texture lookups {:.1f} ms (hashing {:.1f} ms, {:.1f} images created on the "
                  "ring {:.1f} ms), {:.1f} memexport copies {:.2f} ms",
                  double(s.texture_lookup_ns - reported_render_.texture_lookup_ns) / 1e6 / double(frames),
                  double(s.texture_hash_ns - reported_render_.texture_hash_ns) / 1e6 / double(frames),
                  double(s.texture_creates - reported_render_.texture_creates) / double(frames),
                  double(s.texture_create_ns - reported_render_.texture_create_ns) / 1e6 / double(frames),
                  double(s.memexport_copies - reported_render_.memexport_copies) / double(frames),
                  double(s.memexport_ns - reported_render_.memexport_ns) / 1e6 / double(frames));
      REXLOG_INFO("[native-diag] per frame texture source snapshots: {:.2f} MB, {:.3f} ms, {:.1f} jobs "
                  "(included in texture lookups; workers read owned source bytes)",
                  double(s.texture_source_snapshot_bytes - reported_render_.texture_source_snapshot_bytes) /
                      double(1 << 20) / double(frames),
                  double(s.texture_source_snapshot_ns - reported_render_.texture_source_snapshot_ns) /
                      1e6 / double(frames),
                  double(s.texture_source_snapshot_jobs - reported_render_.texture_source_snapshot_jobs) /
                      double(frames));
      REXLOG_INFO("[native] per frame: {:.1f} waits for the game, {:.1f} ms waiting",
                  double(waits_ - reported_waits_) / double(frames),
                  double(wait_ns_ - reported_wait_ns_) / 1e6 / double(frames));
      reported_waits_ = waits_;
      REXLOG_INFO(
          "[native] slow frames: {} over 50 ms, {} over 100 ms; in them {} pipelines, "
          "{} shaders, {} texture uploads, {} render targets",
          s.frames_over_50ms - reported_render_.frames_over_50ms,
          s.frames_over_100ms - reported_render_.frames_over_100ms,
          s.slow_frame_pipelines - reported_render_.slow_frame_pipelines,
          s.slow_frame_shaders - reported_render_.slow_frame_shaders,
          s.slow_frame_uploads - reported_render_.slow_frame_uploads,
          s.slow_frame_render_targets - reported_render_.slow_frame_render_targets);
      reported_wait_ns_ = wait_ns_;
      std::string reasons;
      for (uint32_t i = 0; i < 16; ++i) {
        uint64_t n = s.skipped_by_reason[i] - reported_render_.skipped_by_reason[i];
        if (n) reasons += fmt::format(" {}:{}", i, n);
      }
      REXLOG_INFO("[native] skipped by reason:{}", reasons);
      reported_render_ = s;
    }
  }
#endif
#if COD5_DIAGNOSTICS
  RendererStats reported_render_;
  uint64_t evidence_sample_ = 0;
  cod5::game::DrawSurfCounters reported_drawsurfs_;
  cod5::skinning::Counters reported_skinning_;
  cod5::lightgrid::Counters reported_lightgrid_;
  cod5::game::LodCounters reported_lod_;
  cod5::threading::ZeroWaitCounters reported_zero_wait_;
  cod5::workers::Counters reported_workers_;
  cod5::fill::Counters reported_fill_;
  cod5::writes::FileCounters reported_file_writes_;
  uint64_t reported_swaps_before_ = 0;
#endif

  // --- PM4 packets -----------------------------------------------------------------

  // Consumes a whole packet, or leaves the position untouched if it is not
  // complete yet.
  bool Packet(Reader& reader, int depth) {
    const uint32_t packet = reader.Peek();
    uint32_t count = 0;
    switch (packet >> 30) {
      case 0:
        count = packet ? ((packet >> 16) & 0x3FFF) + 1 : 0;
        break;
      case 1:
        count = 2;
        break;
      case 2:
        count = 0;
        break;
      default:
        count = ((packet >> 16) & 0x3FFF) + 1;
        break;
    }
    if (reader.Pending() < count + 1) {
      return false;
    }
    reader.Advance(1);
    ++packets_;
    Reader data = reader;
    reader.Advance(count);
    if (!packet) {
      return true;
    }
    switch (packet >> 30) {
      case 0: {
        const uint32_t index = packet & 0x7FFF;
        const bool one_register = (packet >> 15) & 0x1;
        if (one_register) {
          for (uint32_t i = 0; i < count; ++i) {
            WriteRegister(index, data.Read());
          }
        } else {
          WriteRegisters(index, count, data);
        }
        break;
      }
      case 1: {
        const uint32_t value1 = data.Read();
        const uint32_t value2 = data.Read();
        WriteRegister(packet & 0x7FF, value1);
        WriteRegister((packet >> 11) & 0x7FF, value2);
        break;
      }
      case 2:
        break;
      default:
        Type3Packet(data, packet, count, depth);
        break;
    }
    return true;
  }

  void Type3Packet(Reader& data, uint32_t packet, uint32_t words, int depth) {
    const uint32_t opcode = (packet >> 8) & 0x7F;
    if ((packet & 0x1) && (!(bin_select_ & bin_mask_) || opcode == xenos::PM4_XE_SWAP)) {
      return;  // predicate fails; predicated swaps are always dropped
    }
    switch (opcode) {
      case xenos::PM4_INTERRUPT: {
        if (words < 1) break;
        const uint32_t cpus = data.Read();
        if (swap_interrupt_pending_) {
          // The swap-complete interrupt: right after the present, which the
          // frame rate cap already held (PM4_XE_SWAP), or held here if the
          // cap is not applied before presenting.
          swap_interrupt_pending_ = false;
          if (!REXCVAR_GET(cod5_pace_before_present)) PaceFrame();
        }
        for (uint32_t cpu = 0; cpu < 6; ++cpu) {
          if (cpus & (1u << cpu)) {
            Interrupt(1, cpu);
          }
        }
        break;
      }
      case xenos::PM4_XE_SWAP:
        // VdSwap writes "SWAP", the physical address of the frontbuffer and
        // its size.
        if (words >= 4) {
          data.Read();
          swap_frontbuffer_ = data.Read();
          swap_width_ = data.Read();
          swap_height_ = data.Read();
        }
        if (REXCVAR_GET(cod5_pace_before_present)) PaceFrame();
        Present();
        swap_interrupt_pending_ = true;
        ReportSlowFrame();
        break;
      case xenos::PM4_INDIRECT_BUFFER:
      case xenos::PM4_INDIRECT_BUFFER_PFD: {
        if (words < 2) break;
        const uint32_t address = data.Read() & 0x1FFFFFFF;
        const uint32_t length = data.Read() & 0xFFFFF;
        if (depth < kMaxIndirectDepth) {
          IndirectBuffer(address, length, depth + 1);
        } else if (++indirect_limit_skips_ == 1) {
          REXLOG_WARN("[native] Indirect buffer depth limit {} reached at {:08X}, {} words; "
                      "commands skipped (compatibility limit)", kMaxIndirectDepth, address, length);
        }
        break;
      }
      case xenos::PM4_WAIT_REG_MEM: {
        if (words < 5) break;
        const uint32_t info = data.Read();
        const uint32_t poll = data.Read();
        const uint32_t reference = data.Read();
        const uint32_t mask = data.Read();
        const auto wait_start = Clock::now();
        const auto limit = wait_start + kWaitRegMemMax;
        bool waited = false;
        for (uint32_t attempt = 0;; ++attempt) {
          if (!(info & 0x10) && poll == rex::graphics::XE_GPU_REG_COHER_STATUS_HOST &&
              (Register(poll) & UINT32_C(0x80000000))) {
            if (renderer_ready_) renderer_.GuestMemorySynchronized();
            registers_[poll] = 0;  // Acknowledge only after cache bookkeeping.
          }
          const uint32_t value = (info & 0x10) ? ReadMemory(poll) : Register(poll);
          if (Compare(info, value & mask, reference)) {
            break;
          }
          if (Clock::now() >= limit || !running_.load(std::memory_order_acquire)) {
            if (waits_cut_short_++ == 0) {
              REXLOG_WARN("[native] WAIT_REG_MEM not met ({} {:08X} ref {:08X} mask {:08X})",
                          (info & 0x10) ? "memory" : "register", poll, reference, mask);
            }
            break;
          }
          if (!waited && renderer_ready_) {
            // Idle time: let the draw recording thread empty its queue.
            renderer_.KickRecorder();
          }
          waited = true;
          // The game usually releases what the ring waits for within tens of
          // microseconds; yielding first avoids oversleeping each wait.
          if (attempt < 64) {
            std::this_thread::yield();
          } else {
            rex::thread::Sleep(std::chrono::microseconds(50));
          }
        }
        if (waited) {
          ++waits_;
          wait_ns_ += uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                   Clock::now() - wait_start)
                                   .count());
        }
        break;
      }
      case xenos::PM4_REG_RMW: {
        if (words < 3) break;
        const uint32_t info = data.Read();
        const uint32_t and_value = data.Read();
        const uint32_t or_value = data.Read();
        uint32_t value = Register(info & 0x1FFF);
        value &= ((info >> 31) & 0x1) ? Register(and_value & 0x1FFF) : and_value;
        value |= ((info >> 30) & 0x1) ? Register(or_value & 0x1FFF) : or_value;
        WriteRegister(info & 0x1FFF, value);
        break;
      }
      case xenos::PM4_REG_TO_MEM: {
        if (words < 2) break;
        const uint32_t reg = data.Read();
        const uint32_t address = data.Read();
        WriteMemory(address, Register(reg));
        break;
      }
      case xenos::PM4_MEM_WRITE: {
        if (words < 1) break;
        uint32_t address = data.Read();
        for (uint32_t i = 1; i < words; ++i) {
          WriteMemory(address, data.Read());
          address += 4;
        }
        break;
      }
      case xenos::PM4_COND_WRITE: {
        if (words < 6) break;
        const uint32_t info = data.Read();
        const uint32_t poll = data.Read();
        const uint32_t reference = data.Read();
        const uint32_t mask = data.Read();
        const uint32_t destination = data.Read();
        const uint32_t value_to_write = data.Read();
        const uint32_t value = (info & 0x10) ? ReadMemory(poll) : Register(poll);
        if (Compare(info, value & mask, reference)) {
          if (info & 0x100) {
            WriteMemory(destination, value_to_write);
          } else {
            WriteRegister(destination, value_to_write);
          }
        }
        break;
      }
      case xenos::PM4_EVENT_WRITE:
        if (words < 1) break;
        WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, data.Read() & 0x3F);
        break;
      case xenos::PM4_EVENT_WRITE_SHD: {
        if (words < 3) break;
        const uint32_t initiator = data.Read();
        const uint32_t address = data.Read();
        const uint32_t value = data.Read();
        WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, initiator & 0x3F);
        WriteMemory(address, ((initiator >> 31) & 0x1) ? counter_.load(std::memory_order_relaxed)
                                                       : value);
        break;
      }
      case xenos::PM4_EVENT_WRITE_EXT: {
        if (words < 2) break;
        const uint32_t initiator = data.Read();
        const uint32_t address = data.Read();
        WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, initiator & 0x3F);
        // The screen extent of the previous draw: the maximum is faked.
        const uint16_t extent[] = {0, kMaxExtent, 0, kMaxExtent, 0, 1};
        uint8_t* destination = memory_->TranslatePhysical(address & ~uint32_t(0x3));
        for (size_t i = 0; i < std::size(extent); ++i) {
          rex::memory::store_and_swap<uint16_t>(destination + i * 2, extent[i]);
        }
        break;
      }
      case xenos::PM4_EVENT_WRITE_ZPD: {
        if (words < 1) break;
        WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, data.Read() & 0x3F);
        const uint32_t address = Register(rex::graphics::XE_GPU_REG_RB_SAMPLE_COUNT_ADDR);
        if (address) {
          FakeOcclusionResult(address);
        }
        break;
      }
      case xenos::PM4_SET_CONSTANT: {
        if (words < 1) break;
        const uint32_t type_index = data.Read();
        const uint32_t base = ConstantBase(type_index);
        if (base != UINT32_MAX) {
          const uint32_t index = base + (type_index & 0x7FF);
          WriteRegisters(index, words - 1, data);
        }
        break;
      }
      case xenos::PM4_SET_CONSTANT2:
      case xenos::PM4_SET_SHADER_CONSTANTS: {
        // Same format: the first value is the register index, the rest are
        // written consecutively.
        if (words < 1) break;
        const uint32_t index = data.Read() & 0xFFFF;
        WriteRegisters(index, words - 1, data);
        break;
      }
      case xenos::PM4_LOAD_ALU_CONSTANT: {
        // Registers loaded from guest memory instead of from the ring.
        if (words < 3) break;
        const uint32_t address = data.Read() & 0x3FFFFFFF;
        const uint32_t type_index = data.Read();
        const uint32_t amount = data.Read() & 0xFFF;
        const uint32_t base = ConstantBase(type_index);
        if (base != UINT32_MAX) {
          const uint8_t* source = memory_->TranslatePhysical(address);
          const uint32_t index = base + (type_index & 0x7FF);
          for (uint32_t i = 0; i < amount; ++i) {
            WriteRegister(index + i, rex::memory::load_and_swap<uint32_t>(source + size_t(i) * 4));
          }
        }
        break;
      }
      case xenos::PM4_SET_BIN_MASK_LO:
        if (words < 1) break;
        bin_mask_ = (bin_mask_ & 0xFFFFFFFF00000000ull) | data.Read();
        break;
      case xenos::PM4_SET_BIN_MASK_HI:
        if (words < 1) break;
        bin_mask_ = (bin_mask_ & 0xFFFFFFFFull) | (uint64_t(data.Read()) << 32);
        break;
      case xenos::PM4_SET_BIN_SELECT_LO:
        if (words < 1) break;
        bin_select_ = (bin_select_ & 0xFFFFFFFF00000000ull) | data.Read();
        break;
      case xenos::PM4_SET_BIN_SELECT_HI:
        if (words < 1) break;
        bin_select_ = (bin_select_ & 0xFFFFFFFFull) | (uint64_t(data.Read()) << 32);
        break;
      case xenos::PM4_SET_BIN_MASK: {
        if (words < 2) break;
        const uint64_t high = data.Read();
        const uint64_t low = data.Read();
        bin_mask_ = (high << 32) | low;
        break;
      }
      case xenos::PM4_SET_BIN_SELECT: {
        if (words < 2) break;
        const uint64_t high = data.Read();
        const uint64_t low = data.Read();
        bin_select_ = (high << 32) | low;
        break;
      }
      case xenos::PM4_VIZ_QUERY: {
        // Like the emulation: when the query ends it is reported visible, in
        // case the game reads the result.
        if (words < 1) break;
        const uint32_t value = data.Read();
        const uint32_t id = value & 0x3F;
        if (!(value & 0x100)) {
          WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, xenos::VIZQUERY_START);
        } else {
          WriteRegister(rex::graphics::XE_GPU_REG_VGT_EVENT_INITIATOR, xenos::VIZQUERY_END);
          const uint32_t reg = id < 32 ? uint32_t(rex::graphics::XE_GPU_REG_PA_SC_VIZ_QUERY_STATUS_0)
                                       : uint32_t(rex::graphics::XE_GPU_REG_PA_SC_VIZ_QUERY_STATUS_1);
          registers_[reg] |= UINT32_C(1) << (id < 32 ? id : id - 32);
        }
        break;
      }
      case xenos::PM4_DRAW_INDX:
      case xenos::PM4_DRAW_INDX_2: {
        // DRAW_INDX starts with the visibility query word.
        const uint32_t skip = opcode == xenos::PM4_DRAW_INDX ? 1 : 0;
        if (words < skip + 1) break;
        if (skip) {
          data.Read();
        }
        const uint32_t initiator = data.Read();
        WriteRegister(rex::graphics::XE_GPU_REG_VGT_DRAW_INITIATOR, initiator);
        // Indices in a buffer (DMA): its base and size come in the packet and
        // are stored in their registers, as ExecutePacketType3Draw does.
        if (((initiator >> 6) & 0x3) == uint32_t(xenos::SourceSelect::kDMA)) {
          if (words < skip + 3) break;
          WriteRegister(rex::graphics::XE_GPU_REG_VGT_DMA_BASE, data.Read());
          WriteRegister(rex::graphics::XE_GPU_REG_VGT_DMA_SIZE, data.Read());
        }
        // Indices in the packet itself.
        const uint32_t* immediate = nullptr;
        uint32_t immediate_words = 0;
        if (((initiator >> 6) & 0x3) == uint32_t(xenos::SourceSelect::kImmediate)) {
          immediate_words = words - skip - 1;
          immediate_indices_.resize(immediate_words);
          for (uint32_t i = 0; i < immediate_words; ++i) {
            immediate_indices_[i] = data.Read();
          }
          immediate = immediate_indices_.data();
        }
        // A draw in copy mode is a resolve and/or a clear.
        if ((Register(rex::graphics::XE_GPU_REG_RB_MODECONTROL) & 0x7) ==
            uint32_t(xenos::EdramMode::kCopy)) {
          ++copies_;
          if (renderer_ready_) {
            renderer_.Resolve();
          }
        } else {
          ++draws_;
          if (renderer_ready_) {
            renderer_.Draw(immediate, immediate_words);
          }
        }
        break;
      }
      case xenos::PM4_IM_LOAD: {
        // The shader microcode from guest memory (CommandProcessor::
        // ExecutePacketType3_IM_LOAD).
        if (words < 2) break;
        const uint32_t address_type = data.Read();
        const uint32_t start_size = data.Read();
        const uint32_t dwords = start_size & 0xFFFF;
        const uint32_t address = address_type & ~uint32_t(0x3) & 0x1FFFFFFF;
        if (renderer_ready_ && dwords && uint64_t(address) + dwords * 4 <= 0x20000000u) {
          renderer_.LoadShader(xenos::ShaderType(address_type & 0x3),
                               memory_->TranslatePhysical<const uint32_t*>(address), dwords,
                               address);
        }
        break;
      }
      case xenos::PM4_IM_LOAD_IMMEDIATE: {
        // The microcode is in the packet, which may wrap around the ring.
        if (words < 2) break;
        const uint32_t type = data.Read();
        const uint32_t dwords = std::min(data.Read() & 0xFFFF, words - 2);
        shader_words_.resize(dwords);
        for (uint32_t i = 0; i < dwords; ++i) {
          // Kept in guest byte order, as the shader expects.
          shader_words_[i] = rex::byte_swap(data.Read());
        }
        if (renderer_ready_ && dwords) {
          renderer_.LoadShader(xenos::ShaderType(type & 0x3), shader_words_.data(), dwords, 0);
        }
        break;
      }
      default:
        // Do not silently treat an unreviewed opcode as supported. Count it
        // without dropping the remainder of this primary command buffer.
        if (unhandled_packets_[opcode]++ == 0) {
          REXLOG_WARN("[native] Unhandled PM4 opcode 0x{:02X}, {} words; compatibility review required",
                      opcode, words);
        }
        break;
    }
  }

  static uint32_t ConstantBase(uint32_t type_index) {
    switch ((type_index >> 16) & 0xFF) {
      case 0:
        return 0x4000;  // ALU
      case 1:
        return 0x4800;  // fetch
      case 2:
        return 0x4900;  // bool
      case 3:
        return 0x4908;  // loop
      case 4:
        return 0x2000;  // registers
      default:
        return UINT32_MAX;
    }
  }

  // As the emulation's fallback: the game marks the counters it waits for with
  // 0xFFFFFEED, and a visible sample count is written when a query ends.
  void FakeOcclusionResult(uint32_t address) {
    auto* counts = memory_->TranslatePhysical<xenos::xe_gpu_depth_sample_counts*>(address);
    const uint32_t kFinished = rex::byte_swap(UINT32_C(0xFFFFFEED));
    const bool end = counts->ZPass_A == kFinished || counts->ZPass_B == kFinished ||
                     counts->ZFail_A == kFinished || counts->ZFail_B == kFinished;
    std::memset(counts, 0, sizeof(*counts));
    if (end) {
      counts->ZPass_A = kFakeOcclusionSamples;
      counts->Total_A = kFakeOcclusionSamples;
    }
  }

  void IndirectBuffer(uint32_t address, uint32_t words, int depth) {
    Reader reader;
    reader.base = memory_->TranslatePhysical(address);
    reader.pos = 0;
    reader.end = words;
    while (reader.Pending()) {
      if (!Packet(reader, depth)) {
        break;
      }
    }
  }

  // --- Test presentation -------------------------------------------------------------

  // Keep the four worst frames as fixed POD. Formatting and logging belong to
  // the report consumer, never the command-ring producer.
  void ReportSlowFrame() {
#if COD5_DIAGNOSTICS
    if (!diagnostics_) return;
    const auto now = Clock::now();
    const uint64_t game_wait_ns = wait_ns_ - frame_start_wait_ns_;
    frame_start_wait_ns_ = wait_ns_;
    const uint64_t idle_ns = ring_idle_ns_ - frame_start_idle_ns_;
    frame_start_idle_ns_ = ring_idle_ns_;
    const uint64_t hold_ns = hold_ns_ - frame_start_hold_ns_;
    frame_start_hold_ns_ = hold_ns_;
    const auto previous = frame_start_;
    frame_start_ = now;
    if (!renderer_ready_ || previous.time_since_epoch().count() == 0) return;
    const uint64_t frame_ns = uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                           now - previous).count());
    const RendererStats& s = renderer_.stats();
    const uint64_t pipelines = s.pipelines_created - frame_start_pipelines_;
    const uint64_t shaders = s.shaders_translated - frame_start_shaders_;
    const uint64_t textures = s.textures_uploaded - frame_start_textures_;
    const uint64_t created = s.textures_created - frame_start_created_;
    frame_start_pipelines_ = s.pipelines_created;
    frame_start_shaders_ = s.shaders_translated;
    frame_start_textures_ = s.textures_uploaded;
    frame_start_created_ = s.textures_created;
    if (frame_ns < UINT64_C(50000000)) return;
    NativeReportSnapshot::SlowFrame sample{
        frame_ns, game_wait_ns, idle_ns, hold_ns, s.frame_fence_wait_ns,
        s.frame_stream_wait_ns, s.frame_upload_wait_ns, s.frame_drain_wait_ns,
        s.frame_texture_ns, pipelines, shaders, textures, created, s.frame_draws};
    slow_frame_samples_.Consider(sample);
#endif
  }

  // The frame rate cap (cod5_min_frame_vblanks): a frame is presented at a
  // vblank at least that many after the previous present, or at once if it is
  // later than that. The hold used to come after the present, before the
  // swap-complete interrupt: an early frame went on screen at once and the
  // next one, if a little over budget, 50 ms later, a short gap then a long
  // one, felt as a hitch even at 30 fps on average. Held before it, presents
  // are evenly spaced, and the interrupt still follows the present.
  void PaceFrame() {
    const uint32_t min_vblanks = REXCVAR_GET(cod5_min_frame_vblanks);
    if (min_vblanks > 1 && last_swap_vblank_) {
      const uint64_t target = last_swap_vblank_ + min_vblanks;
      if (REXCVAR_GET(cod5_pace_before_present) &&
          vblanks_.load(std::memory_order_acquire) < target && renderer_ready_) {
        // The frame's draws go to the GPU now, not after the wait.
        renderer_.Flush();
      }
      const auto hold_start = Clock::now();
      const auto limit = hold_start + std::chrono::milliseconds(100);
      while (vblanks_.load(std::memory_order_acquire) < target &&
             running_.load(std::memory_order_acquire) && Clock::now() < limit) {
        rex::thread::Sleep(std::chrono::microseconds(250));
      }
      hold_ns_ += uint64_t(
          std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - hold_start)
              .count());
    }
    last_swap_vblank_ = vblanks_.load(std::memory_order_acquire);
  }

  void MaybeCaptureFrame(uint64_t swap) {
    if constexpr (!cod5::diagnostics::kEnabled) return;
    const uint32_t every = REXCVAR_GET(cod5_debug_capture_every);
    if (!every || !presenter_ || swap % every) return;
    const std::string dir = REXCVAR_GET(cod5_debug_capture_dir);
    if (dir.empty()) return;
    rex::ui::RawImage image;
    if (!presenter_->CaptureGuestOutput(image) || !image.width || !image.height) return;
    const std::string path = fmt::format("{}/frame_{:06d}.rgbx", dir, swap);
    if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
      const uint32_t header[2] = {image.width, image.height};
      std::fwrite(header, sizeof(header), 1, f);
      for (uint32_t y = 0; y < image.height; ++y) {
        std::fwrite(image.data.data() + y * image.stride, 4, image.width, f);
      }
      std::fclose(f);
      REXLOG_INFO("[native] captured frame {} ({}x{}) to {}", swap, image.width, image.height, path);
    }
  }

  void Present() {
    if (!renderer_ready_ || !presenter_) {
      REXLOG_ERROR("[native] Swap rejected: native renderer/presenter is not ready");
      return;
    }
    const uint64_t swap = ++swaps_;
    g_swaps_presented.fetch_add(1, std::memory_order_relaxed);
    counter_.fetch_add(1, std::memory_order_relaxed);
#if defined(__SWITCH__)
    RexSwitchPerfCount(0);
#endif
    renderer_.Swap(presenter_.get(), swap_frontbuffer_, swap_width_, swap_height_);
    g_last_frame_draws.store(uint32_t(renderer_.stats().frame_draws), std::memory_order_relaxed);
    MaybeCaptureFrame(swap);
  }

  bool ClearOutput(OutputContext& context, uint64_t swap) {
    const rex::ui::vulkan::VulkanDevice* device_info = provider_->vulkan_device();
    const auto& dfn = device_info->functions();
    const VkDevice device = device_info->device();

    if (render_pass_ == VK_NULL_HANDLE) {
      VkAttachmentDescription attachment{};
      attachment.format = rex::ui::vulkan::VulkanPresenter::kGuestOutputFormat;
      attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachment.finalLayout = rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalLayout;
      VkAttachmentReference reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &reference;
      VkRenderPassCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      info.attachmentCount = 1;
      info.pAttachments = &attachment;
      info.subpassCount = 1;
      info.pSubpasses = &subpass;
      if (dfn.vkCreateRenderPass(device, &info, nullptr, &render_pass_) != VK_SUCCESS) {
        render_pass_ = VK_NULL_HANDLE;
        return false;
      }
    }

    if (pool_ == VK_NULL_HANDLE) {
      VkCommandPoolCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      info.queueFamilyIndex = device_info->queue_family_graphics_compute();
      if (dfn.vkCreateCommandPool(device, &info, nullptr, &pool_) != VK_SUCCESS) {
        pool_ = VK_NULL_HANDLE;
        return false;
      }
      VkCommandBufferAllocateInfo allocate{};
      allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate.commandPool = pool_;
      allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate.commandBufferCount = 1;
      if (dfn.vkAllocateCommandBuffers(device, &allocate, &commands_) != VK_SUCCESS) {
        commands_ = VK_NULL_HANDLE;
      }
      VkFenceCreateInfo fence_info{};
      fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      if (dfn.vkCreateFence(device, &fence_info, nullptr, &fence_) != VK_SUCCESS) {
        fence_ = VK_NULL_HANDLE;
      }
    }
    if (commands_ == VK_NULL_HANDLE || fence_ == VK_NULL_HANDLE) {
      return false;
    }

    // A single command buffer: wait for the previous one before reusing it.
    if (fence_pending_) {
      dfn.vkWaitForFences(device, 1, &fence_, VK_TRUE, UINT64_MAX);
      dfn.vkResetFences(device, 1, &fence_);
      fence_pending_ = false;
    }
    dfn.vkResetCommandPool(device, pool_, 0);

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    for (const Framebuffer& f : framebuffers_) {
      if (f.framebuffer != VK_NULL_HANDLE && f.version == context.image_version()) {
        framebuffer = f.framebuffer;
      }
    }
    if (framebuffer == VK_NULL_HANDLE) {
      Framebuffer& f = framebuffers_[next_framebuffer_];
      next_framebuffer_ = (next_framebuffer_ + 1) % std::size(framebuffers_);
      if (f.framebuffer != VK_NULL_HANDLE) {
        dfn.vkDestroyFramebuffer(device, f.framebuffer, nullptr);
        f.framebuffer = VK_NULL_HANDLE;
      }
      VkImageView view = context.image_view();
      VkFramebufferCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      info.renderPass = render_pass_;
      info.attachmentCount = 1;
      info.pAttachments = &view;
      info.width = kOutputWidth;
      info.height = kOutputHeight;
      info.layers = 1;
      if (dfn.vkCreateFramebuffer(device, &info, nullptr, &f.framebuffer) != VK_SUCCESS) {
        f.framebuffer = VK_NULL_HANDLE;
        return false;
      }
      f.version = context.image_version();
      framebuffer = f.framebuffer;
    }

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (dfn.vkBeginCommandBuffer(commands_, &begin) != VK_SUCCESS) {
      return false;
    }
    // Test colour: green rises and falls with the swap count, so a capture
    // tells a running game from a stalled one.
    const float phase = float(swap % 240) / 239.0f;
    VkClearValue color{};
    color.color.float32[0] = 0.05f;
    color.color.float32[1] = 0.10f + 0.40f * phase;
    color.color.float32[2] = 0.35f;
    color.color.float32[3] = 1.0f;
    VkRenderPassBeginInfo pass{};
    pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    pass.renderPass = render_pass_;
    pass.framebuffer = framebuffer;
    pass.renderArea.extent.width = kOutputWidth;
    pass.renderArea.extent.height = kOutputHeight;
    pass.clearValueCount = 1;
    pass.pClearValues = &color;
    dfn.vkCmdBeginRenderPass(commands_, &pass, VK_SUBPASS_CONTENTS_INLINE);
    dfn.vkCmdEndRenderPass(commands_);
    if (dfn.vkEndCommandBuffer(commands_) != VK_SUCCESS) {
      return false;
    }
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commands_;
    {
      const auto queue =
          device_info->AcquireQueue(device_info->queue_family_graphics_compute(), 0);
      if (dfn.vkQueueSubmit(queue.queue(), 1, &submit, fence_) != VK_SUCCESS) {
        return false;
      }
    }
    fence_pending_ = true;
    context.SetIs8bpc(true);
    return true;
  }

  void DestroyVulkan() {
    if (!provider_ || !provider_->vulkan_device()) {
      return;
    }
    const rex::ui::vulkan::VulkanDevice* device_info = provider_->vulkan_device();
    const auto& dfn = device_info->functions();
    const VkDevice device = device_info->device();
    if (fence_ != VK_NULL_HANDLE) {
      if (fence_pending_) {
        dfn.vkWaitForFences(device, 1, &fence_, VK_TRUE, UINT64_MAX);
        fence_pending_ = false;
      }
      dfn.vkDestroyFence(device, fence_, nullptr);
      fence_ = VK_NULL_HANDLE;
    }
    for (Framebuffer& f : framebuffers_) {
      if (f.framebuffer != VK_NULL_HANDLE) {
        dfn.vkDestroyFramebuffer(device, f.framebuffer, nullptr);
        f = Framebuffer{};
      }
    }
    if (pool_ != VK_NULL_HANDLE) {
      dfn.vkDestroyCommandPool(device, pool_, nullptr);
      pool_ = VK_NULL_HANDLE;
      commands_ = VK_NULL_HANDLE;
    }
    if (render_pass_ != VK_NULL_HANDLE) {
      dfn.vkDestroyRenderPass(device, render_pass_, nullptr);
      render_pass_ = VK_NULL_HANDLE;
    }
  }

  // --- State -------------------------------------------------------------------------

  rex::ui::WindowedAppContext* app_context_ = nullptr;
  std::unique_ptr<rex::ui::vulkan::VulkanProvider> provider_;
  std::unique_ptr<rex::ui::Presenter> presenter_;

  rex::runtime::FunctionDispatcher* dispatcher_ = nullptr;
  rex::system::KernelState* kernel_state_ = nullptr;
  rex::memory::Memory* memory_ = nullptr;
  bool mmio_registered_ = false;

  std::atomic<bool> running_{false};
  rex::system::object_ref<rex::system::XHostThread> vblank_thread_;
  rex::system::object_ref<rex::system::XHostThread> ring_thread_;

  std::atomic<uint32_t> callback_{0};
  std::atomic<uint32_t> callback_data_{0};
  // Incremented by vblanks and swaps; EVENT_WRITE_SHD can write it (the SDK's
  // command processor counter).
  std::atomic<uint32_t> counter_{0};
  // Vblanks delivered, and the vblank of the last swap-complete interrupt.
  std::atomic<uint64_t> vblanks_{0};
  uint64_t last_swap_vblank_ = 0;
#if COD5_DIAGNOSTICS
  Clock::time_point frame_start_{};
  uint64_t frame_start_wait_ns_ = 0, frame_start_pipelines_ = 0, frame_start_shaders_ = 0,
           frame_start_textures_ = 0, frame_start_created_ = 0;
  SlowFrameSamples<NativeReportSnapshot::SlowFrame,
                   NativeReportSnapshot::kSlowFrameCapacity> slow_frame_samples_;
#endif
  uint64_t ring_idle_ns_ = 0, hold_ns_ = 0;
  bool ring_ready_fast_path_ = false;
#if COD5_DIAGNOSTICS
  uint64_t ring_ready_bypasses_ = 0, ring_cv_paths_ = 0;
#endif
#if COD5_DIAGNOSTICS
  uint64_t frame_start_idle_ns_ = 0, frame_start_hold_ns_ = 0;
#endif
  bool swap_interrupt_pending_ = false;

  std::atomic<uint32_t> ring_base_{0};
  std::atomic<uint32_t> ring_words_{0};
  std::atomic<uint32_t> ring_generation_{0};
  std::atomic<uint32_t> write_pointer_{0};
  std::atomic<uint32_t> read_pointer_writeback_{0};
  std::mutex ring_mutex_;
  std::condition_variable ring_cv_;

  // Touched by the ring thread and, through MMIO, by the game's threads; as in
  // the emulation, races on these integers are harmless.
  // The renderer reads the GPU state from here: it is the SDK's register file,
  // so its helpers (draw_util and the rest) work on it unchanged.
  std::unique_ptr<rex::graphics::RegisterFile> register_file_;
  uint32_t* registers_;
  Renderer renderer_;
  bool renderer_ready_ = false;
  bool diagnostics_ = false;
  struct InterruptTiming {
    std::atomic<uint64_t> count{0}, ns{0}, maximum{0};
  };
  std::array<InterruptTiming, 2> interrupt_timings_;
  std::atomic<uint64_t> wptr_writes_{0};
#if COD5_DIAGNOSTICS
  // Report() is the nonblocking ring producer. The consumer owns all report
  // baselines, formatting and logging; shutdown drains it before renderer teardown.
  ReportSnapshotQueue<NativeReportSnapshot, 2> report_queue_;
  std::thread report_thread_;
  std::atomic<bool> report_stop_{false};
  std::mutex report_mutex_;
  std::condition_variable report_cv_;
  Clock::time_point report_capture_baseline_{};
  // Previous completed snapshot-capture costs, included in the next snapshot.
  uint64_t report_calls_ = 0, report_ns_ = 0, report_max_ns_ = 0;
#endif
#if COD5_DIAGNOSTICS
  uint64_t reported_idle_ns_ = 0, reported_hold_ns_ = 0;
#endif
  // CommandProcessor's gamma ramp state (DC_LUT_*), 256-entry table only.
  uint32_t gamma_ramp_[256] = {};
  uint32_t gamma_ramp_component_ = 0;
  std::vector<uint32_t> immediate_indices_;
  std::vector<uint32_t> shader_words_;
  uint64_t bin_mask_ = ~uint64_t(0);
  uint64_t bin_select_ = ~uint64_t(0);

  uint32_t swap_frontbuffer_ = 0;
  uint32_t swap_width_ = 0;
  uint32_t swap_height_ = 0;

  // Ring thread only.
  uint64_t packets_ = 0, draws_ = 0, copies_ = 0, swaps_ = 0, waits_cut_short_ = 0;
  uint64_t unhandled_packets_[128] = {};
  uint64_t indirect_limit_skips_ = 0;
  uint64_t waits_ = 0, wait_ns_ = 0;
#if COD5_DIAGNOSTICS
  uint64_t reported_waits_ = 0, reported_wait_ns_ = 0;
  uint64_t reported_packets_ = 0, reported_draws_ = 0, reported_copies_ = 0,
           reported_swaps_ = 0;
#endif

  struct Framebuffer {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    uint64_t version = 0;
  };
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  VkCommandPool pool_ = VK_NULL_HANDLE;
  VkCommandBuffer commands_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;
  bool fence_pending_ = false;
  Framebuffer framebuffers_[3];
  size_t next_framebuffer_ = 0;
};

}  // namespace

bool Enabled() { return REXCVAR_GET(cod5_renderer) == "native"; }

uint64_t SwapsPresented() { return g_swaps_presented.load(std::memory_order_relaxed); }

uint32_t LastFrameDraws() { return g_last_frame_draws.load(std::memory_order_relaxed); }

std::unique_ptr<rex::system::IGraphicsSystem> CreateGraphicsSystem() {
  return std::make_unique<NativeGraphicsSystem>();
}

}  // namespace cod5::native
