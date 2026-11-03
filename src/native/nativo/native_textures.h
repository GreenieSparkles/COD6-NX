// SPDX-License-Identifier: GPL-3.0-or-later
//
// Textures for the native renderer (see native_renderer.h).
//
// Guest textures are untiled and converted on the CPU into a staging buffer and
// copied into an image by the setup command buffer, which runs before the draws
// of its submission. An image is identified by its fetch-constant layout and a
// hash of its guest data, so a texture whose memory changes becomes a different
// image, and an image never changes while a submission may be reading it: that
// is what lets every upload be hoisted out of the draw command buffer, which
// then never has to break a render pass to upload.
//
// Resolve destinations are images the renderer copies render targets into.
// They are registered by guest address, and a texture fetched from that address
// uses them instead of the guest memory, which the native renderer never writes.
//
// The formats, the view swizzles and the sampler parameters follow ReXGlue's
// VulkanTextureCache (itself from Xenia, BSD-3-Clause, see LICENSES/), which is
// what the translated shaders are written against.

#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cod6_diagnostics.h"

#include <rex/graphics/pipeline/shader/spirv.h>
#include <rex/graphics/xenos.h>
#include <rex/ui/vulkan/device.h>

namespace rex::memory {
class Memory;
}

namespace cod6::native {

#if COD6_DIAGNOSTICS
struct TextureUploadReportRecord {
  uint32_t address = 0, format = 0, width = 0, height = 0, count = 0;
  uint64_t bytes = 0, last_hash = 0;
};

struct TextureUploadReportSnapshot {
  static constexpr size_t kCapacity = 4097;
  TextureUploadReportRecord records[kCapacity] = {};
  size_t count = 0;
  bool layout_diagnostics = false;
  bool layout_extents = false;
  uint64_t layout_queries = 0, layout_hits = 0, layout_builds = 0;
};
#endif

// Where the texture cache puts upload data and records the copies.
struct UploadTarget {
  // Returns a CPU pointer for `size` bytes aligned to `alignment`, and their
  // offset in `buffer`. Null if the arena is exhausted.
  std::function<uint8_t*(size_t size, size_t alignment, VkDeviceSize& offset_out)> allocate;
  VkBuffer buffer = VK_NULL_HANDLE;
  // Records into the command buffer that runs before the submission's draws.
  VkCommandBuffer setup_commands = VK_NULL_HANDLE;
  // The submission being recorded (increasing).
  uint64_t submission = 0;
};

// An image a resolve writes into. Owned by the texture cache.
struct ResolveDestination {
  uint32_t address = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  // Plain 2D view, for presenting.
  VkImageView view = VK_NULL_HANDLE;
  // Guest components (XYZW, as a texture fetch sees the resolved memory) in
  // terms of the host components (xenos swizzle encoding, 3 bits each).
  uint32_t host_swizzle = 0;
  // Views as textures (2D arrays), by host swizzle; bit 31 set for plain 2D.
  std::unordered_map<uint32_t, VkImageView> texture_views;
  uint64_t last_written_frame = 0;
  // Copies of the top-left corner at the sizes the game samples this address
  // as, where that differs from the image. Texture coordinates span the whole
  // image, so sampling the image itself scaled the content: the blur the game
  // makes from a 264x152 resolve into a 320-wide destination came back 1.2
  // times too small, a ghost of the skater up and to the left with a halo.
  // Refreshed by every resolve here (CopyToSized).
  struct Sized {
    uint32_t width = 0;
    uint32_t height = 0;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkImageView> views;
    // Plain 2D view for the compute copy into it (null: copied by transfer).
    VkImageView storage_view = VK_NULL_HANDLE;
    bool filled = false;
  };
  std::vector<std::unique_ptr<Sized>> sized;
  // cod5-nx: the image as a 3D texture, its slices stacked vertically in the resolved image (COD5 resolves its
  // 32x32x32 color grading table as a 32x1024 surface and samples it as a volume). Kept by address in the texture
  // cache (it outlives replaced destinations); refilled by every resolve of the same format there.
  struct Volume {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkImageView> views;
    bool filled = false;
    // Copy regions from the resolved image, from the guest tiling of both: on the console the 2D resolve and the
    // 3D fetch address the same bytes (COD5's table maps in 16x8 blocks, scrambled). Empty: slices stacked.
    std::vector<VkImageCopy> regions;
  };
};

struct TextureCacheStats {
  uint64_t uploads = 0;
  uint64_t created = 0;
  uint64_t streamed = 0;
  uint64_t upload_bytes = 0;
  uint64_t hash_bytes = 0;
  uint64_t unsupported = 0;
  uint64_t resolved_hits = 0;
  uint64_t resolve_swizzle_changes = 0; // test-only increments; existing images with changed channel mapping
  uint64_t resolve_answer_checks = 0, resolve_answer_reuses = 0, resolve_answer_fallbacks = 0;
  uint64_t live_textures = 0;
  uint64_t texture_memory_bytes = 0;  // cod5-nx: counted textures, refreshed every 60 frames
  uint64_t budget_evictions = 0;      // cod5-nx: textures freed by the memory budget
#if COD6_DIAGNOSTICS
  // Record-only evidence for the existing 60-frame budget/age eviction pass.
  // These counters never choose or suppress an eviction.
  uint64_t eviction_checks = 0, eviction_over_budget = 0;
  uint64_t eviction_entries_scanned = 0, eviction_eligible = 0, eviction_sorted = 0;
  uint64_t budget_evicted_bytes = 0, age_evictions = 0, age_evicted_bytes = 0;
  uint64_t eviction_scan_ns = 0, eviction_candidate_ns = 0, eviction_sort_ns = 0;
  uint64_t eviction_release_ns = 0, eviction_age_scan_ns = 0;
  uint64_t pool_dedicated = 0, pool_no_fit = 0, pool_slabs_failed = 0;
  // ChangeDetector path mix: trusted skip, sample check/hit, full check,
  // initial/unchanged/changed, sampled input bytes and full input bytes.
  uint64_t change_paths[9] = {};
  // FlatMap clear evidence for checked layouts, memoized answers and view ids.
  uint64_t flat_clear_count[3] = {};
  uint64_t flat_clear_entries[3] = {};
  uint64_t flat_clear_capacity[3] = {};
  uint64_t flat_clear_ns[3] = {};
#endif
  uint64_t placeholders = 0;          // cod5-nx: lookups answered with a predecessor while building
  uint64_t hash_ns = 0;               // cod5-nx: time hashing guest texture data
  uint64_t create_ns = 0;             // cod5-nx: time creating images on the ring thread
  uint64_t creates_on_ring = 0;       // cod5-nx
  uint64_t source_snapshot_ns = 0;
  uint64_t source_snapshot_bytes = 0;
  uint64_t source_snapshot_jobs = 0;
  uint64_t volume_uploads=0,volume_bytes=0,volume_conversion_ns=0,volume_plan_ns=0,volume_retries=0;
  // Lookups answered with the empty texture: still streaming in, or a format
  // or dimension not handled (logged once each).
  uint64_t pending = 0;
  // Textures whose content changed after a draw of this frame had used them.
  uint64_t replaced = 0;
};

class TextureCache {
 public:
  // Added to a depth resolve's address for the image holding what the resolve
  // leaves in guest memory, (depth24 << 8) | stencil per texel: fetches of
  // that address as anything but a depth format get it. Resolve addresses are
  // page aligned, so the tag never names a real one.
  static constexpr uint32_t kDepthWordsTag = 1;

  TextureCache();
  ~TextureCache();

  bool Initialize(const rex::ui::vulkan::VulkanDevice* device, rex::memory::Memory* memory);
  void Shutdown();

  // Frame boundary: textures not used for a while are destroyed once the GPU
  // is done with them. completed_frame is the last frame whose work finished.
  void BeginFrame(uint64_t frame, uint64_t completed_frame);

  // Records the null images' first transition. Called once, before any use.
  void InitializeNullImages(VkCommandBuffer commands);

  // The view for a translated shader's texture binding. Never null: a texture
  // that cannot be made gives a transparent black one.
  VkImageView GetView(const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch,
                      rex::graphics::xenos::FetchOpDimension dimension, bool is_signed,
                      UploadTarget& upload);

  // About the view the last GetView returned: the texture behind it (null for
  // a resolve or the empty texture), and the last frame through which that
  // answer stays the same while the fetch constant does.
  void* last_texture() const;
  // cod5-nx debug: the image and host format of a texture last_texture() returned.
  static VkImage DebugTextureImage(const void* texture, VkFormat* format);
  uint64_t last_valid_until() const;
  // Marks a texture as used by this submission and frame, as GetView does:
  // what frees textures and rewrites them in place goes by it.
  void Touch(void* texture, uint64_t submission);

  VkSampler GetSampler(const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch,
                       const rex::graphics::SpirvShader::SamplerBinding& binding);

  // Finds or makes the image a resolve at this address writes into; recreated
  // if the size or the format changes. commands receives the layout
  // transition of a new image.
  ResolveDestination* GetResolveDestination(uint32_t address, uint32_t width, uint32_t height,
                                            VkFormat format, uint32_t host_swizzle,
                                            VkCommandBuffer commands);
  const ResolveDestination* FindResolveDestination(uint32_t address) const;
  // After a resolve wrote dest (transfer writes recorded in commands): brings
  // its sized copies up to date.
  // cod5-nx: see ResolveDestination::Volume::regions.
  static std::vector<VkImageCopy> VolumeRegions(const ResolveDestination& dest, uint32_t w, uint32_t h, uint32_t depth,
                                                bool tiled, uint32_t pitch);
  void CopyToSized(ResolveDestination& dest, VkCommandBuffer commands);
  // cod5-nx: a view of a texture (opaque Texture*) for a fetch.
  VkImageView ViewOf(void* texture, const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch, bool is_signed);
  // cod5-nx: the predecessor of a texture still being built (see Impl::placeholders), or null.
  VkImageView Placeholder(uint64_t signature, const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch,
                          bool is_signed, uint64_t except_key);
  // cod5-nx: an uncompressed 3D texture from guest memory (null if not handled).
  VkImageView GetVolumeView(const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch, VkFormat host_format,
                            uint32_t host_format_swizzle, uint32_t width, uint32_t height, uint32_t depth,
                            uint32_t base_page, UploadTarget& upload, bool as_2d_array);
  // cod5-nx: textures reading [start, start + bytes) check their data in full on their next use.
  void InvalidateRange(uint32_t start, uint32_t bytes);
  // Copies a rectangle between two views on the GPU without vkCmdCopyImage,
  // barriers included (the renderer's compute copy); false if it cannot.
  using ComputeCopy = std::function<bool(VkCommandBuffer commands, VkImageView source,
                                         VkImageView dest, int32_t source_x, int32_t source_y,
                                         int32_t dest_x, int32_t dest_y, uint32_t width,
                                         uint32_t height)>;
  void SetComputeCopy(ComputeCopy copy);
  // Whether images of this format can be written by the compute copy.
  bool StorageCapable(VkFormat format);

  // Whether a depth resolve to this address has been sampled as anything but
  // a depth format, so it needs the guest memory words (kDepthWordsTag).
  bool WantsDepthWords(uint32_t address) const;
#if COD6_DIAGNOSTICS
  // Ring producer: copies then clears the bounded interval records. Sorting,
  // formatting and logging are consumer work and never touch TextureCache.
  void TakeUploadReportSnapshot(TextureUploadReportSnapshot& snapshot);
  static std::string FormatUploadReport(const TextureUploadReportSnapshot& snapshot);
#endif
  // A plain 2D view of a resolve destination with a host swizzle, for the
  // swap (which samples a non-array image).
  VkImageView GetResolveDestinationView2D(uint32_t address, uint32_t guest_swizzle,
                                          uint32_t* width_out, uint32_t* height_out);

  // Untiling runs on a worker: call before submitting the command buffers
  // that copy from the staging data.
  void WaitForUploads();

  const TextureCacheStats& stats() const { return stats_; }
  // Changes whenever image views are destroyed, so descriptor sets cached by
  // view handle must be dropped (a new view may reuse a handle).
  uint64_t views_epoch() const;
  // An id for a view that is never given to another: a handle can be reused
  // once its view is destroyed, an id cannot, so descriptor sets cached by id
  // stay right without being dropped whenever any view is destroyed.
  uint64_t ViewId(VkImageView view);
  // Whether the view with this id still exists.
  bool ViewAlive(VkImageView view, uint64_t id
#if COD6_DIAGNOSTICS
                 , uint64_t* probes = nullptr
#endif
                 ) const;
  // Changes when what GetView answers for some fetch may have changed other
  // than by its own checks (a resolve destination made, a texture replaced).
  uint64_t answers_epoch() const;
  // Diagnostic dependency token for resolve fanout images (exact-size and
  // volume copies). Global by design: unrelated changes only lose candidates.
  uint64_t resolve_fanout_generation() const;
  // Only bypass a moved epoch when every intervening change is a recorded
  // resolve at an address not sampled by these fetch slots. Other lifetime,
  // frame, submission and content-deadline checks remain the caller's job.
  bool CanReuseAnswers(uint64_t previous, const uint32_t* fetch_words, uint32_t slot_mask);

 private:
  VkImageView GetViewUncached(const rex::graphics::xenos::xe_gpu_texture_fetch_t& fetch,
                              rex::graphics::xenos::FetchOpDimension dimension, bool is_signed,
                              UploadTarget& upload, VkImageView null_view);
  struct Texture;
  struct Impl;
  std::unique_ptr<Impl> impl_;
  TextureCacheStats stats_;
};

}  // namespace cod6::native
