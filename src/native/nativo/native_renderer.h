// SPDX-License-Identifier: GPL-3.0-or-later
//
// Native renderer for the Switch port: draws the game's frames with Vulkan
// directly from the Xbox 360 GPU state the command ring leaves in the registers,
// without emulating the GPU's embedded memory (EDRAM).
//
// The design follows the native renderer of nfsmw-nx (StevensND/nfsmw-nx,
// app/src/nfsmw_nativo_*.cpp, GPL-3.0):
//   - Render targets are ordinary Vulkan images, one per EDRAM address, format,
//     pitch and sample count. Nothing is ever copied in or out of an EDRAM
//     buffer, so there are no ownership transfers.
//   - A resolve is an image copy into a texture registered at the destination
//     address. Textures fetched from that address, and the frontbuffer, use it
//     directly; the guest memory is not written.
//   - Vertex and index data are copied per submission into arenas the shaders
//     read, and deduplicated by content.
// Where nfsmw-nx uses an offline library of shaders recompiled for Need for
// Speed, this uses ReXGlue's runtime shader translator, the same one the
// emulated path uses, so the game's shaders need no preparation.
//
// Everything here runs on the thread that consumes the command ring.

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "native_fetch_generations.h"
#include "native_gpu_profile_snapshot.h"
#include "native_nvk_draw.h"
#include "native_textures.h"

#include <rex/graphics/register_file.h>
#include <rex/graphics/xenos.h>

#include "cod6_diagnostics.h"

namespace rex::memory {
class Memory;
}
namespace rex::ui {
class Presenter;
}
namespace rex::ui::vulkan {
class VulkanProvider;
}

namespace cod6::native {

struct RendererStats {
  uint64_t guest_cache_syncs = 0;
  uint64_t vertex_sync_rechecks = 0;
  uint64_t draws = 0;
  uint64_t draws_skipped = 0;
  uint64_t resolves = 0;
  uint64_t pipelines_created = 0;
  uint64_t shaders_translated = 0;
  uint64_t shaders_from_cache = 0;  // of shaders_translated, taken from the SPIR-V cache
  uint64_t textures_uploaded = 0;
  uint64_t texture_bytes_uploaded = 0;
  uint64_t textures_created = 0;
  uint64_t textures_streamed = 0;
  uint64_t textures_pending = 0;
  uint64_t texture_memory_bytes = 0;  // cod5-nx
  uint64_t texture_budget_evictions = 0;  // cod5-nx
#if COD6_DIAGNOSTICS
  uint64_t texture_eviction_checks = 0, texture_eviction_over_budget = 0;
  uint64_t texture_eviction_entries_scanned = 0, texture_eviction_eligible = 0;
  uint64_t texture_eviction_sorted = 0, texture_budget_evicted_bytes = 0;
  uint64_t texture_age_evictions = 0, texture_age_evicted_bytes = 0;
  uint64_t texture_eviction_scan_ns = 0, texture_eviction_candidate_ns = 0;
  uint64_t texture_eviction_sort_ns = 0, texture_eviction_release_ns = 0;
  uint64_t texture_eviction_age_scan_ns = 0;
  uint64_t texture_pool_dedicated = 0, texture_pool_no_fit = 0;
  uint64_t texture_pool_slabs_failed = 0;
  uint64_t texture_change_paths[9] = {};
  uint64_t texture_flat_clear_count[3] = {};
  uint64_t texture_flat_clear_entries[3] = {};
  uint64_t texture_flat_clear_capacity[3] = {};
  uint64_t texture_flat_clear_ns[3] = {};
  // Cached texture-binding lifetime checks after the global view epoch moves.
  uint64_t view_epoch_fast_hits = 0, view_alive_batches = 0;
  uint64_t view_alive_queries = 0, view_alive_failures = 0;
  uint64_t view_alive_revalidated = 0, view_alive_oversized = 0;
  uint64_t view_alive_probes = 0, view_alive_probe_max = 0;
  uint64_t vertex_change_paths[9] = {};
  // setup, index bounds, vertex ranges, vertex hashes, texture bindings.
  uint64_t flat_clear_count[5] = {};
  uint64_t flat_clear_entries[5] = {};
  uint64_t flat_clear_capacity[5] = {};
  uint64_t flat_clear_ns[5] = {};
#endif
  uint64_t texture_placeholders = 0;  // cod5-nx
  uint64_t texture_hash_ns = 0, texture_create_ns = 0, texture_creates = 0;  // cod5-nx
  uint64_t texture_source_snapshot_ns = 0, texture_source_snapshot_bytes = 0;
  uint64_t texture_source_snapshot_jobs = 0;
  uint64_t volume_uploads=0,volume_bytes=0,volume_conversion_ns=0,volume_plan_ns=0,volume_retries=0;
  uint64_t textures_replaced = 0;
  uint64_t vertex_bytes_copied = 0;
  uint64_t vertex_bytes_whole = 0;  // subset copied by the conservative full-buffer fallback
  uint64_t vertex_copy_ns = 0;
  uint64_t vertex_hash_ns = 0;
  uint64_t vertex_write_hits=0,vertex_write_compared=0,vertex_write_mismatches=0,vertex_write_busy=0;
  uint64_t render_passes = 0;
#if COD6_DIAGNOSTICS
  // Switch NVK ZCULL candidate for the fixed playable-scene depth target.
  uint64_t scene_zcull_targets = 0;
  uint64_t scene_zcull_growth_fallbacks = 0;
  uint64_t scene_zcull_pass_clears = 0;
#endif
  uint64_t submissions = 0;
  uint64_t stream_rollovers_during_draw = 0;  // diagnostic: allocations forced a submit before recording this draw
  // Renderer wall time on the ring thread (draws, resolves, swaps), including
  // blocking calls. This is not scheduled CPU execution time. GPU
  // time of the completed submissions (raw timestamp ticks times the
    // reported period). The period has not been independently calibrated on
    // this device; CPU/GPU overlap and different measurement windows also mean
    // these times must not be added or used to infer a clock scale factor.
  uint64_t cpu_ns = 0;
  // Wall time inside RefreshGuestOutput, including the native refresher and
  // synchronous presentation/back-pressure. Subset of cpu_ns, not extra work.
  uint64_t presentation_call_ns = 0;
  // cod5-nx: ring time breakdown (cumulative).
  uint64_t texture_lookup_ns = 0;
  uint64_t memexport_ns = 0;
  uint64_t memexport_copies = 0;
  uint64_t gpu_ns = 0;
  uint64_t gpu_submissions = 0;
  // Frames (swap to swap on the ring thread) longer than 50 and 100 ms, and
  // what those frames did.
  uint64_t frames_over_50ms = 0;
  uint64_t frames_over_100ms = 0;
  uint64_t slow_frame_pipelines = 0;
  uint64_t slow_frame_shaders = 0;
  uint64_t slow_frame_uploads = 0;
  uint64_t slow_frame_render_targets = 0;
  uint64_t render_targets_created = 0;
  // How often the cached draw state and texture bindings had to be rebuilt.
  uint64_t setups_built = 0;
  uint64_t texture_bindings_built = 0;
  // Bounded dependency memo for the fetch-word hash used by texture bindings.
  uint64_t binding_key_enabled = 0;
  uint64_t binding_key_queries = 0, binding_key_memo_hits = 0, binding_key_hashes = 0;
  uint64_t binding_key_slot_mask_scans = 0, binding_key_collisions = 0;
  // COD6_V303_BINDCHECK: draws whose shader samples a fetch slot the texture
  // binding key did not cover. Non-zero means two draws can share a descriptor
  // set while pointing at different textures in that slot - an object wearing
  // another object's texture, with no pixel comparison needed to see it.
  uint64_t binding_key_mask_checked = 0, binding_key_mask_missing = 0;
  // System constant dependency identity and final-content reuse.
  uint64_t system_constant_stable_setup = 0;
  uint64_t system_constant_queries = 0, system_constant_input_hits = 0;
  uint64_t system_constant_recomputes = 0, system_constant_content_hits = 0;
  uint64_t system_constant_uploads = 0;
  uint64_t resolve_swizzle_changes = 0;
  uint64_t resolve_answer_checks = 0, resolve_answer_reuses = 0, resolve_answer_fallbacks = 0;
#if COD6_DIAGNOSTICS
  // Successful resolve writes that were exact repeats of a previous
  // successful write. No GPU command is skipped by this counter.
  uint64_t resolve_repeat_observations = 0, resolve_repeat_candidates = 0;
  uint64_t resolve_repeat_collisions = 0;
  uint64_t resolve_repeat_path_observations[6] = {};
  uint64_t resolve_repeat_path_candidates[6] = {};
  // COD4-style adjacent rigid/static instance candidates. These are
  // record-only, content-hash-based diagnostics; rendering is unchanged.
  uint64_t instance_candidate_draws = 0, instance_candidate_eligible = 0;
  uint64_t instance_candidate_groups = 0, instance_candidate_reducible = 0;
  uint64_t instance_candidate_changed = 0, instance_candidate_same = 0;
  uint64_t instance_break_context = 0, instance_break_no_instance = 0;
  uint64_t instance_break_pipeline_state = 0, instance_break_geometry = 0;
  uint64_t instance_break_textures = 0, instance_break_fixed_constants = 0;
  uint64_t instance_break_dynamic_state = 0, instance_candidate_max_group = 0;
  uint64_t instance_batching_enabled = 0;
  uint64_t instance_runtime_guest_draws = 0, instance_runtime_host_draws = 0;
  uint64_t instance_runtime_groups = 0, instance_runtime_singles = 0;
  uint64_t instance_runtime_max_group = 0;
#endif
  // Batched recorder wall-time totals (including scheduling), not GPU time.
  uint64_t record_queued=0,record_publications=0,record_wakes=0,record_capacity_waits=0,record_drains=0;
  uint64_t record_executed=0,record_batches=0,record_execute_ns=0;
  uint64_t record_capacity_wait_ns=0,record_drain_wait_ns=0;
  // Where the last frame's time went on the ring thread, for reporting slow
  // frames: waits for texture untiling, for the GPU (a submission slot or
  // stream space), for the draw recording thread, and time spent finding
  // textures; set at each swap.
  uint64_t frame_upload_wait_ns = 0;
  uint64_t frame_fence_wait_ns = 0;
  uint64_t frame_drain_wait_ns = 0;
  uint64_t frame_texture_ns = 0;
  uint64_t frame_draws = 0;
  // The part of frame_fence_wait_ns spent waiting for stream space.
  uint64_t frame_stream_wait_ns = 0;
  // Draws skipped, by reason (see SkipReason in the .cpp).
  uint64_t skipped_by_reason[16] = {};
};

#if COD6_DIAGNOSTICS
struct RendererReportSnapshot {
  static constexpr size_t kGpuProfileCapacity = 14;
  struct ShadowEntry {
    uint64_t key[10] = {};
    uint64_t count = 0;
  };
  struct ShadowGroup {
    ShadowEntry entries[16] = {};
    size_t size = 0;
    uint64_t total = 0, omitted = 0;
  };
  TextureUploadReportSnapshot texture_uploads;
  GpuProfileSnapshot<kGpuProfileCapacity> gpu_profile;
  NvkDriverReportSnapshot nvk_driver;
  ShadowGroup shadows[4];
  uint32_t state_change_counts[0x4000] = {};
  bool collect_diagnostics = false;
};
#endif

class Renderer {
 public:
  Renderer();
  ~Renderer();

  bool Initialize(rex::ui::vulkan::VulkanProvider* provider, rex::memory::Memory* memory,
                  rex::graphics::RegisterFile* regs);
  void Shutdown();
  // Transfer single-producer ownership after initialization or ring join.
  // Caller must ensure the previous producer can no longer issue commands.
  void AdoptRecorderProducer();

  // PM4_IM_LOAD / IM_LOAD_IMMEDIATE: the microcode in guest byte order.
  // guest_address is where the microcode lives (0 when it came in the packet).
  void LoadShader(rex::graphics::xenos::ShaderType type, const uint32_t* ucode_big_endian,
                  uint32_t dword_count, uint32_t guest_address);

  // A draw in the color/depth EDRAM modes. index_words holds the indices of an
  // immediate-index draw (DRAW_INDX with the indices in the packet), already in
  // host byte order; null otherwise.
  void Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words);

  // A draw in the copy EDRAM mode: resolve and/or clear.
  void Resolve();

  // PM4_XE_SWAP. Presents the texture resolved at the frontbuffer address.
  void Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer_address, uint32_t width,
            uint32_t height);

  // Loads the driver's compiled pipelines saved by an earlier session, and
  // saves them there as new ones appear. Call before the guest starts.
  void InitializeDriverCache(const std::filesystem::path& cache_root, uint32_t title_id);
  // Loads the SPIR-V translated by earlier sessions from the same directory (called by InitializeDriverCache).
  void InitializeSpirvCache(const std::filesystem::path& directory, uint32_t title_id);
  // Compiles, on the pipeline worker, the pipelines earlier sessions made (called by InitializeDriverCache).
  void PrewarmPipelines(const std::filesystem::path& directory, uint32_t title_id);

  // The 256-entry gamma ramp, as the DC_LUT_30_COLOR register packs it.
  void SetGammaRamp256(const uint32_t* entries);

  const RendererStats& stats() const { return stats_; }
  // Lets the draw recording thread catch up: the ring thread is about to wait.
  void KickRecorder();
  // Submits what has been recorded so far, so the GPU works while the ring
  // thread waits (the frame rate cap holds a frame before presenting it).
  void Flush();
#if COD6_DIAGNOSTICS
  // The producer only copies/resets bounded POD state. The consumer owns all
  // allocation, sorting, formatting and logging and never dereferences this renderer.
  void CaptureReportSnapshot(RendererReportSnapshot& snapshot);
  static void FormatReportSnapshot(const RendererReportSnapshot& snapshot);
#endif
  // Ring-thread cache-publication boundary; does not submit or wait on the GPU.
  void GuestMemorySynchronized();

  // Called for every register the command ring writes, so constant uploads
  // can be reused while their registers stay unchanged.
  void RegisterWritten(uint32_t index, uint32_t old_value, uint32_t new_value) {
    if (index >= 0x4000) {
      if (index < 0x4400) {
        ++vs_float_generation_;
      } else if (index < 0x4800) {
        ++ps_float_generation_;
      } else if (index < 0x48C0) {
        uint32_t slot_bit = 1u << ((index - 0x4800) / 6);
        fetch_generations_.Written(slot_bit);
        texture_fetch_dirty_ |= slot_bit;
        vertex_fetch_dirty_ |= slot_bit;
      } else if (index < 0x4928) {
        ++bool_loop_generation_;
      } else {
        ++state_generation_;
      }
      return;
    }
    // Vertex index range and offset: system constants only.
    if (index >= 0x2100 && index <= 0x2102) {
      ++index_generation_;
      return;
    }
    // Registers written for every draw, or only read by resolves and the
    // ring, do not invalidate the render state the draws cache.
    if (index == 0x2103 || (index >= 0x21F9 && index <= 0x21FC) ||
        (index >= 0x2318 && index <= 0x231F) || index == 0x2324 || index == 0x2325 ||
        index == 0x2293 || (index >= 0x0578 && index <= 0x057F) || index == 0x01DC ||
        index == 0x01DD || (index >= 0x0A2F && index <= 0x0A31) || index == 0x0C44 ||
        index == 0x0C45 || index == 0x01C5) {
      return;
    }
    ++state_generation_;
#if COD6_DIAGNOSTICS
    if (collect_diagnostics_) {
      ++state_change_counts_[index];
    }
#endif
    // The render state is the context block; its hash is kept up to date one
    // register at a time (each register contributes Mix(index, value), and
    // an XOR swaps the old contribution for the new one).
    if (index >= 0x2000 && index < 0x2400) {
      state_hash_ ^= Mix(index, old_value) ^ Mix(index, new_value);
    }
  }

  // A block of shader constant registers (0x4000-0x4927) written: the same
  // bookkeeping as RegisterWritten for each changed one, done once.
  void ConstantsWritten(bool vs_float, bool ps_float, uint32_t fetch_slots, bool bool_loop) {
    if (vs_float) ++vs_float_generation_;
    if (ps_float) ++ps_float_generation_;
    if (fetch_slots) {
      fetch_generations_.Written(fetch_slots);
      texture_fetch_dirty_ |= fetch_slots;
      vertex_fetch_dirty_ |= fetch_slots;
    }
    if (bool_loop) ++bool_loop_generation_;
  }

  static uint64_t Mix(uint32_t index, uint32_t value) {
    if (!value) {
      return 0;
    }
    uint64_t x = (uint64_t(index) << 32) | value;
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
  }

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  RendererStats stats_;

 public:
  // Bumped by every register write that can change how a draw renders.
  uint64_t state_generation_ = 1;
  // Hash of the render state registers (all zero at the start: hash 0).
  uint64_t state_hash_ = 0;
  // Test-only register-change histogram. Production has neither the hot-path
  // increment nor the 64 KiB table.
#if COD6_DIAGNOSTICS
  bool collect_diagnostics_ = false;
  uint32_t state_change_counts_[0x4000] = {};
#endif
  // Fetch constant slots (6 registers each) written since the renderer last
  // looked at them, for textures and for vertex buffers.
  uint32_t texture_fetch_dirty_ = ~0u;
  uint32_t vertex_fetch_dirty_ = ~0u;
  uint64_t index_generation_ = 1;
  uint64_t vs_float_generation_ = 1;
  uint64_t ps_float_generation_ = 1;
  FetchGenerations fetch_generations_;
  uint64_t bool_loop_generation_ = 1;
};

}  // namespace cod6::native
