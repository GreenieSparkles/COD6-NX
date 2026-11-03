// SPDX-License-Identifier: GPL-3.0-or-later
//
// Native renderer for the Switch port. See native_renderer.h for the design.
//
// Parts of this file are ports of ReXGlue's Vulkan backend (src/graphics/vulkan,
// itself from Xenia: Copyright Ben Vanik and the Xenia contributors, BSD-3-Clause,
// see LICENSES/): the system constants (VulkanCommandProcessor::
// UpdateSystemConstantValues), the dynamic state (UpdateDynamicState), the
// shader modifications and pipeline state (VulkanPipelineCache), and the resolve
// rectangle (draw_util::GetResolveInfo). They are what the translated shaders
// expect, so they are kept as close to the original as the native design allows;
// everything about EDRAM, fragment shader interlock and tessellation is gone.

#include "native_renderer.h"
#include "native_descriptor_writes.h"
#include "native_diagnostic_buckets.h"
#include "native_fetch_generations.h"
#include "native_msaa_alias.h"
#include "native_nvk_draw.h"
#include "native_batch_recorder.h"
#include "native_binding_key_memo.h"
#if COD6_INSTANCE_CONSTANTS
#include "native_instance_group.h"
#endif
#if COD6_DIAGNOSTICS
#include "native_instance_candidates.h"
#endif
#include "native_resolve_repeat_tracker.h"
#include "native_scene_scale.h"
#include "native_system_constant_inputs.h"
#include "cod6_page_writes.h"
#include "cod6_diagnostics.h"

#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <cmath>
#include <map>
#include <tuple>
#include <mutex>
#include <unordered_set>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <cstring>
#include <unordered_map>
#include <vector>

#include <xxhash.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/logging/assert.h>
#include <rex/math.h>
#include <rex/memory/utils.h>
#include <rex/string/buffer.h>
#include <rex/graphics/pipeline/shader/spirv_translator.h>
#include <rex/graphics/pipeline/texture/util.h>
#include <rex/graphics/registers.h>
#include <rex/graphics/util/draw.h>
#include <rex/graphics/vulkan/shader.h>
#include <rex/kernel/xboxkrnl/video.h>
#include <rex/system/xmemory.h>
#include <rex/system/xvideo.h>
#include <rex/ui/presenter.h>
#include <rex/ui/vulkan/presenter.h>
#include <rex/ui/vulkan/provider.h>
#include <rex/ui/vulkan/instance.h>
#include <rex/ui/vulkan/util.h>

#if defined(__SWITCH__)
extern "C" {
#include <switch/kernel/svc.h>
}
#endif

#include "native_geometry_shaders.h"
#include "native_hash.h"
#include "native_textures.h"

#ifndef COD6_INSTANCE_CONSTANTS
#define COD6_INSTANCE_CONSTANTS 0
#endif

// project8-sw's SDK can compile texture signs into each pipeline (gpu_specialize_texture_signs, off by default); our
// SDK's translator can't, so the signs are always selected at run time.
REXCVAR_DECLARE(std::string, cod6_debug_capture_dir);
REXCVAR_DECLARE(uint32_t, cod6_debug_capture_every);
REXCVAR_DECLARE(uint32_t, cod6_debug_tex_album);
REXCVAR_DECLARE(uint32_t, cod6_debug_album_frames);
REXCVAR_DECLARE(uint32_t, cod6_debug_album_skip);
REXCVAR_DECLARE(uint32_t, cod6_debug_draw_table);
REXCVAR_DECLARE(uint32_t, cod6_debug_draw_table_skip);
REXCVAR_DECLARE(bool, cod6_native_bind_check);
REXCVAR_DECLARE(std::string, log_network);
REXCVAR_DECLARE(bool, cod6_native_diagnostics);
REXCVAR_DEFINE_STRING(cod6_debug_dump_host, "", "COD5",
                      "Debug: send trace image dumps to this ip:port over TCP (tools/dump_receiver.py) instead of "
                      "writing them to cod6_debug_capture_dir; for the Switch");
REXCVAR_DEFINE_UINT32(cod6_debug_trace_frame, 0, "COD5",
                      "Debug: log every render pass, resolve and skipped draw of this frame number (0 = off)");
REXCVAR_DEFINE_UINT32(cod6_debug_trace_after_s, 0, "COD5",
                      "Debug: like cod6_debug_trace_frame, for the first frame this many seconds after start (0 = off)");
REXCVAR_DEFINE_UINT32(cod6_debug_dump_swaps, 0, "COD5",
                      "Debug: with cod6_debug_trace_after_s, also dump the presented image of this many consecutive "
                      "frames from the traced one (flicker hunting)");
REXCVAR_DEFINE_UINT32(cod6_native_resolve_exp_bias, 2, "COD5",
                      "Native renderer: apply the exponent bias of color resolves (0 = ignore it; 1 = 2^bias; 2 = "
                      "2^(bias+5) for 16_16 and 16_16_16_16 sources, as the SDK's resolve does)");
REXCVAR_DEFINE_BOOL(cod6_debug_mip_watch, false, "COD5",
                    "Debug: watch the mip data of large DXT textures (entropy at first sight and later changes)");
/* COD6_V238_SOMBRAS: descartar los dibujos dirigidos a un render target de
 * solo profundidad, es decir los mapas de sombra. Ahorra el pase entero y
 * quita las sombras de la imagen. */
/* COD6_V241_MEZCLA: descartar los dibujos con mezcla (humo, polvo, fogonazo,
 * calcomanias y tambien el HUD). Quita mucho relleno transparente. */
REXCVAR_DEFINE_BOOL(cod6_native_skip_blended_draws, false, "COD5",
                    "Native renderer: drop blended draws (particles, decals AND the HUD)");

/* COD6_V248_SALTAR_PS: descartar los dibujos cuyo pixel shader sea uno de la
 * lista. Sirve para apagar efectos concretos (fogonazo, casquillos, desenfoque)
 * sin tocar nada mas: se identifica el shader por su hash de microcodigo, que
 * sale en el trace de un fotograma. */
REXCVAR_DEFINE_STRING(cod6_native_skip_ps, "", "COD6",
                      "Lista separada por espacios de hashes de pixel shader (16 digitos hex) que se descartan");

REXCVAR_DEFINE_BOOL(cod6_native_skip_shadow_passes, false, "COD5",
                    "Native renderer: drop every draw into a depth-only render target (shadow maps)");

REXCVAR_DEFINE_BOOL(cod6_native_skip_aliased_depth, false, "COD5",
                    "Native renderer: skip depth-only draws into a depth target that lies inside a larger one with "
                    "the same pitch and is never resolved itself. Each EDRAM base is its own image here, so those "
                    "draws (the far half of COD5's 1024x2048 sun shadow map) never reached what the game resolves");
REXCVAR_DEFINE_BOOL(cod6_native_memexport_copy, false, "COD5",
                    "Native renderer: do COD5's GPU memory copy (a memexport vertex shader) on the CPU; its texture "
                    "streamer copies the low mips into place with it");
REXCVAR_DEFINE_BOOL(cod6_native_ranged_vertices, true, "COD5",
                    "Native renderer: copy only the vertices a draw reads (its index range) when the vertex "
                    "shader fetches by vertex index, instead of whole vertex buffers");
REXCVAR_DEFINE_BOOL(cod6_native_vertex_sync_cache, false, "COD5",
                    "Audit candidate: revalidate vertex/index copies after guest cache synchronization, including within a frame")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
/* COD6_V230_D24: por defecto ACTIVADO.
 * El juego usa buffers de profundidad de 24 bits en el nivel. Con 32 bits
 * flotantes la prueba de profundidad no da lo mismo y el mundo 3D se ve mal,
 * mientras la interfaz (que no usa profundidad) se ve perfecta.
 * Se activa por defecto porque, al ser kInitOnly y leerse dentro de Initialize(),
 * el valor del toml llega demasiado tarde: en la prueba 224 el toml decia true y
 * el renderer seguia leyendo false. Poniendolo en el codigo no depende del orden
 * de carga. Sigue siendo un cvar: para volver atras, ponlo a false en el toml. */
REXCVAR_DEFINE_BOOL(cod6_native_depth24, true, "COD5",
                    "Store depth in 32 bits per pixel (24-bit depth with stencil, or float "
                    "without) instead of 32-bit float with stencil, which NVIDIA GPUs keep "
                    "in 64")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
#if COD6_DIAGNOSTICS
REXCVAR_DEFINE_BOOL(cod6_native_gpu_profile, false, "COD5",
                    "Log GPU time by render pass and resolve, measured on one frame in 16");
#endif
REXCVAR_DEFINE_BOOL(cod6_native_record_thread, true, "COD5",
                    "Make the Vulkan calls of draws on a second thread, so the command ring "
                    "thread prepares the next draw meanwhile; false records draws synchronously")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_record_batch_publish, true, "COD5",
                    "With record_thread: publish/complete draws in batches and block when draining; "
                    "false keeps the original per-draw publication and polling recorder")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_binding_key_memo, true, "COD5",
                    "Native renderer: reuse the texture-binding fetch-word hash while its exact "
                    "shader, layout, frame, submission and guest/host fetch generations are unchanged")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_native_binding_key_memo_capacity, 256, "COD5",
                      "Native renderer: bounded texture binding-key memo slots; larger tables "
                      "reduce replacement misses without changing key equality")
    .allowed({"256", "1024", "4096"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_stable_setup_identity, true, "COD5",
                    "Native renderer: identify cached draw setups by their immutable entry identity "
                    "when deciding whether the system constant UBO inputs changed")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_msaa, false, "COD5",
                    "Render with the multisampling the game asks for; off draws every target "
                    "with one sample, which costs the GPU much less memory bandwidth");
REXCVAR_DEFINE_UINT32(cod6_native_scene_scale_percent, 100, "COD5",
                      "Render only the fixed COD5 1024x600 playable-scene EDRAM targets at this "
                      "percentage in each axis, then upscale resolves to the original guest size")
    .allowed({"50", "75", "100"})
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_scene_zcull, false, "COD5",
                    "Switch NVK candidate: omit TRANSFER_DST only from the fixed playable-scene "
                    "depth target and clear it with attachment loadOp=CLEAR so NVK may allocate "
                    "a persistent ZCULL plane")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_async_pipelines, true, "COD5",
                    "Native renderer: compile new pipelines on a worker thread and skip the "
                    "draws that need them until they are ready, instead of stalling the frame");
#if COD6_INSTANCE_CONSTANTS
REXCVAR_DEFINE_BOOL(cod6_native_instance_batching, false, "COD5",
                    "COD4-style strict adjacent rigid/static draw instancing using VS c4-c8; "
                    "disabled until the record-only candidate gate is accepted")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
#endif

namespace cod6::native {

/* COD6_PSHIST
 *
 * Para apagar efectos (humo, chispas, casquillos, fogonazos) hace falta saber
 * QUE pixel shader usa cada uno. cod6_native_skip_ps ya sabe descartar por hash
 * de microcodigo, pero hasta ahora los hashes se ponian a ojo y por eso la v248
 * rompio el renderizado.
 *
 * Esta tabla cuenta cuantas veces se dibuja cada pixel shader y apunta si algun
 * dibujo suyo era mezclado (los efectos casi siempre lo son). Se vuelca en el
 * resumen, con eso se pueden elegir los hashes con criterio.
 *
 * Solo la escribe el hilo del anillo, asi que no necesita cerrojo.
 */
struct PSHit {
  uint64_t hash;
  uint64_t draws;
  uint64_t blended;
};
constexpr uint32_t kPSHitSlots = 64;
PSHit g_ps_hits[kPSHitSlots] = {};
uint32_t g_ps_hit_count = 0;

void RexPSAnotar(uint64_t hash, bool blended) {
  for (uint32_t k = 0; k < g_ps_hit_count; ++k) {
    if (g_ps_hits[k].hash == hash) {
      ++g_ps_hits[k].draws;
      if (blended) ++g_ps_hits[k].blended;
      return;
    }
  }
  if (g_ps_hit_count < kPSHitSlots) {
    PSHit& h = g_ps_hits[g_ps_hit_count++];
    h.hash = hash;
    h.draws = 1;
    h.blended = blended ? 1 : 0;
  }
}


/* COD6_PRODUCER_NS
 *
 * Medido: 16,5 us por dibujo, y ExecuteDrawCommand solo son ~15 llamadas vkCmd*
 * (1-2 us). Los otros 14 us estan en el camino del productor: seguimiento de
 * estado, constantes, busqueda de pipeline, enlaces de textura. No habia forma
 * de medirlo por separado.
 *
 * Aqui se mide el hueco entre dos SubmitDraw consecutivos: eso ES el coste
 * completo de preparar un dibujo.
 */
uint64_t g_draw_gap_ns = 0;
uint64_t g_draw_gap_last = 0;
uint64_t g_draw_gap_count = 0;
uint64_t rex_draws_medidos() { return g_draw_gap_count; }
double rex_draws_ns() { return double(g_draw_gap_ns); }
double rex_draws_media_us() {
  return g_draw_gap_count ? double(g_draw_gap_ns) / 1e3 / double(g_draw_gap_count) : 0.0;
}
void rex_draws_reiniciar() { g_draw_gap_ns = 0; g_draw_gap_count = 0; }

namespace shaders {
#include "shaders/vulkan_spirv/apply_gamma_table_cs.h"
#include "shaders/depth_resolve_cs.h"
#include "shaders/depth_to_float_cs.h"
#include "shaders/copy_color_cs.h"
}  // namespace shaders

namespace {

namespace xenos = rex::graphics::xenos;
namespace reg = rex::graphics::reg;
namespace draw_util = rex::graphics::draw_util;
using rex::graphics::RegisterFile;
using rex::graphics::Shader;
using rex::graphics::SpirvShader;
using rex::graphics::SpirvShaderTranslator;
using rex::graphics::vulkan::VulkanShader;
using rex::ui::vulkan::VulkanDevice;

void CheckGpuResult(VkResult result, const char* operation) {
  if (result != VK_SUCCESS) {
    REX_FATAL("[native] {} failed with VkResult {}; GPU submission cannot continue safely",
              operation, int32_t(result));
  }
}

// cod5-nx: walks a vertex shader in program order and tells whether every vertex fetch reads the unmodified vertex
// index (r0.x, which the translator loads with the guest index of the vertex). Then a draw only reads the vertices
// between its lowest and highest index, and only that part of each vertex buffer has to be copied. CoD keeps large
// shared vertex buffers (up to 16 MB) that it partly rewrites every frame; copying them whole was ~330 MB per frame
// in the front end. Anything the walk can't follow (r0.x written or a jump, loop or call before a full fetch, an index
// from another register, relative addressing) makes the shader "unsafe", and its buffers are copied whole as before.
class VertexIndexAnalyzer : public rex::graphics::ShaderTranslator {
 public:
  bool safe() const { return safe_ && fetches_ > 0; }

 protected:
  void Reset() override {
    ShaderTranslator::Reset();
    safe_ = true;
    r0_written_ = false;
    flow_ = false;
    fetches_ = 0;
  }
  void ProcessLoopStartInstruction(const rex::graphics::ParsedLoopStartInstruction&) override { flow_ = true; }
  void ProcessLoopEndInstruction(const rex::graphics::ParsedLoopEndInstruction&) override { flow_ = true; }
  void ProcessCallInstruction(const rex::graphics::ParsedCallInstruction&) override { flow_ = true; }
  void ProcessReturnInstruction(const rex::graphics::ParsedReturnInstruction&) override { flow_ = true; }
  void ProcessJumpInstruction(const rex::graphics::ParsedJumpInstruction&) override { flow_ = true; }
  void ProcessVertexFetchInstruction(const rex::graphics::ParsedVertexFetchInstruction& instr) override {
    if (!instr.is_mini_fetch) {
      ++fetches_;
      const auto& index = instr.operands[0];
      if (r0_written_ || flow_ || instr.operand_count < 1 ||
          index.storage_source != rex::graphics::InstructionStorageSource::kRegister ||
          index.storage_index != 0 ||
          index.storage_addressing_mode != rex::graphics::InstructionStorageAddressingMode::kAbsolute ||
          index.is_negated || index.is_absolute_value ||
          index.GetComponent(0) != rex::graphics::SwizzleSource::kX) {
        safe_ = false;
      }
    }
    NoteResult(instr.result);
  }
  void ProcessTextureFetchInstruction(const rex::graphics::ParsedTextureFetchInstruction& instr) override {
    NoteResult(instr.result);
  }
  void ProcessAluInstruction(const rex::graphics::ParsedAluInstruction& instr,
                             uint8_t /*memexport_eM_potentially_written_before*/) override {
    NoteResult(instr.vector_and_constant_result);
    NoteResult(instr.scalar_result);
  }

 private:
  void NoteResult(const rex::graphics::InstructionResult& result) {
    if (result.storage_target == rex::graphics::InstructionStorageTarget::kRegister &&
        (result.original_write_mask & 0b0001) &&
        (result.storage_index == 0 ||
         result.storage_addressing_mode != rex::graphics::InstructionStorageAddressingMode::kAbsolute)) {
      // The fetch index is r0.x. Writes confined to y/z/w preserve it, even
      // when the destination register is relatively addressed. An x write
      // with a relative destination may alias r0 and remains unsafe.
      r0_written_ = true;
    }
  }
  bool safe_ = true;
  bool r0_written_ = false;
  bool flow_ = false;
  uint32_t fetches_ = 0;
};

// What the vertex copy needs to know about a vertex shader (cod5-nx).
struct VertexShaderRanges {
  bool index_safe = false;
  struct Binding {
    uint32_t fetch_constant;
    uint32_t stride_bytes;
    // Bytes past the start of a vertex that its attributes read (largest offset plus 16, the largest format).
    uint32_t vertex_extent;
    bool usable;  // no negative attribute offsets
  };
  std::vector<Binding> bindings;
};

// Four: with three, a single frame of heavy GPU work stalled the ring thread
// at once (30-49 ms waits for a free slot in frames that then dropped).
constexpr uint32_t kSubmissionSlots = 4;
// Vertex, index, constant and texture upload data of the submissions in flight.
constexpr VkDeviceSize kStreamSize = 160u << 20;
// Room at the end of the stream for the fixed ranges of constant bindings.
constexpr VkDeviceSize kStreamGuard = 16u << 10;
// Past the end of the stream, never allocated from but inside the range the
// shaders' vertex fetches may address: a fetch past its copied data (a guest
// index out of range) reads garbage there instead of faulting the GPU's MMU,
// which loses the device for good.
constexpr VkDeviceSize kStreamSlack = 16u << 20;
// Sizes of the constant buffer bindings. The float ones are bound with their
// maximum size and filled up to the shader's count.
constexpr VkDeviceSize kFloatConstantsBytes = 256 * 4 * sizeof(float);
constexpr VkDeviceSize kBoolLoopConstantsBytes = sizeof(uint32_t) * (8 + 32);
constexpr VkDeviceSize kFetchConstantsBytes = sizeof(uint32_t) * 6 * 32;

VkFormat ColorFormatToVk(xenos::ColorRenderTargetFormat format) {
  using F = xenos::ColorRenderTargetFormat;
  switch (format) {
    case F::k_8_8_8_8:
    case F::k_8_8_8_8_GAMMA:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case F::k_2_10_10_10:
    case F::k_2_10_10_10_AS_10_10_10_10:
      return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case F::k_2_10_10_10_FLOAT:
    case F::k_2_10_10_10_FLOAT_AS_16_16_16_16:
    case F::k_16_16_16_16_FLOAT:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case F::k_16_16:
      return VK_FORMAT_R16G16_SNORM;
    case F::k_16_16_16_16:
      return VK_FORMAT_R16G16B16A16_SNORM;
    case F::k_16_16_FLOAT:
      return VK_FORMAT_R16G16_SFLOAT;
    case F::k_32_FLOAT:
      return VK_FORMAT_R32_SFLOAT;
    case F::k_32_32_FLOAT:
      return VK_FORMAT_R32G32_SFLOAT;
    default:
      return VK_FORMAT_R8G8B8A8_UNORM;
  }
}

bool IsColorFormat64bpp(xenos::ColorRenderTargetFormat format) {
  using F = xenos::ColorRenderTargetFormat;
  return format == F::k_16_16_16_16 || format == F::k_16_16_16_16_FLOAT ||
         format == F::k_32_32_FLOAT;
}

// Depth buffers are D32_SFLOAT_S8_UINT unless cod6_native_depth24 stores the
// game's unorm 24-bit ones as D24_UNORM_S8_UINT and its float 24-bit ones as
// D32_SFLOAT (see DepthFormatFor). A resolve turns depth into the R32_SFLOAT
// the translated shaders sample k_24_8 as, and into the guest's memory words
// (see Resolve).
constexpr VkFormat kDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

VkBlendFactor BlendFactorToVk(xenos::BlendFactor factor) {
  switch (uint32_t(factor)) {
    case 0:
      return VK_BLEND_FACTOR_ZERO;
    case 1:
      return VK_BLEND_FACTOR_ONE;
    case 4:
      return VK_BLEND_FACTOR_SRC_COLOR;
    case 5:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case 6:
      return VK_BLEND_FACTOR_SRC_ALPHA;
    case 7:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case 8:
      return VK_BLEND_FACTOR_DST_COLOR;
    case 9:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case 10:
      return VK_BLEND_FACTOR_DST_ALPHA;
    case 11:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case 12:
      return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case 13:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case 14:
      return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case 15:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case 16:
      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    default:
      return VK_BLEND_FACTOR_ZERO;
  }
}

VkBlendOp BlendOpToVk(xenos::BlendOp op) {
  switch (uint32_t(op)) {
    case 1:
      return VK_BLEND_OP_SUBTRACT;
    case 2:
      return VK_BLEND_OP_MIN;
    case 3:
      return VK_BLEND_OP_MAX;
    case 4:
      return VK_BLEND_OP_REVERSE_SUBTRACT;
    default:
      return VK_BLEND_OP_ADD;
  }
}

struct RenderTargetKey {
  uint32_t base_tiles;
  uint32_t pitch;
  uint32_t format;
  uint32_t msaa;
  uint32_t is_depth;
  bool operator==(const RenderTargetKey& o) const { return !std::memcmp(this, &o, sizeof(o)); }
};
struct RenderTargetKeyHasher {
  size_t operator()(const RenderTargetKey& k) const { return size_t(XXH3_64bits(&k, sizeof(k))); }
};

struct RenderTarget {
  RenderTargetKey key;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  // Depth only, for the compute pass that resolves depth.
  VkImageView sample_view = VK_NULL_HANDLE;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
  uint32_t width = 0;
  uint32_t height = 0;
  // Guest sample modes that have addressed this host image. The host may be
  // forced to 1x, but EDRAM aliasing and clear/downsample semantics still use
  // the Xbox 360 sample mode.
  GuestMsaaModes guest_msaa_modes;
  uint64_t last_used_frame = 0;
  uint64_t last_drawn_frame = 0;  // cod5-nx: last frame a pass rendered into it
  bool ever_resolved = false;     // cod5-nx: a resolve has read it (see cod6_native_skip_aliased_depth)
  // COD6_V240_SOMBRAS: este objetivo de profundidad se ha usado en un pase que
  // tambien tenia color, es decir es la profundidad de la escena y no un mapa de
  // sombra. Nunca se descarta.
  bool used_with_color = false;
  bool transfer_dst = true;
  bool scene_zcull = false;
#if COD6_DIAGNOSTICS
  // A unique lifetime plus a conservative version of every possible write.
  // Used only to count strict repeat-resolve opportunities; never skips work.
  uint64_t diagnostic_identity = 0;
  uint64_t diagnostic_content_generation = 0;
#endif
};

// The attachments of a rendering scope: color 0-3 and depth.
struct PassTargets {
  RenderTarget* color[4] = {};
  RenderTarget* depth = nullptr;
  bool operator==(const PassTargets& o) const {
    return !std::memcmp(this, &o, sizeof(o));
  }
  bool Empty() const { return !color[0] && !color[1] && !color[2] && !color[3] && !depth; }
};

// Everything a graphics pipeline depends on, hashed as plain memory.
struct PipelineKey {
  uint64_t vs_hash;
  uint64_t vs_modification;
  uint64_t ps_hash;
  uint64_t ps_modification;
  uint32_t color_formats[4];
  uint32_t depth_format;
  uint32_t samples;
  uint32_t topology;
  uint32_t geometry_shader;
  uint32_t primitive_restart;
  uint32_t cull_mode;
  uint32_t front_face_clockwise;
  uint32_t polygon_line;
  uint32_t depth_clamp;
  uint32_t depth_test;
  uint32_t depth_write;
  uint32_t depth_compare;
  uint32_t stencil_test;
  uint32_t stencil_front[4];
  uint32_t stencil_back[4];
  uint32_t blend[4];
  uint32_t write_masks;
  uint32_t texture_signs[8];
  uint64_t layout;
};

struct PipelineLayout {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout textures_vertex = VK_NULL_HANDLE;
  VkDescriptorSetLayout textures_pixel = VK_NULL_HANDLE;
};

struct VertexRangeKey {
  uint32_t address;
  uint32_t size;
  uint64_t hash;
  bool operator==(const VertexRangeKey& o) const {
    return address == o.address && size == o.size && hash == o.hash;
  }
};
struct VertexRangeKeyHasher {
  size_t operator()(const VertexRangeKey& k) const { return size_t(k.hash ^ k.address); }
};

}  // namespace

struct Renderer::Impl {
  Renderer* owner = nullptr;
  rex::ui::vulkan::VulkanProvider* provider = nullptr;
  const VulkanDevice* device = nullptr;
  VkDevice vk = VK_NULL_HANDLE;
  rex::memory::Memory* memory = nullptr;
  RegisterFile* regs = nullptr;

  // Shaders.
  std::unique_ptr<SpirvShaderTranslator> translator;
  rex::string::StringBuffer disasm;
  std::map<std::pair<uint64_t, uint32_t>, std::unique_ptr<VulkanShader>> shaders;
  struct KnownShader {
    VulkanShader* shader;
    uint64_t sample;  // of all of the code
  };
  std::unordered_map<uint64_t, KnownShader> shaders_by_location;
  VulkanShader* active_vs = nullptr;
  VulkanShader* active_ps = nullptr;
  bool specialize_signs = false;

  // Layouts.
  VkDescriptorSetLayout layout_empty = VK_NULL_HANDLE;
  VkDescriptorSetLayout layout_shared = VK_NULL_HANDLE;
  VkDescriptorSetLayout layout_constants = VK_NULL_HANDLE;
  uint32_t shared_binding_count = 1;
  std::unordered_map<uint64_t, VkDescriptorSetLayout> texture_set_layouts;
  std::unordered_map<uint64_t, PipelineLayout> pipeline_layouts;

  // The stream: one host-visible buffer the shaders read vertices from (as
  // the translator's "shared memory"), with the indices, the constants and the
  // texture staging data in it too.
  VkBuffer stream_buffer = VK_NULL_HANDLE;
  VkDeviceMemory stream_memory = VK_NULL_HANDLE;
  uint8_t* stream_mapping = nullptr;
  uint64_t stream_head = 0;  // total bytes ever allocated
  VkDescriptorPool static_pool = VK_NULL_HANDLE;
  VkDescriptorSet set_shared = VK_NULL_HANDLE;
  VkDescriptorSet set_constants = VK_NULL_HANDLE;
  VkDeviceSize ubo_alignment = 256;
#if COD6_INSTANCE_CONSTANTS
  struct InstanceConstantRecord {
    float values[5][4];
  };
  static_assert(sizeof(InstanceConstantRecord) == 80);
  static constexpr uint32_t kInstanceRecordsPerSubmission = 32768;
  static constexpr VkDeviceSize kInstanceSubmissionBytes =
      VkDeviceSize(kInstanceRecordsPerSubmission) * sizeof(InstanceConstantRecord);
  VkBuffer instance_buffer = VK_NULL_HANDLE;
  VkDeviceMemory instance_memory = VK_NULL_HANDLE;
  uint8_t* instance_mapping = nullptr;
  uint32_t instance_record_cursors[kSubmissionSlots] = {};
  struct FixedInstanceVsUpload {
    uint64_t submission = 0;
    const VulkanShader* shader = nullptr;
    VkDeviceSize offset = 0;
    VkDeviceSize bytes = 0;
    std::array<uint8_t, kFloatConstantsBytes> contents = {};
  } fixed_instance_vs_upload;
#endif

  // Submissions.
  struct Submission {
    VkCommandPool pool = VK_NULL_HANDLE;
    // The main command buffer's own: the setup one is recorded on the ring
    // thread while the recording thread records this one.
    VkCommandPool main_pool = VK_NULL_HANDLE;
    VkCommandBuffer setup = VK_NULL_HANDLE;
    VkCommandBuffer main = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    bool pending = false;
    uint64_t number = 0;
    uint64_t frame = 0;
    // Oldest stream position the submission reads.
    uint64_t stream_oldest = 0;
    std::vector<VkDescriptorPool> descriptor_pools;
    size_t descriptor_pool_used = 0;
    bool timed = false;
  };
  VkQueryPool timestamps = VK_NULL_HANDLE;
  float timestamp_period = 1.0f;
  Submission submissions[kSubmissionSlots];
  uint64_t submission_number = 0;  // the one being recorded, if recording
  bool recording = false;
  uint64_t frame = 1;
  uint64_t completed_frame = 0;
  // Whether render targets get the game's sample count (cod6_native_msaa).
  // The targets are sized in guest pixels whatever the sample count, so one
  // sample changes only the images, the pipelines and the resolves.
  bool multisample = false;
  // D24_UNORM_S8_UINT for the game's kD24S8 buffers (cod6_native_depth24).
  bool depth24 = false;
  // The float24 ones (here only shadow maps) become D32_SFLOAT without
  // stencil, 32 instead of 64 bits per pixel; a stencil test on them passes.
  VkFormat DepthFormatFor(uint32_t guest_format) const {
    if (!depth24) return kDepthFormat;
    return guest_format == uint32_t(xenos::DepthRenderTargetFormat::kD24S8)
               ? VK_FORMAT_D24_UNORM_S8_UINT
               : VK_FORMAT_D32_SFLOAT;
  }
  static bool HasStencil(VkFormat format) { return format != VK_FORMAT_D32_SFLOAT; }
  VkDescriptorSetLayout depth_convert_set_layout = VK_NULL_HANDLE;
  VkPipelineLayout depth_convert_layout = VK_NULL_HANDLE;
  VkPipeline depth_convert_pipeline = VK_NULL_HANDLE;
  VkDescriptorPool depth_convert_pool = VK_NULL_HANDLE;
  VkDescriptorSet depth_convert_set = VK_NULL_HANDLE;
  // Depth straight from the depth buffer into the R32_SFLOAT image, in one
  // compute pass (depth_to_float.cs.glsl).
  VkDescriptorSetLayout depth_copy_set_layout = VK_NULL_HANDLE;
  VkPipelineLayout depth_copy_layout = VK_NULL_HANDLE;
  VkPipeline depth_copy_pipeline = VK_NULL_HANDLE;
  // Color resolves by compute (copy_color.cs.glsl), same set layout.
  VkPipelineLayout copy_color_layout = VK_NULL_HANDLE;
  VkPipeline copy_color_pipeline = VK_NULL_HANDLE;
  bool ComputeColorCopy(VkCommandBuffer c, VkImageView source, VkImageView dest, int32_t sx,
                        int32_t sy, int32_t dx, int32_t dy, uint32_t width, uint32_t height);
  bool CreateDepthConversion();

#if COD6_DIAGNOSTICS
  // GPU time by render pass and resolve (cod6_native_gpu_profile): on one
  // frame in 16, a timestamp is written where each pass or resolve begins,
  // and the time to the next one is charged to it.
  static constexpr uint32_t kProfileQueries = 512;
  VkQueryPool profile_pool = VK_NULL_HANDLE;
  bool profile_frame = false;
#endif
  // cod5-nx debug (cod6_debug_trace_frame): this frame's passes, resolves and skips go to the log.
  bool trace_frame = false;
  uint32_t dump_swaps_left = 0;
  uint32_t trace_pass_draws = 0;
  uint32_t trace_skip_logged = 0;
  uint64_t clear_redirects = 0;
  uint32_t trace_draws_detailed = 0;
  // Texture fetches looked up in the traced frame, by format and layout: count and one example.
  std::map<std::string, std::pair<uint32_t, std::string>> trace_texture_formats;
  struct TraceTextureDump {
    VkImage image;
    uint32_t width, height;
    VkFormat format;
    std::string name;
  };
  std::vector<TraceTextureDump> trace_texture_dumps;
  std::unordered_set<uint32_t> trace_texture_dumped;
  std::unordered_map<uint32_t, uint32_t> trace_texture_quota;
  bool trace_mips_dumped = false;
  // cod5-nx debug (cod6_debug_mip_watch): mip data of large DXT textures. Real DXT blocks measure ~6 bits of entropy
  // per byte; the noise in COD5's mips measured ~7.1, like still-compressed data.
  struct MipWatch {
    uint32_t mip_address = 0;
    uint32_t bytes = 0;
    uint64_t first_frame = 0;
    float first_entropy = 0;
    uint64_t hash = 0;
    uint32_t changes = 0;
    float last_entropy = 0;
  };
  std::unordered_map<uint32_t, MipWatch> mip_watch;  // by base address
  static float Entropy(const uint8_t* p, size_t n) {
    uint32_t counts[256] = {};
    for (size_t i = 0; i < n; ++i) ++counts[p[i]];
    double e = 0;
    for (uint32_t c : counts) {
      if (c) {
        const double q = double(c) / double(n);
        e -= q * std::log2(q);
      }
    }
    return float(e);
  }
  void WatchMips(const xenos::xe_gpu_texture_fetch_t& f) {
    const uint32_t format = uint32_t(f.format);
    if ((format != 18 && format != 19 && format != 20 && format != 49) || !f.mip_address ||
        f.size_2d.width + 1 < 256 || f.dimension != xenos::DataDimension::k2DOrStacked) {
      return;
    }
    const uint32_t base = f.base_address << 12;
    auto [it, inserted] = mip_watch.try_emplace(base);
    MipWatch& w = it->second;
    if (inserted) {
      // Level 1 alone: a quarter of level 0's blocks.
      const uint32_t blocks = ((f.size_2d.width + 1) / 8) * ((f.size_2d.height + 1) / 8);
      w.bytes = std::min<uint32_t>(blocks * (format == 18 ? 8 : 16), 0x10000);
      w.mip_address = f.mip_address << 12;
      w.first_frame = frame;
      const uint8_t* p = memory->TranslatePhysical<const uint8_t*>(w.mip_address);
      w.first_entropy = w.last_entropy = Entropy(p, w.bytes);
      w.hash = XXH3_64bits(p, w.bytes);
      return;
    }
    if (frame % 120 != (base >> 12) % 120) return;
    const uint8_t* p = memory->TranslatePhysical<const uint8_t*>(w.mip_address);
    const uint64_t hash = XXH3_64bits(p, w.bytes);
    if (hash != w.hash) {
      const float e = Entropy(p, w.bytes);
      if (w.changes < 3) {
        REXLOG_INFO("[mipwatch] {:08X} mips {:08X} changed at frame {} (first seen {}): entropy {:.2f} -> {:.2f}",
                    base, w.mip_address, frame, w.first_frame, w.last_entropy, e);
      }
      w.hash = hash;
      w.last_entropy = e;
      ++w.changes;
    }
  }
  void ReportMipWatch() {
    uint32_t total = 0, noisy_first = 0, noisy_now = 0, changed = 0;
    for (const auto& [base, w] : mip_watch) {
      ++total;
      noisy_first += w.first_entropy > 6.8f;
      noisy_now += w.last_entropy > 6.8f;
      changed += w.changes != 0;
    }
    REXLOG_INFO("[mipwatch] {} textures: {} looked compressed at first sight, {} now; {} changed after first sight",
                total, noisy_first, noisy_now, changed);
  }
  uint32_t trace_raw_mip_address = 0, trace_raw_base_address = 0;
  uint64_t last_frame_draw_count = 0;
  std::unordered_set<uint64_t> trace_disassembled;
  // cod5-nx: the exponent bias of the last color resolve to each destination (by 4 KB page address). The resolve
  // copies without scaling; the bias is added to the exp_adjust of the texture fetch constants that sample the
  // destination, which the translated shaders apply to the fetched value (exact apart from the hardware's clamp).
  // COD5's exposure chain resolves its 16_16_16_16 targets with bias 11 and samples them back with -11: ignoring
  // the bias read the values 2048 times too small, zero after a few passes, and the whole frame came out black.
  std::unordered_map<uint32_t, int32_t> resolve_exp_bias;
  void NoteResolveExpBias(uint32_t dest_base, xenos::ColorRenderTargetFormat source_format) {
    const uint32_t mode = REXCVAR_GET(cod6_native_resolve_exp_bias);
    int32_t bias = regs->Get<reg::RB_COPY_DEST_INFO>().copy_dest_exp_bias;
    if (mode == 2 && (source_format == xenos::ColorRenderTargetFormat::k_16_16 ||
                      source_format == xenos::ColorRenderTargetFormat::k_16_16_16_16)) {
      bias += 5;
    }
    const uint32_t page = dest_base & 0x1FFFF000;
    if (!mode || !bias) {
      resolve_exp_bias.erase(page);
    } else {
      resolve_exp_bias[page] = bias;
    }
  }
  uint32_t trace_pass_skips[16] = {};
  void TraceEndPass() {
    if (!trace_frame) return;
    std::string skips;
    for (uint32_t i = 0; i < 16; ++i) {
      if (trace_pass_skips[i]) skips += fmt::format(" {}:{}", i, trace_pass_skips[i]);
    }
    REXLOG_INFO("[trace]   {} draws, skipped{}", trace_pass_draws, skips.empty() ? " none" : skips);
    trace_pass_draws = 0;
    std::memset(trace_pass_skips, 0, sizeof(trace_pass_skips));
  }
  // The destination of the last resolve of the traced frame, saved by DebugDumpResolve.
  ResolveDestination* trace_last_dest = nullptr;
  uint32_t trace_dumps = 0;
  // Copies a single-sampled image back and writes <cod6_debug_capture_dir>/<name>.img: a 16-byte header (width,
  // height, VkFormat, bytes per pixel; little endian), then the rows. tools/rgbx_to_png.py converts it.
  void DebugDumpImage(VkImage image, uint32_t width, uint32_t height, VkFormat format, const std::string& name,
                      uint32_t level = 0) {
    if constexpr (!cod6::diagnostics::kEnabled) return;
    const std::string dir = REXCVAR_GET(cod6_debug_capture_dir);
    std::string host = REXCVAR_GET(cod6_debug_dump_host);
    // Without a dump host, the machine receiving the network log (port 28772): the netloader cuts the arguments
    // at about 256 characters, so a second address did not fit.
    const std::string log_host = REXCVAR_GET(log_network);
    if (host.empty() && dir.empty() && !log_host.empty()) {
      host = log_host.substr(0, log_host.rfind(':')) + ":28772";
    }
    if ((dir.empty() && host.empty()) || !image || !width || !height) return;
    uint32_t bpp = 0;
    switch (format) {
      case VK_FORMAT_R8G8B8A8_UNORM: case VK_FORMAT_B8G8R8A8_UNORM: case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_R32_SFLOAT: case VK_FORMAT_R16G16_UNORM: case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_SFLOAT:
        bpp = 4;
        break;
      case VK_FORMAT_R16G16B16A16_UNORM: case VK_FORMAT_R16G16B16A16_SNORM: case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32G32_SFLOAT:
        bpp = 8;
        break;
      case VK_FORMAT_R5G6B5_UNORM_PACK16: case VK_FORMAT_R8G8_UNORM:
        bpp = 2;
        break;
      // Block compressed: "bpp" is bytes per 4x4 block; the copy below works in texels either way.
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: case VK_FORMAT_BC1_RGB_UNORM_BLOCK: case VK_FORMAT_BC4_UNORM_BLOCK:
        bpp = 8;
        break;
      case VK_FORMAT_BC2_UNORM_BLOCK: case VK_FORMAT_BC3_UNORM_BLOCK: case VK_FORMAT_BC5_UNORM_BLOCK:
      case VK_FORMAT_BC5_SNORM_BLOCK:
        bpp = 16;
        break;
      default:
        REXLOG_INFO("[trace]   (no dump of {}: format {})", name, int(format));
        return;
    }
    const bool blocks = format >= VK_FORMAT_BC1_RGB_UNORM_BLOCK && format <= VK_FORMAT_BC7_SRGB_BLOCK;
    const VkDeviceSize size = blocks ? VkDeviceSize((width + 3) / 4) * ((height + 3) / 4) * bpp
                                     : VkDeviceSize(width) * height * bpp;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
    if (!rex::ui::vulkan::util::CreateDedicatedAllocationBuffer(
            device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, rex::ui::vulkan::util::MemoryPurpose::kReadback,
            buffer, buffer_memory)) {
      return;
    }
    BeginRecording();
    EndPass();
    VkCommandBuffer c = cmd();
    VkMemoryBarrier barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_MEMORY_WRITE_BIT,
                               VK_ACCESS_TRANSFER_READ_BIT};
    dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &barrier, 0, nullptr, 0, nullptr);
    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1};
    region.imageExtent = {width, height, 1};
    dfn().vkCmdCopyImageToBuffer(c, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    Submit();
    vkDeviceWaitIdle(vk);
    void* mapping = nullptr;
    if (dfn().vkMapMemory(vk, buffer_memory, 0, VK_WHOLE_SIZE, 0, &mapping) == VK_SUCCESS && !host.empty()) {
      const uint32_t header[4] = {width, height, uint32_t(format), bpp};
      if (SendDump(host, name + ".img", header, mapping, size_t(size))) {
        REXLOG_INFO("[trace]   sent {}x{} format {} as {}.img to {}", width, height, int(format), name, host);
      }
      dfn().vkUnmapMemory(vk, buffer_memory);
    } else if (mapping) {
      const std::string path = fmt::format("{}/{}.img", dir, name);
      if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
        const uint32_t header[4] = {width, height, uint32_t(format), bpp};
        std::fwrite(header, sizeof(header), 1, f);
        std::fwrite(mapping, 1, size_t(size), f);
        std::fclose(f);
        REXLOG_INFO("[trace]   dumped {}x{} format {} to {}", width, height, int(format), path);
      }
      dfn().vkUnmapMemory(vk, buffer_memory);
    }
    dfn().vkDestroyBuffer(vk, buffer, nullptr);
    dfn().vkFreeMemory(vk, buffer_memory, nullptr);
  }
  // One TCP connection per dump: a 64-byte name, the 16-byte header, then the data.
  static bool SendDump(const std::string& host, const std::string& name, const uint32_t header[4], const void* data,
                       size_t size) {
    const size_t colon = host.rfind(':');
    if (colon == std::string::npos) return false;
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(uint16_t(std::atoi(host.c_str() + colon + 1)));
    if (inet_pton(AF_INET, host.substr(0, colon).c_str(), &address.sin_addr) != 1) return false;
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;
    bool ok = connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
    auto send_all = [&](const void* p, size_t n) {
      const char* c = static_cast<const char*>(p);
      while (ok && n) {
        const ssize_t sent = send(fd, c, n, 0);
        if (sent <= 0) {
          ok = false;
          break;
        }
        c += sent;
        n -= size_t(sent);
      }
    };
    char name_field[64] = {};
    std::snprintf(name_field, sizeof(name_field), "%s", name.c_str());
    send_all(name_field, sizeof(name_field));
    send_all(header, 16);
    // Through a plain heap buffer, in 64 KB pieces: Horizon's socket service can't read GPU-mapped memory (the
    // send failed and only the name and header arrived).
    std::vector<char> bounce(64 << 10);
    for (size_t offset = 0; ok && offset < size; offset += bounce.size()) {
      const size_t n = std::min(bounce.size(), size - offset);
      std::memcpy(bounce.data(), static_cast<const char*>(data) + offset, n);
      send_all(bounce.data(), n);
    }
    close(fd);
    return ok;
  }
  void DebugDumpResolve() {
    if (!trace_frame || !trace_last_dest) return;
    ResolveDestination* d = trace_last_dest;
    trace_last_dest = nullptr;
    DebugDumpImage(d->image, d->width, d->height, d->format,
                   fmt::format("f{}_r{:02d}_{:08X}", frame, trace_dumps++, d->address));
  }
  static std::string TraceTarget(const RenderTarget* rt) {
    if (!rt) return "-";
    return fmt::format("{}{}@{} p{} {}x{} ms{}", rt->key.is_depth ? "D" : "C", rt->key.format, rt->key.base_tiles,
                       rt->key.pitch, rt->width, rt->height, rt->key.msaa);
  }
#if COD6_DIAGNOSTICS
  struct ProfileSegment {
    uint64_t key;
    uint32_t draws;
  };
  std::array<ProfileSegment, kProfileQueries - 1> profile_segments[kSubmissionSlots] = {};
  uint32_t profile_segment_counts[kSubmissionSlots] = {};
  std::array<uint64_t, kProfileQueries> profile_ticks = {};
  bool profile_closed[kSubmissionSlots] = {};
  struct ProfileTotal {
    double ns = 0;
    uint64_t count = 0;
    uint64_t draws = 0;
  };
  std::unordered_map<uint64_t, ProfileTotal> profile_totals;
  uint32_t profiled_frames = 0;
  double profiled_ns = 0;
  // Kinds, in the key's top 4 bits.
  enum : uint64_t { kProfileOther = 0, kProfilePass = 1, kProfileResolve = 2 };
  static uint64_t ProfileTargetBits(const RenderTarget* rt) {
    if (!rt) return 0xFFFFFF;
    return (uint64_t(rt->key.base_tiles & 0xFFF) << 12) | (uint64_t(rt->key.pitch & 0x7FF) << 1) |
           (rt->key.format & 1);
  }
  void ProfileMark(uint64_t key) {
    if (!profile_frame || !profile_pool || !recording) return;
    uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
    auto& segments = profile_segments[slot];
    uint32_t& count = profile_segment_counts[slot];
    // The last query is kept for the closing timestamp.
    if (count >= segments.size()) return;
    dfn().vkCmdWriteTimestamp(cmd(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, profile_pool,
                              slot * kProfileQueries + count);
    segments[count++] = {key, 0};
  }
  void ProfileDraw() {
    if (!profile_frame) return;
    const uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
    const uint32_t count = profile_segment_counts[slot];
    if (count) ++profile_segments[slot][count - 1].draws;
  }
  void ProfileCollect(uint32_t slot);
  void CaptureProfileReport(
      GpuProfileSnapshot<RendererReportSnapshot::kGpuProfileCapacity>& snapshot);
#endif
  struct TextureTimer {
    uint64_t& total;
    uint64_t start;
    explicit TextureTimer(uint64_t& t) : total(t), start(NowNs()) {}
    ~TextureTimer() { total += NowNs() - start; }
  };
  uint32_t HostMsaa(xenos::MsaaSamples guest) const {
    return GuestMsaaModes::HostMode(multisample, uint32_t(guest));
  }
  bool null_images_initialized = false;

  // Render targets.
  std::unordered_map<RenderTargetKey, std::unique_ptr<RenderTarget>, RenderTargetKeyHasher>
      render_targets;
  bool in_pass = false;
  PassTargets pass;
  VkExtent2D pass_extent = {};

  // Pipelines.
  std::unordered_map<uint64_t, VkPipeline> pipelines;
  std::unordered_map<uint32_t, VkShaderModule> geometry_shaders;
  VkPipelineCache driver_cache = VK_NULL_HANDLE;
  std::filesystem::path driver_cache_path;
  uint64_t pipelines_at_last_save = 0;
  std::chrono::steady_clock::time_point last_pipeline_time;
  size_t driver_cache_saved_size = 0;
  std::atomic<bool> driver_cache_writing{false};
  std::thread driver_cache_writer;

  // Frame time tracking.
  std::chrono::steady_clock::time_point last_swap_time;
  RendererStats at_last_swap;

  // What the main command buffer has bound.
  VkPipeline bound_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout bound_layout = VK_NULL_HANDLE;
  bool bound_static_sets = false;
  VkViewport bound_viewport = {};
  VkRect2D bound_scissor = {};
  float bound_depth_bias[2] = {NAN, NAN};
  float bound_blend_constants[4] = {NAN, NAN, NAN, NAN};
  uint32_t bound_stencil[6] = {~0u, ~0u, ~0u, ~0u, ~0u, ~0u};

  // Vertex and index data already in the stream: stream position by the
  // range and its content.
  struct RangeCopy {
    VertexRangeKey key = {};
    uint64_t position = 0;
  };
  FlatMap64<RangeCopy> vertex_ranges;
  static uint64_t RangeSlot(const VertexRangeKey& k) {
    uint64_t h = k.hash ^ ((uint64_t(k.address) << 32) | k.size);
    h *= 0xBF58476D1CE4E5B9ull;
    h ^= h >> 31;
    return h == ~uint64_t(0) ? 0 : h;
  }
  // The stream position of a copy of this range, or UINT64_MAX.
  uint64_t FindRange(const VertexRangeKey& k) {
    RangeCopy* c = vertex_ranges.find(RangeSlot(k));
    return c && c->key == k ? c->position : UINT64_MAX;
  }
  void AddRange(const VertexRangeKey& k, uint64_t position) {
    vertex_ranges[RangeSlot(k)] = {k, position};
  }

  TextureCache textures;
  UploadTarget upload;
  bool diagnostics = false;
  bool vertex_sync_cache = false;
  uint64_t vertex_memory_epoch = 0;
  uint64_t observed_write_serial = 0;
#if COD6_DIAGNOSTICS
  bool preparing_draw = false;
  ChangeDetectorStats vertex_change_stats;
  uint64_t instance_pass_serial = 0;
  InstanceCandidateTracker instance_candidate_tracker;
  // Draws, no-target skips, depth copies, and depth-candidate texture lookups.
  std::array<DiagnosticBuckets<16>, 4> shadow_diagnostics;
  void NoteShadow(size_t kind, uint64_t a = 0, uint64_t b = 0, uint64_t c = 0) {
    if (!diagnostics) return;
    const auto& r = *regs;
    shadow_diagnostics[kind].Record({active_vs ? active_vs->ucode_data_hash() : 0,
        active_ps ? active_ps->ucode_data_hash() : 0,
        r[rex::graphics::XE_GPU_REG_RB_SURFACE_INFO],
        r[rex::graphics::XE_GPU_REG_RB_DEPTH_INFO],
        r[rex::graphics::XE_GPU_REG_RB_DEPTHCONTROL],
        r[rex::graphics::XE_GPU_REG_RB_COLOR_MASK], a, b, c,
        r.Get<reg::RB_MODECONTROL>().value});
  }
#endif

  // Texture descriptor sets, by content: the game uses the same few hundred
  // combinations of textures and samplers every frame, and allocating and
  // writing a set per draw (then freeing them one by one on pool reset) was
  // an eighth of the renderer's time.
  std::unordered_map<uint64_t, VkDescriptorSet> texture_set_cache;
  std::vector<VkDescriptorPool> set_pools;
  size_t set_pool_used = 0;
  struct RetiredPools {
    uint64_t frame;
    std::vector<VkDescriptorPool> pools;
  };
  std::vector<RetiredPools> retired_set_pools;
  uint64_t texture_views_epoch = 0;

  // Vertex buffer content hashes, computed once per frame for each range: many
  // draws share one buffer.
  struct VertexHash {
    cod6::writes::WriteHashRecord write_hash;
    uint64_t frame = UINT64_MAX;
    uint64_t memory_epoch = 0;
    uint64_t hash = 0;
    // Last real content hash, and how many checks in a row found it changed.
    uint64_t content = 0;
    uint32_t changes = 0;
    ChangeDetector detector;
  };
  // This frame's hash of a vertex or index range. Data the game rewrites
  // every frame (skinned meshes, particles) was hashed in full every frame
  // only to be copied anyway, as the hash never matched: once a range has
  // changed on 4 checks in a row it gets a hash unique to the frame without
  // reading it (still copied once per frame, shared by its draws), and is
  // checked for real every 16 frames to notice it settling.
  uint64_t RangeHash(VertexHash& known, const uint8_t* data, uint32_t bytes, uint32_t address) {
    if(cod6::writes::Enabled()) {
      const auto result=known.write_hash.Check(cod6::writes::g_pages,address,bytes,
          cod6::writes::g_verify_only,cod6::writes::g_healthy,[&] {
            const uint64_t start=NowNs();
            const uint64_t hash=XXH3_64bits(data,bytes);
            stats().vertex_hash_ns+=NowNs()-start;vertex_hash_bytes+=bytes;
            return hash;
          });
      stats().vertex_write_hits+=result.reused;
      stats().vertex_write_compared+=result.compared;
      stats().vertex_write_mismatches+=result.mismatch;
      stats().vertex_write_busy+=result.busy;
      if(result.mismatch)REXLOG_ERROR("[writes] unmarked vertex/index change at {:08X} + {}; full hash retained, all write-based reuse disabled",address,bytes);
      known.frame=frame;known.memory_epoch=vertex_memory_epoch;
      known.content=known.hash=result.hash;
      return result.hash;
    }
    if (known.frame == frame && known.memory_epoch == vertex_memory_epoch) return known.hash;
    const bool synchronized = known.memory_epoch != vertex_memory_epoch;
    known.frame = frame;
    known.memory_epoch = vertex_memory_epoch;
    if (synchronized) {
      known.detector.ResetTrust();
      ++stats().vertex_sync_rechecks;
    }
    if (known.changes >= 4 && (frame & 15) && !synchronized) {
      known.hash = (frame + 1) * 0xD1B54A32D192ED03ull ^ (uint64_t(address) << 20) ^ bytes;
      return known.hash;
    }
    const uint64_t hash_start = NowNs();
    uint64_t content = known.detector.Check(data, bytes, frame, address >> 4, vertex_hash_bytes
#if COD6_DIAGNOSTICS
                                            , diagnostics ? &vertex_change_stats : nullptr
#endif
                                            );
    stats().vertex_hash_ns += NowNs() - hash_start;
    known.changes = content != known.content ? std::min(known.changes + 1, 8u) : 0;
    known.content = content;
    known.hash = content;
    return content;
  }
  uint64_t vertex_hash_bytes = 0;
  FlatMap64<VertexHash> vertex_hashes;

  // Swap.
  VkDescriptorSetLayout swap_layout_ramp = VK_NULL_HANDLE;
  VkDescriptorSetLayout swap_layout_source = VK_NULL_HANDLE;
  VkDescriptorSetLayout swap_layout_dest = VK_NULL_HANDLE;
  VkPipelineLayout swap_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline swap_pipeline = VK_NULL_HANDLE;
  VkBuffer ramp_buffer = VK_NULL_HANDLE;
  VkDeviceMemory ramp_memory = VK_NULL_HANDLE;
  uint8_t* ramp_mapping = nullptr;
  VkBufferView ramp_views[kSubmissionSlots] = {};
  VkDescriptorPool swap_pool = VK_NULL_HANDLE;
  VkDescriptorSet swap_sets[kSubmissionSlots][3] = {};
  uint32_t gamma_ramp[256] = {};

  SpirvShaderTranslator::SystemConstants system_constants = {};
  SceneScale scene_scale;

  bool IsSceneTarget(const RenderTarget* target) const {
    // The fixed scene uses a guest MSAA EDRAM mode even when native MSAA is
    // disabled and the actual host image is 1x. Preserve the guest mode for
    // tile aliases and clear/resolve semantics; eligibility is determined by
    // the host target key below. Requiring guest_msaa == 0 silently rejected
    // every gameplay pass in cod5sp_008.
    return target &&
           scene_scale.IsSceneTarget(target->key.base_tiles, target->key.pitch,
                                     target->key.is_depth != 0, target->key.msaa);
  }

  bool IsScenePass(const PassTargets& targets) const {
    bool any = false;
    for (const RenderTarget* target : {targets.color[0], targets.color[1],
                                       targets.color[2], targets.color[3],
                                       targets.depth}) {
      if (!target) {
        continue;
      }
      if (!IsSceneTarget(target)) {
        return false;
      }
      any = true;
    }
    return any;
  }

  struct DrawSetup {
    bool valid = false;
    uint64_t identity = 0;
    uint64_t state_generation = 0;
    uint64_t fetch_generation = 0;
    uint64_t rt_generation = 0;
    VulkanShader* active_vs = nullptr;
    VulkanShader* active_ps = nullptr;
    uint32_t initiator_state = 0;
    uint32_t skip_reason = UINT32_MAX;
    VulkanShader* vs = nullptr;
    VulkanShader* ps = nullptr;
    bool primitive_polygonal = false;
    bool scene_scaled = false;
    reg::RB_DEPTHCONTROL depth_control = {};
    bool convert_fan = false, convert_loop = false, strip = false, reset_enabled = false;
#if COD6_INSTANCE_CONSTANTS
    bool instance_constants = false;
#endif
    draw_util::Scissor scissor = {};
    draw_util::ViewportInfo viewport = {};
    PassTargets targets;
    const PipelineLayout* layout = nullptr;
    uint32_t counts[4] = {};
    uint32_t texture_slot_mask = 0;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkViewport vp = {};
    float bias_constant = 0.0f, bias_slope = 0.0f;
    float blend_constants[4] = {};
    uint32_t stencil[6] = {};
  };
  DrawSetup setup;
  std::unordered_map<uint64_t, DrawSetup> setup_cache;
  uint64_t setup_cache_rt_generation = 0;
  // Original dependency token: changes on a setup build or cache retrieval.
  uint64_t setup_generation = 0;
  uint64_t next_setup_identity = 1;
  bool stable_setup_identity = false;
  SystemConstantInputIdentity system_inputs;
  struct RetiredRenderTarget {
    uint64_t frame;
    VkImage image;
    VkImageView view;
    VkImageView sample_view;
    VkDeviceMemory memory;
  };
  std::vector<RetiredRenderTarget> retired_render_targets;
  // Staging for depth resolves (depth to buffer to color image).
  static constexpr VkDeviceSize kDepthScratchSize = 16u << 20;
  VkBuffer depth_scratch = VK_NULL_HANDLE;
  VkDeviceMemory depth_scratch_memory = VK_NULL_HANDLE;
  // Bumped whenever a render target is created, as setups hold pointers.
  uint64_t rt_generation = 1;
#if COD6_DIAGNOSTICS
  uint64_t next_render_target_diagnostic_identity = 1;
  ResolveRepeatTracker<> resolve_repeat_tracker;
  enum ResolveRepeatPath : uint32_t {
    kRepeatDownsample = 0,
    kRepeatDepthDirect = 1,
    kRepeatDepthScratch = 2,
    kRepeatColorMsaa = 3,
    kRepeatColorCompute = 4,
    kRepeatColorTransfer = 5,
  };
  void MarkRenderTargetWritten(RenderTarget* target) {
    if (diagnostics && target) ++target->diagnostic_content_generation;
  }
  void NoteSuccessfulResolve(RenderTarget* source, ResolveDestination* destination,
                             uint32_t source_x, uint32_t source_y, uint32_t width,
                             uint32_t height, uint32_t copy_control,
                             uint32_t destination_info, ResolveRepeatPath path) {
    if (!diagnostics || !source || !destination || uint32_t(path) >= 6) return;
    ResolveRepeatKey key;
    key.source_identity = source->diagnostic_identity;
    key.source_content_generation = source->diagnostic_content_generation;
    key.destination_identity = uint64_t(reinterpret_cast<uintptr_t>(destination));
    key.answers_epoch = textures.answers_epoch();
    key.fanout_generation = textures.resolve_fanout_generation();
    key.destination_address = destination->address;
    key.source_x = source_x;
    key.source_y = source_y;
    key.width = width;
    key.height = height;
    key.destination_width = destination->width;
    key.destination_height = destination->height;
    key.source_format = uint32_t(source->format);
    key.destination_format = uint32_t(destination->format);
    key.destination_swizzle = destination->host_swizzle;
    key.copy_control = copy_control;
    key.destination_info = destination_info;
    key.path = uint32_t(path);
    ++stats().resolve_repeat_observations;
    ++stats().resolve_repeat_path_observations[key.path];
    const ResolveRepeatResult result = resolve_repeat_tracker.Observe(key);
    if (result == ResolveRepeatResult::kRepeat) {
      ++stats().resolve_repeat_candidates;
      ++stats().resolve_repeat_path_candidates[key.path];
    } else if (result == ResolveRepeatResult::kCollision) {
      ++stats().resolve_repeat_collisions;
    }
  }
#endif
  bool BuildDrawSetup(uint32_t initiator_value);
  uint64_t AllocateSetupIdentity() {
    if (!next_setup_identity) {
      // A full 64-bit wrap is theoretical, but must not alias an entry that is
      // still cached if it ever happens.
      ClearMapMeasured(setup_cache, 0, setup_cache.bucket_count());
      next_setup_identity = 1;
    }
    return next_setup_identity++;
  }
  // cod5-nx: COD5's GPU memcpy, a point list whose vertex shader (kCod5MemcpyVs) reads 64 bytes per vertex from vertex
  // fetch 0 and memexports them as 8 elements of 8 bytes (k_16_16_16_16) to the stream in c0, at element index
  // c0.y + (c255.w * i) * c255.y and, for the second half, + c255.z. Its texture streamer copies low mips with it;
  // skipped, the mips kept stale data (the rainbow noise on distant surfaces). Done here on the CPU, in ring order.
  static constexpr uint64_t kCod5MemcpyVs = 0xA4827826E2589916ull;
  void EmulateMemcpyExport(const reg::VGT_DRAW_INITIATOR& initiator, VulkanShader* vs);
  uint64_t memexport_copies = 0, memexport_bytes = 0;

  struct TextureBindings {
    bool valid = false;
    uint64_t frame = 0;
    uint64_t submission = 0;
    uint64_t views_epoch = 0;
    const VulkanShader* vs = nullptr;
    const VulkanShader* ps = nullptr;
    const PipelineLayout* layout = nullptr;
    uint32_t used_mask = 0;
    uint32_t slots = 0;
    VkDescriptorSet sets[2] = {};
    // Reusable in later frames through this one: no texture behind it is due
    // a content check before then. The textures are marked used on reuse.
    uint64_t valid_until = 0;
    uint64_t set_generation = 0;
    uint64_t answers_epoch = 0;
    // The views the sets hold, to check them alive when views_epoch moved.
    uint32_t view_count = 0;
    VkImageView views[24] = {};
    uint64_t view_ids[24] = {};
    uint32_t texture_count = 0;
    void* textures[24] = {};
  };
  FlatMap64<TextureBindings> tb_cache;
  using BindingMemo = BindingKeyMemo<4096>;
  std::unique_ptr<BindingMemo> binding_key_memo;

  // What each fetch slot last resolved to, this frame and submission: most
  // binding rebuilds change one or two slots of several.
  struct SlotViewEntry {
    uint64_t frame = 0;
    uint64_t submission = 0;
    uint64_t views_epoch = 0;
    uint64_t answers_epoch = 0;
    uint32_t words[6] = {};
    uint32_t variant = 0;
    VkImageView view = VK_NULL_HANDLE;
    void* texture = nullptr;
    uint64_t valid_until = 0;
    bool Reusable(uint64_t current_frame, uint64_t current_submission,
                  uint64_t current_views_epoch, uint64_t current_answers_epoch,
                  uint32_t current_variant, const uint32_t* current_words) const {
      return frame == current_frame && submission == current_submission &&
             views_epoch == current_views_epoch && answers_epoch == current_answers_epoch &&
             variant == current_variant && !std::memcmp(words, current_words, sizeof(words));
    }
  };
  SlotViewEntry slot_views[32][8];
  // Set by SlotView: the texture behind its answer and how long it holds.
  void* slot_texture = nullptr;
  uint64_t slot_valid_until = 0;
  struct SlotSamplerEntry {
    bool valid = false;
    uint32_t words[6] = {};
    uint32_t binding_bits = 0;
    VkSampler sampler = VK_NULL_HANDLE;
  };
  SlotSamplerEntry slot_samplers[32][4];

  /* COD6_V303_ALBUM
   *
   * Notes which textures a captured frame samples, so the frame image and the
   * textures behind it can be looked at side by side: a surface showing a clean
   * image that belongs to another object is a binding problem, a garbled one is
   * decoding. The two looked the same in the log. Queued here and written after
   * the frame's submission, like the trace dumps: a readback mid-recording would
   * split the command buffer.
   */
  struct AlbumEntry {
    uint32_t address = 0;
    VkImage image = VK_NULL_HANDLE;
    uint32_t width = 0, height = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
  };
  std::vector<AlbumEntry> album_queue;
  uint64_t album_frame = UINT64_MAX;
  bool album_armed = false;
  uint32_t album_frames_done = 0;
  /* COD6_V304_DRAWTABLE
   *
   * The frame image says a surface is textured wrong; it cannot say which draw,
   * with which pixel shader, sampling which guest address did it. One line per
   * draw of an album frame answers that: shader, viewport, and every texture slot
   * with the address and shape the fetch constant declares.
   */
  uint32_t draw_table_left = 0;
  uint32_t draw_table_skip_left = 0;
  uint32_t draw_table_index = 0;
  uint32_t album_capture_points = 0;
  void AlbumNote(uint32_t slot, VkImageView view, void* texture) {
    if (album_frame != frame) {
      album_frame = frame;
      album_armed = false;
      const uint32_t every = REXCVAR_GET(cod6_debug_capture_every);
      const uint32_t frames = REXCVAR_GET(cod6_debug_album_frames);
      if (every && frame % every == 0 && !REXCVAR_GET(cod6_debug_capture_dir).empty()) {
        ++album_capture_points;
        if (frames && album_frames_done < frames &&
            album_capture_points > REXCVAR_GET(cod6_debug_album_skip)) {
          album_armed = true;
          album_queue.clear();
          draw_table_left = REXCVAR_GET(cod6_debug_draw_table);
          draw_table_index = 0;
          draw_table_skip_left = REXCVAR_GET(cod6_debug_draw_table_skip);
          if (draw_table_left) {
            REXLOG_INFO("[dtab] frame {}: logging the next {} draws", frame, draw_table_left);
          }
        }
      }
    }
    const uint32_t max = REXCVAR_GET(cod6_debug_tex_album);
    if (!max || !album_armed || !view || !texture || album_queue.size() >= max) return;
    const xenos::xe_gpu_texture_fetch_t f = regs->GetTextureFetch(slot);
    const uint32_t address = f.base_address << 12;
    for (const AlbumEntry& e : album_queue) {
      if (e.address == address) return;
    }
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImage image = TextureCache::DebugTextureImage(texture, &format);
    if (!image) return;
    AlbumEntry entry;
    entry.address = address;
    entry.image = image;
    entry.width = f.size_2d.width + 1;
    entry.height = f.size_2d.height + 1;
    entry.format = format;
    album_queue.push_back(entry);
  }

  /* COD6_V307_TEXCHECK
   *
   * BO2's rule, which this port only half implemented: "reuses checked against
   * the sampler loop ... 1 in 1,024 still checked", and on the first
   * disagreement "reuse off for this session". slot_views[] answers a draw's
   * texture from a previous draw; nothing ever checked it. So once every 1024
   * cache hits, resolve the same fetch constant the slow way and compare. A
   * difference means the draw binds a texture the fetch constant does not
   * describe - the reported symptom - so it is printed with the slot, the fetch
   * words and the guest address, and the fast path is turned off for the rest of
   * the session.
   */
  uint64_t slot_view_hits = 0, texcheck_checks = 0, texcheck_mismatches = 0;
  bool texcheck_disabled = false;
  VkImageView SlotView(uint32_t slot, xenos::FetchOpDimension dimension, bool is_signed) {
    const uint32_t* words = &(*regs)[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 +
                                     slot * 6];
    uint32_t variant = uint32_t(dimension) | (uint32_t(is_signed) << 2);
    SlotViewEntry& e = slot_views[slot][variant & 7];
    // An old image/view may remain alive for already-recorded draws after the
    // texture's answer changes. Its Texture object may already be gone. Check
    // the answer epoch as well as view destruction before touching e.texture.
    if (e.Reusable(frame, submission_number, textures.views_epoch(), e.answers_epoch,
                   variant, words) && (e.answers_epoch == textures.answers_epoch() ||
                                       textures.CanReuseAnswers(e.answers_epoch, words, 1))) {
      e.answers_epoch = textures.answers_epoch();
      slot_texture = e.texture;
      slot_valid_until = e.valid_until;
      ++slot_view_hits;
      if (!texcheck_disabled && (slot_view_hits & 1023) == 0) {
        // Resolve the same fetch words the slow way, then put the cache-hit
        // bookkeeping back: GetView overwrites the pending-texture members.
        void* cached_texture = slot_texture;
        const uint64_t cached_valid_until = slot_valid_until;
        VkImageView fresh =
            textures.GetView(regs->GetTextureFetch(slot), dimension, is_signed, upload);
        ++texcheck_checks;
        if ((texcheck_checks & 255) == 0) {
          REXLOG_INFO("[texcheck] {} comprobaciones, {} fallos (1 de cada 1024 usos de la "
                      "cache de ranuras)",
                      texcheck_checks, texcheck_mismatches);
        }
        if (fresh != e.view) {
          ++texcheck_mismatches;
          if (texcheck_mismatches <= 8) {
            const xenos::xe_gpu_texture_fetch_t f = regs->GetTextureFetch(slot);
            REXLOG_ERROR("[texcheck] MISS: slot {} cached {} but the fetch words give {} "
                         "(base {:08X} fmt {} {}x{} dim {} words {:08X} {:08X} {:08X}) - "
                         "slot view reuse OFF for this session",
                         slot, static_cast<const void*>(e.view), static_cast<const void*>(fresh),
                         f.base_address << 12, uint32_t(f.format), f.size_2d.width + 1,
                         f.size_2d.height + 1, uint32_t(f.dimension), words[0], words[1],
                         words[2]);
          }
          texcheck_disabled = true;
        }
        slot_texture = cached_texture;
        slot_valid_until = cached_valid_until;
      }
      AlbumNote(slot, e.view, e.texture);
      return e.view;
    }
    VkImageView view =
        textures.GetView(regs->GetTextureFetch(slot), dimension, is_signed, upload);
#if COD6_DIAGNOSTICS
    if (diagnostics) {
      const auto fetch = regs->GetTextureFetch(slot);
      const auto* resolved = textures.FindResolveDestination(fetch.base_address << 12);
      if (uint32_t(fetch.format) == 22 || uint32_t(fetch.format) == 23 ||
          (resolved && resolved->format == VK_FORMAT_R32_SFLOAT)) {
        NoteShadow(3, fetch.base_address << 12,
                   uint64_t(uint32_t(fetch.format)) | (uint64_t(slot) << 32),
                   uint64_t(view != VK_NULL_HANDLE) | (uint64_t(resolved != nullptr) << 1));
      }
    }
#endif
    if (REXCVAR_GET(cod6_debug_mip_watch)) WatchMips(regs->GetTextureFetch(slot));
    if (trace_frame) {
      xenos::xe_gpu_texture_fetch_t f = regs->GetTextureFetch(slot);
      const std::string key = fmt::format(
          "fmt {:2} dim {} tiled {} packed_mips {} endian {} mips {}-{} signs {}{}{}{} {}", uint32_t(f.format),
          uint32_t(f.dimension), uint32_t(f.tiled), uint32_t(f.packed_mips), uint32_t(f.endianness),
          uint32_t(f.mip_min_level), uint32_t(f.mip_max_level), uint32_t(f.sign_x), uint32_t(f.sign_y),
          uint32_t(f.sign_z), uint32_t(f.sign_w), view ? "" : "(no view)");
      auto& entry = trace_texture_formats[key];
      if (entry.second.empty()) {
        entry.second = fmt::format("{:08X} mips at {:08X} {}x{} pitch {}", f.base_address << 12, f.mip_address << 12,
                                   f.size_2d.width + 1, f.size_2d.height + 1, uint32_t(f.pitch));
      }
      ++entry.first;
      // Queue level 0 of the formats under suspicion for saving after the frame.
      const uint32_t fmt_id = uint32_t(f.format);
      const uint32_t address = f.base_address << 12;
      static const std::unordered_map<uint32_t, uint32_t> kDumpQuota = {{18, 4}, {19, 3}, {20, 4}, {49, 4}};
      auto quota = kDumpQuota.find(fmt_id);
      if (quota != kDumpQuota.end() && f.size_2d.width >= 127 && trace_texture_quota[fmt_id] < quota->second &&
          trace_texture_dumped.insert(address).second) {
        VkFormat host_format = VK_FORMAT_UNDEFINED;
        VkImage image = TextureCache::DebugTextureImage(textures.last_texture(), &host_format);
        if (image) {
          if (fmt_id == 18 && f.size_2d.width + 1 == 512 && !trace_raw_mip_address) {
            trace_raw_mip_address = f.mip_address << 12;
            trace_raw_base_address = f.base_address << 12;
          }
          ++trace_texture_quota[fmt_id];
          trace_texture_dumps.push_back({image, f.size_2d.width + 1u, f.size_2d.height + 1u, host_format,
                                         fmt::format("f{}_tex_fmt{}_{:08X}", frame, fmt_id, address)});
        }
      }
    }
    e.texture = slot_texture = textures.last_texture();
    e.valid_until = slot_valid_until = textures.last_valid_until();
    e.frame = frame;
    e.submission = submission_number;
    e.views_epoch = textures.views_epoch();
    e.answers_epoch = textures.answers_epoch();
    e.variant = variant;
    std::memcpy(e.words, words, sizeof(e.words));
    e.view = view;
    AlbumNote(slot, view, slot_texture);
    return view;
  }

  VkSampler SlotSampler(const SpirvShader::SamplerBinding& binding) {
    uint32_t slot = binding.fetch_constant;
    const uint32_t* words = &(*regs)[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 +
                                     slot * 6];
    uint32_t bits = uint32_t(binding.mag_filter) | (uint32_t(binding.min_filter) << 2) |
                    (uint32_t(binding.mip_filter) << 4) |
                    (uint32_t(binding.aniso_filter) << 6);
    SlotSamplerEntry* entries = slot_samplers[slot];
    for (uint32_t i = 0; i < 4; ++i) {
      SlotSamplerEntry& e = entries[i];
      if (e.valid && e.binding_bits == bits && !std::memcmp(e.words, words, sizeof(e.words))) {
        return e.sampler;
      }
    }
    VkSampler sampler = textures.GetSampler(regs->GetTextureFetch(slot), binding);
    // Samplers are never destroyed, so these stay valid across frames.
    SlotSamplerEntry& e = entries[(bits ^ words[0]) & 3];
    e.valid = true;
    e.binding_bits = bits;
    std::memcpy(e.words, words, sizeof(e.words));
    e.sampler = sampler;
    return sampler;
  }
  uint64_t tb_cache_frame = 0;

  // The fetch constants as the shaders get them: the guest's, with vertex
  // buffer addresses pointing into the stream.
  std::array<uint32_t, 6 * 32> fetch_words = {};
  std::array<uint64_t, 96> fetch_resolved_frame = {};
  uint64_t fetch_words_frame = 0;
  uint64_t fetch_words_submission = 0;
  uint32_t last_fetch_words[6 * 32] = {};

  // cod5-nx: vertex copies limited to the range of vertices a draw reads (see VertexIndexAnalyzer).
  std::unordered_map<const Shader*, VertexShaderRanges> vs_ranges;
  VertexIndexAnalyzer vs_analyzer;
  // What part of each fetch slot was copied this frame, and the fetch word that points at it.
  struct SlotRange {
    uint64_t frame = 0;
    uint64_t submission = 0;
    uint32_t base = 0;   // guest address of the buffer
    uint32_t bytes = 0;  // its size
    uint32_t start = 0;  // copied bytes [start, end) of the buffer
    uint32_t end = 0;
    uint32_t dword0 = 0;
    uint64_t memory_epoch = UINT64_MAX;
  };
  std::array<SlotRange, 96> slot_ranges = {};
  // Lowest and highest index of guest index buffers, by address and size, valid while the content hash matches.
  struct IndexBounds {
    uint64_t hash = 0;
    uint32_t min = 0;
    uint32_t max = 0;
  };
  FlatMap64<IndexBounds> index_bounds;
  void CopyVertices(uint8_t* out, const uint8_t* source, uint32_t bytes, bool whole) {
    const uint64_t start = NowNs();
    std::memcpy(out, source, bytes);
    stats().vertex_copy_ns += NowNs() - start;
    stats().vertex_bytes_copied += bytes;
    if (whole) stats().vertex_bytes_whole += bytes;
  }

  const VertexShaderRanges& GetVertexShaderRanges(VulkanShader* vs) {
    auto it = vs_ranges.find(vs);
    if (it != vs_ranges.end()) return it->second;
    VertexShaderRanges info;
    // A modification no draw uses: the walk leaves an empty translation behind under it.
    auto* translation = vs->GetOrCreateTranslation(0xB01F00000000B01Full);
    if (translation && vs_analyzer.TranslateAnalyzedShader(*translation)) {
      info.index_safe = vs_analyzer.safe();
    }
    for (const auto& binding : vs->vertex_bindings()) {
      VertexShaderRanges::Binding b = {binding.fetch_constant, binding.stride_words * 4, 16, true};
      for (const auto& attribute : binding.attributes) {
        const int32_t offset = attribute.fetch_instr.attributes.offset;
        if (offset < 0) b.usable = false;
        b.vertex_extent = std::max(b.vertex_extent, uint32_t(std::max(offset, 0)) * 4 + 16);
      }
      info.bindings.push_back(b);
    }
    REXLOG_INFO("[native] vertex shader {:016X}: {} binding(s), fetches {}", vs->ucode_data_hash(),
                info.bindings.size(), info.index_safe ? "by vertex index (ranged copies)" : "unsafe (whole copies)");
    return vs_ranges.emplace(vs, std::move(info)).first->second;
  }

  uint32_t bound_offsets[SpirvShaderTranslator::kConstantBufferCount] = {};
  bool bound_offsets_valid = false;
  VkDescriptorSet bound_texture_sets[2] = {};
  VkDeviceSize bound_index_offset = 0;
  VkIndexType bound_index_type = VK_INDEX_TYPE_UINT16;
  bool bound_index_valid = false;

  // The last upload of each constant buffer, reused by the next draws while
  // the registers behind it are unchanged (the ring counts register writes)
  // and the stream still holds it (same submission).
  struct ReusedUpload {
    uint64_t submission = 0;
    uint64_t generation = 0;
    const void* shader = nullptr;
    uint64_t frame = 0;
    VkDeviceSize offset = 0;
  };
  ReusedUpload last_fetch, last_vs_float, last_ps_float, last_bool_loop;
  SpirvShaderTranslator::SystemConstants last_system_constants = {};
  ReusedUpload last_system;
  bool Reusable(const ReusedUpload& u, uint64_t generation, const void* shader,
                bool per_frame = false) const {
    return u.submission == submission_number && u.generation == generation &&
           u.shader == shader && (!per_frame || u.frame == frame);
  }

  const VulkanDevice::Functions& dfn() const { return device->functions(); }
  // Not in ReXGlue's device function table.
  PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
  PFN_vkCmdResolveImage vkCmdResolveImage = nullptr;
  PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
  Submission& current() { return submissions[submission_number % kSubmissionSlots]; }
  // Waits for queued draws first: the recording thread and this one never
  // record into the command buffer at the same time.
  VkCommandBuffer cmd() {
    DrainRecorder();
    return current().main;
  }

  // --- Recording thread ------------------------------------------------------------
  // A draw's Vulkan calls (binds, dynamic state, the draw), decided on the
  // ring thread and made on the recording thread (cod6_native_record_thread)
  // or straight away. In the busiest areas the driver's share of a draw was
  // about a quarter of the ring thread's time; on another core it overlaps
  // with the ring preparing the next draw. Everything else recorded into the
  // main command buffer goes through cmd(), which waits for the queue to
  // empty, and the texture uploads go into the setup command buffer, which
  // has a pool of its own so the two threads never share one.
  struct DrawCommand {
    VkCommandBuffer commands;
    uint32_t flags;
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkDescriptorSet set_shared;
    VkDescriptorSet set_constants;
    uint32_t offsets[SpirvShaderTranslator::kConstantBufferCount];
    VkDescriptorSet texture_sets[2];
    VkViewport viewport;
    VkRect2D scissor;
    float bias_constant, bias_slope;
    float blend[4];
    uint32_t stencil[6];
    VkBuffer index_buffer;
    VkDeviceSize index_offset;
    VkIndexType index_type;
    uint32_t count;
#if COD6_INSTANCE_CONSTANTS
    VkBuffer instance_buffer;
    VkDeviceSize instance_offset;
#endif
    uint32_t instance_count;
  };
#if COD6_INSTANCE_CONSTANTS
  struct InstanceDrawSignature {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    RenderTarget* targets[5] = {};
    VkDescriptorSet textures[2] = {};
    uint32_t constant_offsets[SpirvShaderTranslator::kConstantBufferCount] = {};
    VkViewport viewport = {};
    VkRect2D scissor = {};
    float bias[2] = {};
    float blend[4] = {};
    uint32_t stencil[6] = {};
    VkDeviceSize index_offset = 0;
    VkIndexType index_type = VK_INDEX_TYPE_UINT16;
    uint32_t count = 0;
    bool indexed = false;

    bool operator==(const InstanceDrawSignature& other) const {
      return pipeline == other.pipeline && layout == other.layout &&
             !std::memcmp(targets, other.targets, sizeof(targets)) &&
             !std::memcmp(textures, other.textures, sizeof(textures)) &&
             !std::memcmp(constant_offsets, other.constant_offsets,
                          sizeof(constant_offsets)) &&
             !std::memcmp(&viewport, &other.viewport, sizeof(viewport)) &&
             !std::memcmp(&scissor, &other.scissor, sizeof(scissor)) &&
             !std::memcmp(bias, other.bias, sizeof(bias)) &&
             !std::memcmp(blend, other.blend, sizeof(blend)) &&
             !std::memcmp(stencil, other.stencil, sizeof(stencil)) &&
             index_offset == other.index_offset && index_type == other.index_type &&
             count == other.count && indexed == other.indexed;
    }
  };
  static constexpr uint32_t kMaxInstancesPerDraw = 32;
  AdjacentInstanceGroup<DrawCommand, InstanceDrawSignature, VkDeviceSize,
                        kMaxInstancesPerDraw>
      instance_group;
#endif
  enum : uint32_t {
    kDrawPipeline = 1u << 0,
    kDrawSharedSet = 1u << 1,
    kDrawConstants = 1u << 2,
    kDrawTexturesVertex = 1u << 3,
    kDrawTexturesPixel = 1u << 4,
    kDrawViewport = 1u << 5,
    kDrawScissor = 1u << 6,
    kDrawDepthBias = 1u << 7,
    kDrawBlend = 1u << 8,
    kDrawStencilCompare = 1u << 9,
    kDrawStencilWrite = 1u << 10,
    kDrawStencilReference = 1u << 11,
    kDrawIndexBuffer = 1u << 12,
    kDrawIndexed = 1u << 13,
#if COD6_INSTANCE_CONSTANTS
    kDrawInstanceBuffer = 1u << 14,
#endif
  };
  void ExecuteDrawCommand(const DrawCommand& d) const {
    VkCommandBuffer c = d.commands;
    const auto& f = device->functions();
    if (d.flags & kDrawPipeline) {
      f.vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_GRAPHICS, d.pipeline);
    }
    if (d.flags & kDrawSharedSet) {
      f.vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_GRAPHICS, d.layout,
                                SpirvShaderTranslator::kDescriptorSetSharedMemoryAndEdram, 1,
                                &d.set_shared, 0, nullptr);
    }
    if (d.flags & kDrawConstants) {
      f.vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_GRAPHICS, d.layout,
                                SpirvShaderTranslator::kDescriptorSetConstants, 1,
                                &d.set_constants, SpirvShaderTranslator::kConstantBufferCount,
                                d.offsets);
    }
    if (d.flags & kDrawTexturesVertex) {
      f.vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_GRAPHICS, d.layout,
                                SpirvShaderTranslator::kDescriptorSetTexturesVertex, 1,
                                &d.texture_sets[0], 0, nullptr);
    }
    if (d.flags & kDrawTexturesPixel) {
      f.vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_GRAPHICS, d.layout,
                                SpirvShaderTranslator::kDescriptorSetTexturesPixel, 1,
                                &d.texture_sets[1], 0, nullptr);
    }
    if (d.flags & kDrawViewport) f.vkCmdSetViewport(c, 0, 1, &d.viewport);
    if (d.flags & kDrawScissor) f.vkCmdSetScissor(c, 0, 1, &d.scissor);
    if (d.flags & kDrawDepthBias) f.vkCmdSetDepthBias(c, d.bias_constant, 0.0f, d.bias_slope);
    if (d.flags & kDrawBlend) f.vkCmdSetBlendConstants(c, d.blend);
    if (d.flags & kDrawStencilCompare) {
      f.vkCmdSetStencilCompareMask(c, VK_STENCIL_FACE_FRONT_BIT, d.stencil[0]);
      f.vkCmdSetStencilCompareMask(c, VK_STENCIL_FACE_BACK_BIT, d.stencil[1]);
    }
    if (d.flags & kDrawStencilWrite) {
      f.vkCmdSetStencilWriteMask(c, VK_STENCIL_FACE_FRONT_BIT, d.stencil[2]);
      f.vkCmdSetStencilWriteMask(c, VK_STENCIL_FACE_BACK_BIT, d.stencil[3]);
    }
    if (d.flags & kDrawStencilReference) {
      f.vkCmdSetStencilReference(c, VK_STENCIL_FACE_FRONT_BIT, d.stencil[4]);
      f.vkCmdSetStencilReference(c, VK_STENCIL_FACE_BACK_BIT, d.stencil[5]);
    }
    if (d.flags & kDrawIndexBuffer) {
      f.vkCmdBindIndexBuffer(c, d.index_buffer, d.index_offset, d.index_type);
    }
#if COD6_INSTANCE_CONSTANTS
    if (d.flags & kDrawInstanceBuffer) {
      f.vkCmdBindVertexBuffers(c, 0, 1, &d.instance_buffer, &d.instance_offset);
    }
#endif
    if (d.flags & kDrawIndexed) {
      f.vkCmdDrawIndexed(c, d.count, d.instance_count, 0, 0, 0);
    } else {
      f.vkCmdDraw(c, d.count, d.instance_count, 0, 0);
    }
  }
  std::unique_ptr<BatchRecorder<DrawCommand>> batch_recorder;
  static constexpr uint32_t kRecordQueueSize = 4096;
  std::unique_ptr<DrawCommand[]> record_queue;
  std::atomic<uint32_t> record_head{0};
  std::atomic<uint32_t> record_tail{0};
  std::atomic<bool> record_stop{false};
  std::atomic<bool> record_sleeping{false};
  std::mutex record_mutex;
  std::condition_variable record_cv;
  std::thread record_thread;
  bool record_threaded = false;

  void WakeRecorder() {
    if (record_sleeping.load(std::memory_order_seq_cst)) {
      std::lock_guard<std::mutex> lock(record_mutex);
      record_cv.notify_one();
    }
  }

  void RecordThread() {
#if defined(__SWITCH__)
    /* COD6_COREMASK: fuera de los nucleos del juego (0/1), a 2-3. Ver batch_recorder. */
    svcSetThreadPriority(CUR_THREAD_HANDLE, 0x2C);
    svcSetThreadCoreMask(CUR_THREAD_HANDLE, 2, 0xC);
#endif
    uint32_t idle = 0;
    while (true) {
      uint32_t tail = record_tail.load(std::memory_order_relaxed);
      if (tail != record_head.load(std::memory_order_acquire)) {
        ExecuteDrawCommand(record_queue[tail % kRecordQueueSize]);
        record_tail.store(tail + 1, std::memory_order_release);
        idle = 0;
        continue;
      }
      if (record_stop.load(std::memory_order_acquire)) return;
      // Briefly: spinning longer took cores from the game's own threads.
      if (++idle < 16) {
        std::this_thread::yield();
        continue;
      }
      std::unique_lock<std::mutex> lock(record_mutex);
      record_sleeping.store(true, std::memory_order_seq_cst);
      if (record_head.load(std::memory_order_seq_cst) == tail &&
          !record_stop.load(std::memory_order_acquire)) {
        record_cv.wait_for(lock, std::chrono::milliseconds(2));
      }
      record_sleeping.store(false, std::memory_order_relaxed);
      idle = 0;
    }
  }

  void SubmitDraw(const DrawCommand& d) {
    {   /* COD6_PRODUCER_NS: hueco desde el SubmitDraw anterior. */
      const uint64_t t = NowNs();
      if (g_draw_gap_last) { g_draw_gap_ns += t - g_draw_gap_last; ++g_draw_gap_count; }
      g_draw_gap_last = t;
    }
    if (!record_threaded) {
      ExecuteDrawCommand(d);
      return;
    }
    if(batch_recorder) {batch_recorder->Enqueue(d);return;}
    uint32_t head = record_head.load(std::memory_order_relaxed);
    while (head - record_tail.load(std::memory_order_acquire) >= kRecordQueueSize) {
      WakeRecorder();
      std::this_thread::yield();
    }
    record_queue[head % kRecordQueueSize] = d;
    record_head.store(head + 1, std::memory_order_seq_cst);
    // In batches: the recording thread kept up draw by draw, slept between
    // them, and waking it for every draw (a lock and a system call) showed as
    // 6% of this thread in lock contention. It is also woken when this
    // thread is about to wait for the game (KickRecorder), and anything
    // recording into the command buffer drains the queue first.
    if (head + 1 - record_tail.load(std::memory_order_relaxed) >= kRecordBatch) WakeRecorder();
  }
  static constexpr uint32_t kRecordBatch = 32;

#if COD6_INSTANCE_CONSTANTS
  void EmitInstanceGroup(const DrawCommand& first,
                         const VkDeviceSize* offsets, uint32_t count) {
    DrawCommand merged = first;
    merged.instance_offset = offsets[0];
    merged.instance_count = count;
    for (uint32_t i = 1; i < count; ++i) {
      assert_true(offsets[i] ==
                  offsets[0] + VkDeviceSize(i) * sizeof(InstanceConstantRecord));
    }
#if COD6_DIAGNOSTICS
    stats().instance_runtime_guest_draws += count;
    ++stats().instance_runtime_host_draws;
    if (count > 1) {
      ++stats().instance_runtime_groups;
    } else {
      ++stats().instance_runtime_singles;
    }
    stats().instance_runtime_max_group =
        std::max<uint64_t>(stats().instance_runtime_max_group, count);
#endif
    SubmitDraw(merged);
  }

  void FlushInstanceGroup() {
    instance_group.Flush([this](const DrawCommand& first,
                                const VkDeviceSize* offsets, uint32_t count) {
      EmitInstanceGroup(first, offsets, count);
    });
  }
#endif

  void KickRecorder() {
    if(batch_recorder) {batch_recorder->Publish();return;}
    if (record_threaded &&
        record_tail.load(std::memory_order_acquire) != record_head.load(std::memory_order_relaxed)) {
      WakeRecorder();
    }
  }

  void DrainRecorder() {
    if (!record_threaded) return;
    if(batch_recorder) {
      const auto before=batch_recorder->ReadStats().drain_wait_ns;
      batch_recorder->Drain();
      frame_drain_wait_ns+=batch_recorder->ReadStats().drain_wait_ns-before;
      return;
    }
    uint32_t head = record_head.load(std::memory_order_relaxed);
    if (record_tail.load(std::memory_order_acquire) == head) return;
    uint64_t start = NowNs();
    uint32_t spins = 0;
    while (record_tail.load(std::memory_order_acquire) != head) {
      WakeRecorder();
      if (++spins > 32) std::this_thread::yield();
    }
    frame_drain_wait_ns += NowNs() - start;
  }
  RendererStats& stats() { return owner->stats_; }

  // --- Setup ---------------------------------------------------------------------

  bool Initialize();
  void Shutdown();
  bool CreateSwapResources();

  // --- Submissions and the stream ----------------------------------------------

  // A frame is complete when every submission up to it is: a frame can be
  // split over several submissions (when the stream fills up), and counting it
  // complete when its first one finished freed textures and staging memory the
  // rest still used, an MMU fault that lost the GPU. The frame being recorded
  // is never complete.
  void UpdateCompletedFrame() {
    uint64_t oldest = frame;
    for (const Submission& other : submissions) {
      if (other.pending) oldest = std::min(oldest, other.frame);
    }
    if (oldest > 0) {
      completed_frame = std::max(completed_frame, oldest - 1);
    }
  }

  void Completed(Submission& s) {
    s.pending = false;
    UpdateCompletedFrame();
#if COD6_DIAGNOSTICS
    ProfileCollect(uint32_t(&s - submissions));
#endif
    if (s.timed && timestamps) {
      uint32_t slot = uint32_t(&s - submissions);
      uint64_t ticks[2] = {};
      if (dfn().vkGetQueryPoolResults(vk, timestamps, slot * 2, 2, sizeof(ticks), ticks,
                                      sizeof(uint64_t), VK_QUERY_RESULT_64_BIT) == VK_SUCCESS &&
          ticks[1] > ticks[0]) {
        stats().gpu_ns += uint64_t(double(ticks[1] - ticks[0]) * timestamp_period);
        ++stats().gpu_submissions;
      }
      s.timed = false;
    }
  }

  // This frame's waits and texture time (RendererStats::frame_*).
  uint64_t frame_upload_wait_ns = 0, frame_fence_wait_ns = 0, frame_drain_wait_ns = 0,
           frame_texture_ns = 0, frame_draw_count = 0, frame_stream_wait_ns = 0;
  static uint64_t NowNs() {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count());
  }

  template <typename Map>
  void ClearMapMeasured(Map& map, size_t index, uint64_t capacity) {
#if COD6_DIAGNOSTICS
    if (diagnostics) {
      const uint64_t start = NowNs();
      const uint64_t entries = map.size();
      map.clear();
      ++stats().flat_clear_count[index];
      stats().flat_clear_entries[index] += entries;
      stats().flat_clear_capacity[index] += capacity;
      stats().flat_clear_ns[index] += NowNs() - start;
      return;
    }
#endif
    map.clear();
  }

  void WaitSubmission(Submission& s) {
    if (s.pending) {
      uint64_t start = NowNs();
      CheckGpuResult(dfn().vkWaitForFences(vk, 1, &s.fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");
      frame_fence_wait_ns += NowNs() - start;
      Completed(s);
    }
  }

  void PollCompleted() {
    for (Submission& s : submissions) {
      if (s.pending) {
        const VkResult result = dfn().vkGetFenceStatus(vk, s.fence);
        if (result == VK_SUCCESS) Completed(s);
        else if (result != VK_NOT_READY) CheckGpuResult(result, "vkGetFenceStatus");
      }
    }
    UpdateCompletedFrame();
  }

  void BeginRecording() {
    if (recording) {
      return;
    }
    ++submission_number;
    Submission& s = current();
    WaitSubmission(s);
    CheckGpuResult(dfn().vkResetFences(vk, 1, &s.fence), "vkResetFences");
    CheckGpuResult(dfn().vkResetCommandPool(vk, s.pool, 0), "vkResetCommandPool(setup)");
    CheckGpuResult(dfn().vkResetCommandPool(vk, s.main_pool, 0), "vkResetCommandPool(main)");
    for (VkDescriptorPool pool : s.descriptor_pools) {
      CheckGpuResult(dfn().vkResetDescriptorPool(vk, pool, 0), "vkResetDescriptorPool");
    }
    s.descriptor_pool_used = 0;
    s.number = submission_number;
    s.frame = frame;
    s.stream_oldest = stream_head;
#if COD6_INSTANCE_CONSTANTS
    instance_record_cursors[submission_number % kSubmissionSlots] = 0;
#endif
    VkCommandBufferBeginInfo begin = {};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CheckGpuResult(dfn().vkBeginCommandBuffer(s.setup, &begin), "vkBeginCommandBuffer(setup)");
    CheckGpuResult(dfn().vkBeginCommandBuffer(s.main, &begin), "vkBeginCommandBuffer(main)");
    if (timestamps) {
      uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
      dfn().vkCmdResetQueryPool(s.setup, timestamps, slot * 2, 2);
      dfn().vkCmdWriteTimestamp(s.setup, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestamps, slot * 2);
      s.timed = true;
    }
    if (!null_images_initialized) {
      textures.InitializeNullImages(s.setup);
      null_images_initialized = true;
    }
#if COD6_DIAGNOSTICS
    {
      uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
      profile_segment_counts[slot] = 0;
      profile_closed[slot] = false;
      if (profile_frame && profile_pool) {
        dfn().vkCmdResetQueryPool(s.setup, profile_pool, slot * kProfileQueries,
                                  kProfileQueries);
      }
    }
#endif
    // Everything the setup command buffer uploads is visible to the draws.
    VkMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                            VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    dfn().vkCmdPipelineBarrier(s.main, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrier, 0, nullptr, 0,
                               nullptr);
    recording = true;
    bound_pipeline = VK_NULL_HANDLE;
    bound_layout = VK_NULL_HANDLE;
    bound_static_sets = false;
    std::memset(&bound_viewport, 0, sizeof(bound_viewport));
    std::memset(&bound_scissor, 0, sizeof(bound_scissor));
    bound_depth_bias[0] = bound_depth_bias[1] = NAN;
    bound_offsets_valid = false;
    bound_texture_sets[0] = bound_texture_sets[1] = VK_NULL_HANDLE;
    bound_index_valid = false;
    for (float& f : bound_blend_constants) f = NAN;
    for (uint32_t& v : bound_stencil) v = ~0u;
  }

  void Submit() {
    if (!recording) {
      return;
    }
    DrainRecorder();
    EndPass();
    // The setup command buffer copies staging data the upload worker fills.
    {
      uint64_t start = NowNs();
      textures.WaitForUploads();
      frame_upload_wait_ns += NowNs() - start;
    }
    // Submission boundaries order everything anyway.
    resolve_barrier_pending = false;
    Submission& s = current();
#if COD6_DIAGNOSTICS
    {
      uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
      const uint32_t count = profile_segment_counts[slot];
      if (profile_pool && count) {
        dfn().vkCmdWriteTimestamp(s.main, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, profile_pool,
                                  slot * kProfileQueries + count);
        profile_closed[slot] = true;
      }
    }
#endif
    if (s.timed) {
      dfn().vkCmdWriteTimestamp(s.main, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestamps,
                                uint32_t(submission_number % kSubmissionSlots) * 2 + 1);
    }
    CheckGpuResult(dfn().vkEndCommandBuffer(s.setup), "vkEndCommandBuffer(setup)");
    CheckGpuResult(dfn().vkEndCommandBuffer(s.main), "vkEndCommandBuffer(main)");
    VkCommandBuffer buffers[] = {s.setup, s.main};
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 2;
    submit.pCommandBuffers = buffers;
    VkResult submit_result;
    {
      auto queue = device->AcquireQueue(device->queue_family_graphics_compute(), 0);
      submit_result = dfn().vkQueueSubmit(queue.queue(), 1, &submit, s.fence);
    }
    CheckGpuResult(submit_result, "vkQueueSubmit");
    s.pending = true;
    recording = false;
    ++stats().submissions;
  }

  // Stream memory for this submission; stalls on older submissions if the
  // ring would run over data they still read.
  // The start of the stream's next lap. Not rex::align: that rounds with a
  // mask, right only for powers of two, and the stream is 160 MB. Wrapping
  // with it put allocations at arbitrary places, far enough ahead that the
  // stream looked full and the renderer waited for the GPU to finish all its
  // work (30-45 ms, every wrap), or across the end of the buffer.
  static uint64_t NextStreamLap(uint64_t position) {
    return (position / kStreamSize + 1) * kStreamSize;
  }

  uint8_t* Allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& offset_out) {
    if (size > kStreamSize / 4) {
      return nullptr;
    }
    BeginRecording();
    // Allocations stay kStreamGuard short of the end, so a constant binding's
    // fixed range starting at any allocation stays inside the buffer.
    uint64_t start = rex::align(stream_head, uint64_t(alignment));
    if ((start % kStreamSize) + size > kStreamSize - kStreamGuard) {
      start = NextStreamLap(start);
    }
    uint64_t end = start + size;
    // The current submission itself must not wrap onto what it still reads.
    const uint64_t fence_before = frame_fence_wait_ns;
    if (end - current().stream_oldest > kStreamSize) {
#if COD6_DIAGNOSTICS
      if (preparing_draw) ++stats().stream_rollovers_during_draw;
#endif
      Submit();
      WaitSubmission(submissions[submission_number % kSubmissionSlots]);
      BeginRecording();
      start = rex::align(stream_head, uint64_t(alignment));
      if ((start % kStreamSize) + size > kStreamSize - kStreamGuard) {
        start = NextStreamLap(start);
      }
      end = start + size;
    }
    for (Submission& s : submissions) {
      if (&s != &current() && s.pending && end - s.stream_oldest > kStreamSize) {
        WaitSubmission(s);
      }
    }
    frame_stream_wait_ns += frame_fence_wait_ns - fence_before;
    stream_head = end;
    offset_out = VkDeviceSize(start % kStreamSize);
    return stream_mapping + offset_out;
  }

  VkDescriptorSet AllocatePersistentSet(VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    VkDescriptorSet set = VK_NULL_HANDLE;
    while (true) {
      if (set_pool_used >= set_pools.size()) {
        VkDescriptorPoolSize sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16384},
            {VK_DESCRIPTOR_TYPE_SAMPLER, 16384},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.maxSets = 4096;
        pool_info.poolSizeCount = 2;
        pool_info.pPoolSizes = sizes;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        if (dfn().vkCreateDescriptorPool(vk, &pool_info, nullptr, &pool) != VK_SUCCESS) {
          return VK_NULL_HANDLE;
        }
        set_pools.push_back(pool);
      }
      info.descriptorPool = set_pools[set_pool_used];
      if (dfn().vkAllocateDescriptorSets(vk, &info, &set) == VK_SUCCESS) {
        return set;
      }
      ++set_pool_used;
    }
  }

  // Forgets every cached set; the pools are freed once the GPU is done with
  // the frames that may use them.
  // Bumped when the cached texture sets are dropped: their pools are freed
  // once the GPU is done, so bindings kept from earlier frames must not name
  // them any more.
  uint64_t set_cache_generation = 1;
  void FlushSetCache() {
    ++set_cache_generation;
    texture_set_cache.clear();
    if (!set_pools.empty()) {
      retired_set_pools.push_back({frame, std::move(set_pools)});
      set_pools.clear();
    }
    set_pool_used = 0;
  }

  void FreeRetiredSetPools() {
    for (auto it = retired_set_pools.begin(); it != retired_set_pools.end();) {
      if (it->frame <= completed_frame) {
        for (VkDescriptorPool pool : it->pools) {
          dfn().vkDestroyDescriptorPool(vk, pool, nullptr);
        }
        it = retired_set_pools.erase(it);
      } else {
        ++it;
      }
    }
  }

  VkDescriptorSet AllocateSet(VkDescriptorSetLayout layout) {
    Submission& s = current();
    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    VkDescriptorSet set = VK_NULL_HANDLE;
    while (true) {
      if (s.descriptor_pool_used >= s.descriptor_pools.size()) {
        VkDescriptorPoolSize sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16384},
            {VK_DESCRIPTOR_TYPE_SAMPLER, 16384},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 256},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.maxSets = 4096;
        pool_info.poolSizeCount = 3;
        pool_info.pPoolSizes = sizes;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        if (dfn().vkCreateDescriptorPool(vk, &pool_info, nullptr, &pool) != VK_SUCCESS) {
          return VK_NULL_HANDLE;
        }
        s.descriptor_pools.push_back(pool);
      }
      info.descriptorPool = s.descriptor_pools[s.descriptor_pool_used];
      if (dfn().vkAllocateDescriptorSets(vk, &info, &set) == VK_SUCCESS) {
        return set;
      }
      ++s.descriptor_pool_used;
    }
  }

  // --- Shaders, layouts and pipelines ----------------------------------------------

  VkDescriptorSetLayout GetTextureSetLayout(bool vertex, uint32_t textures, uint32_t samplers);
  const PipelineLayout* GetPipelineLayout(uint32_t tex_vs, uint32_t samp_vs, uint32_t tex_ps,
                                          uint32_t samp_ps);
  bool EnsureTranslated(VulkanShader::VulkanTranslation* translation, bool* pending_out);
  void DrainTranslations();
  void TranslationThread();
  std::unique_ptr<SpirvShaderTranslator> worker_translator;
  // 1 = being translated on the worker, 2 = done.
  std::unordered_map<const void*, uint8_t> translation_state;
  std::thread translation_thread;
  std::mutex translation_mutex;
  std::condition_variable translation_cv;
  std::deque<VulkanShader::VulkanTranslation*> translation_jobs;
  std::vector<VulkanShader::VulkanTranslation*> translations_done;
  std::atomic<uint32_t> translations_done_count{0};
  bool translation_stop = false;
  // SPIR-V from earlier sessions (a file next to the driver cache), by shader type, ucode hash and modification:
  // a hit skips the translation. Only the thread calling EnsureTranslated uses the map; the file is appended to
  // under spirv_cache_mutex by whichever thread translated.
  struct CachedSpirv {
    uint32_t type = 0;
    uint64_t ucode_hash = 0;
    uint64_t modification = 0;
    uint32_t ucode_dwords = 0;
    std::vector<SpirvShader::TextureBinding> textures;
    std::vector<SpirvShader::SamplerBinding> samplers;
    std::vector<uint8_t> binary;
  };
  std::unordered_map<uint64_t, CachedSpirv> spirv_cache;
  std::mutex spirv_cache_mutex;
  FILE* spirv_cache_file = nullptr;
  bool TakeCachedTranslation(VulkanShader::VulkanTranslation* translation);
  void StoreTranslation(VulkanShader::VulkanTranslation* translation);
  struct PipelineJob {
    uint64_t hash = 0;
    PipelineKey key = {};
    VkShaderModule vertex = VK_NULL_HANDLE;
    VkShaderModule pixel = VK_NULL_HANDLE;
    VkShaderModule geometry = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    bool prewarm = false;  // from the list of earlier sessions, compiled at boot
  };
  VkPipeline GetPipeline(const PipelineKey& key, VulkanShader::VulkanTranslation* vs,
                         VulkanShader::VulkanTranslation* ps, VkShaderModule geometry,
                         VkPipelineLayout layout, bool* pending_out);
  VkPipeline CreatePipeline(const PipelineJob& job);
  void DrainCompletedPipelines();
  void PipelineThread();
  std::unordered_set<uint64_t> pending_pipelines;
  std::thread pipeline_thread;
  std::mutex pipeline_mutex;
  std::condition_variable pipeline_cv;
  std::deque<PipelineJob> pipeline_jobs;
  std::vector<std::pair<uint64_t, VkPipeline>> pipelines_completed;
  std::atomic<uint32_t> pipelines_completed_count{0};
  bool pipeline_stop = false;
  // Pipeline list (cod5-nx): the keys of every pipeline earlier sessions made, in a file next to the driver cache.
  // At boot they are compiled on the worker, after any pipeline a draw is waiting for, with shader modules made from
  // the SPIR-V cache; those modules are destroyed once the last of them is compiled.
  std::deque<PipelineJob> prewarm_jobs;
  std::vector<VkShaderModule> prewarm_modules;
  uint32_t prewarm_remaining = 0;  // under pipeline_mutex
  std::chrono::steady_clock::time_point prewarm_start;
  std::unordered_set<uint64_t> listed_pipelines;  // in the file, or queued for it
  std::unordered_map<uint64_t, PipelineKey> pending_list_keys;  // asked for, being compiled
  std::vector<PipelineKey> pipelines_to_list;     // made this session, not yet written
  FILE* pipeline_list_file = nullptr;              // written only by driver_cache_writer
  void ListPipeline(uint64_t hash, const PipelineKey& key);

  // --- Render targets ------------------------------------------------------------

  RenderTarget* GetRenderTarget(const RenderTargetKey& key, uint32_t min_height,
                                uint32_t guest_msaa);
  // cod5-nx: see GetRenderTarget and FindClearAlias.
  RenderTargetKey CanonicalTargetKey(const RenderTargetKey& guest_key);
  RenderTarget* GetRenderTargetCanonical(const RenderTargetKey& key, uint32_t min_height);
  RenderTarget* FindClearAlias(const RenderTargetKey& host_key, uint32_t guest_msaa,
                               uint32_t& scale_x, uint32_t& scale_y);
  bool DepthInsideAnother(const RenderTarget& rt) const;
  void EndPass() {
    if (in_pass) {
#if COD6_INSTANCE_CONSTANTS
      FlushInstanceGroup();
#endif
      dfn().vkCmdEndRendering(cmd());
      in_pass = false;
    }
  }
  bool BeginPass(const PassTargets& targets);
  bool ClearDepthTargetInPass(RenderTarget& target, const VkRect2D& rect,
                              float depth, uint32_t stencil);

  // --- Draws ---------------------------------------------------------------------

  void Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words);
  void UpdateSystemConstants(bool primitive_polygonal, const draw_util::ViewportInfo& viewport,
                             uint32_t used_texture_mask, uint32_t vertex_index_endian_flags,
                             const VulkanShader* vs, const VulkanShader* ps);
  void Resolve();
  bool ResolveDownsampled(RenderTarget* full, int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                          xenos::CopyCommand command);
  bool resolve_barrier_pending = false;
  void FlushResolveBarrier();
  void Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer, uint32_t width, uint32_t height);
  void SaveDriverCache();
};

// --- Setup -------------------------------------------------------------------------

bool Renderer::Impl::Initialize() {
  device = provider->vulkan_device();
  vk = device->device();
  scene_scale = SceneScale::FromPercent(
      REXCVAR_GET(cod6_native_scene_scale_percent));
  REXLOG_INFO(
      "[scene-scale] schema=1 percent={} enabled={} target=color:t0p1040/depth:t975p1040 "
      "guest_rect=1024x600 resolve=guest-size guest_msaa=preserved "
      "exclusions=host-msaa,menu,video,shadow,model-lighting",
      REXCVAR_GET(cod6_native_scene_scale_percent), scene_scale.enabled());
  REXLOG_INFO(
      "[scene-zcull] schema=1 requested={} target=depth:t975p1040 host_msaa=1x "
      "growth_fallback=transfer-dst clear=load-op-clear",
      REXCVAR_GET(cod6_native_scene_zcull));
  ConfigureNvkDrawDriver();
  auto get_proc = device->vulkan_instance()->functions().vkGetDeviceProcAddr;
  vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(
      get_proc(vk, "vkCmdClearDepthStencilImage"));
  vkCmdResolveImage =
      reinterpret_cast<PFN_vkCmdResolveImage>(get_proc(vk, "vkCmdResolveImage"));
  vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(get_proc(vk, "vkDeviceWaitIdle"));
  if (!vkCmdClearDepthStencilImage || !vkCmdResolveImage || !vkDeviceWaitIdle) {
    REXLOG_ERROR("[native] Missing Vulkan device functions");
    return false;
  }
  const auto& properties = device->properties();
  if (!properties.dynamicRendering) {
    REXLOG_ERROR("[native] The native renderer needs dynamic rendering");
    return false;
  }
  specialize_signs = false;
  diagnostics = cod6::diagnostics::kEnabled && REXCVAR_GET(cod6_native_diagnostics);
#if COD6_INSTANCE_CONSTANTS
#if COD6_DIAGNOSTICS
  stats().instance_batching_enabled =
      REXCVAR_GET(cod6_native_instance_batching) ? 1 : 0;
#endif
  REXLOG_INFO("[instance-batching] schema=1 compiled=true enabled={} "
              "max_instances={} records_per_submission={} exact=1 reordered=0",
              REXCVAR_GET(cod6_native_instance_batching), kMaxInstancesPerDraw,
              kInstanceRecordsPerSubmission);
#endif
#if COD6_DIAGNOSTICS
  owner->collect_diagnostics_ = diagnostics;
  REXLOG_INFO("[diagnostic-hot-path] schema=1 compiled=true enabled={} "
              "state_register_slots={} shadow_bucket_slots={}",
              diagnostics, diagnostics ? 0x4000 : 0, diagnostics ? 64 : 0);
#endif
  if (REXCVAR_GET(cod6_native_binding_key_memo)) {
    binding_key_memo = std::make_unique<BindingMemo>(
        REXCVAR_GET(cod6_native_binding_key_memo_capacity));
  }
  stable_setup_identity = REXCVAR_GET(cod6_native_stable_setup_identity);
  if constexpr (cod6::diagnostics::kEnabled) {
    stats().system_constant_stable_setup = stable_setup_identity ? 1 : 0;
    REXLOG_INFO("[system-constants] schema=1 stable_setup_identity={} "
                "dependencies=setup,fetch,index,used_mask final_memcmp=preserved",
                stable_setup_identity);
  }
  if constexpr (cod6::diagnostics::kEnabled) {
    stats().binding_key_enabled = binding_key_memo ? 1 : 0;
    REXLOG_INFO("[binding-key] schema=1 enabled={} capacity={} bytes={} "
                "dependencies=vs,ps,layout,guest_fetch,host_fetch,frame,submission,slot_mask "
                "content_checks=unchanged",
                bool(binding_key_memo), binding_key_memo ? binding_key_memo->capacity() : 0,
                binding_key_memo ? BindingMemo::Bytes() : 0);
  }
  vertex_sync_cache = REXCVAR_GET(cod6_native_vertex_sync_cache);
  REXLOG_INFO("[native] Vertex/index cache synchronization checks: {}", vertex_sync_cache);
  multisample = REXCVAR_GET(cod6_native_msaa);
  ubo_alignment = std::max<VkDeviceSize>(properties.minUniformBufferOffsetAlignment, 16);

  translator = std::make_unique<SpirvShaderTranslator>(SpirvShaderTranslator::Features(device),
                                                       true, true, false);

  VkShaderStageFlags guest_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |
                                    (properties.geometryShader ? VK_SHADER_STAGE_GEOMETRY_BIT : 0);

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &layout_empty) != VK_SUCCESS) {
    return false;
  }
  // Set 0: the translator's shared memory, here the stream. With a small
  // maxStorageBufferRange the translator splits it into several bindings.
  shared_binding_count = 1u << SpirvShaderTranslator::GetSharedMemoryStorageBufferCountLog2(
                             properties.maxStorageBufferRange);
  VkDescriptorSetLayoutBinding shared_binding = {};
  shared_binding.binding = 0;
  shared_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  shared_binding.descriptorCount = shared_binding_count;
  shared_binding.stageFlags = guest_stages;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &shared_binding;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &layout_shared) != VK_SUCCESS) {
    return false;
  }
  // Set 1: the five constant buffers, dynamic so one set serves every draw.
  VkDescriptorSetLayoutBinding constant_bindings[SpirvShaderTranslator::kConstantBufferCount] = {};
  for (uint32_t i = 0; i < SpirvShaderTranslator::kConstantBufferCount; ++i) {
    constant_bindings[i].binding = i;
    constant_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    constant_bindings[i].descriptorCount = 1;
    constant_bindings[i].stageFlags = guest_stages;
  }
  constant_bindings[SpirvShaderTranslator::kConstantBufferFloatVertex].stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT;
  constant_bindings[SpirvShaderTranslator::kConstantBufferFloatPixel].stageFlags =
      VK_SHADER_STAGE_FRAGMENT_BIT;
  layout_info.bindingCount = SpirvShaderTranslator::kConstantBufferCount;
  layout_info.pBindings = constant_bindings;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &layout_constants) !=
      VK_SUCCESS) {
    return false;
  }

  // The stream.
  VkDeviceSize stream_allocation = 0;
  uint32_t stream_memory_type = 0;
  if (!rex::ui::vulkan::util::CreateDedicatedAllocationBuffer(
          device, kStreamSize + kStreamSlack,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          rex::ui::vulkan::util::MemoryPurpose::kUpload, stream_buffer, stream_memory,
          &stream_memory_type, &stream_allocation)) {
    REXLOG_ERROR("[native] Could not create the {} MB stream buffer", kStreamSize >> 20);
    return false;
  }
  if (!(device->memory_types().host_coherent & (1u << stream_memory_type))) {
    REXLOG_WARN("[native] The stream memory is not host-coherent");
  }
  void* mapping = nullptr;
  if (dfn().vkMapMemory(vk, stream_memory, 0, VK_WHOLE_SIZE, 0, &mapping) != VK_SUCCESS) {
    return false;
  }
  stream_mapping = static_cast<uint8_t*>(mapping);
#if COD6_INSTANCE_CONSTANTS
  uint32_t instance_memory_type = 0;
  VkDeviceSize instance_allocation = 0;
  if (!rex::ui::vulkan::util::CreateDedicatedAllocationBuffer(
          device, kInstanceSubmissionBytes * kSubmissionSlots,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          rex::ui::vulkan::util::MemoryPurpose::kUpload, instance_buffer,
          instance_memory, &instance_memory_type, &instance_allocation)) {
    REXLOG_ERROR("[instance-batching] Could not create the {} KB instance buffer",
                 (kInstanceSubmissionBytes * kSubmissionSlots) >> 10);
    return false;
  }
  if (!(device->memory_types().host_coherent & (1u << instance_memory_type))) {
    REXLOG_ERROR("[instance-batching] Instance memory is not host-coherent");
    return false;
  }
  void* instance_map = nullptr;
  if (dfn().vkMapMemory(vk, instance_memory, 0, VK_WHOLE_SIZE, 0,
                        &instance_map) != VK_SUCCESS) {
    return false;
  }
  instance_mapping = static_cast<uint8_t*>(instance_map);
#endif

  VkDescriptorPoolSize static_sizes[] = {
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SpirvShaderTranslator::kConstantBufferCount},
  };
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets = 2;
  pool_info.poolSizeCount = 2;
  pool_info.pPoolSizes = static_sizes;
  if (dfn().vkCreateDescriptorPool(vk, &pool_info, nullptr, &static_pool) != VK_SUCCESS) {
    return false;
  }
  VkDescriptorSetLayout static_layouts[] = {layout_shared, layout_constants};
  VkDescriptorSet static_sets[2];
  VkDescriptorSetAllocateInfo allocate = {};
  allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate.descriptorPool = static_pool;
  allocate.descriptorSetCount = 2;
  allocate.pSetLayouts = static_layouts;
  if (dfn().vkAllocateDescriptorSets(vk, &allocate, static_sets) != VK_SUCCESS) {
    return false;
  }
  set_shared = static_sets[0];
  set_constants = static_sets[1];
  // With several shared memory bindings each covers a slice of guest memory;
  // the stream is far smaller than the first slice, so all of them can point at
  // the whole stream.
  std::vector<VkDescriptorBufferInfo> shared_infos(shared_binding_count);
  VkDeviceSize slice = VkDeviceSize(512u << 20) / shared_binding_count;
  for (uint32_t i = 0; i < shared_binding_count; ++i) {
    // Binding i serves the addresses from i * slice, so it starts there in
    // the stream (clamped for the slices past the end of the stream, which
    // are never addressed).
    VkDeviceSize offset = std::min(VkDeviceSize(i) * slice, kStreamSize - 256);
    offset &= ~(properties.minStorageBufferOffsetAlignment - 1);
    shared_infos[i].buffer = stream_buffer;
    shared_infos[i].offset = offset;
    shared_infos[i].range = kStreamSize + kStreamSlack - offset;
  }
  VkDescriptorBufferInfo constant_infos[SpirvShaderTranslator::kConstantBufferCount];
  VkDeviceSize constant_ranges[] = {sizeof(SpirvShaderTranslator::SystemConstants),
                                    kFloatConstantsBytes, kFloatConstantsBytes,
                                    kBoolLoopConstantsBytes, kFetchConstantsBytes};
  for (uint32_t i = 0; i < SpirvShaderTranslator::kConstantBufferCount; ++i) {
    constant_infos[i].buffer = stream_buffer;
    constant_infos[i].offset = 0;
    constant_infos[i].range = constant_ranges[i];
  }
  constexpr uint32_t write_count = 1 + SpirvShaderTranslator::kConstantBufferCount;
  VkWriteDescriptorSet writes[write_count] = {};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = set_shared;
  writes[0].descriptorCount = shared_binding_count;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  writes[0].pBufferInfo = shared_infos.data();
  MakeConstantDescriptorWrites(set_constants, constant_infos,
                               SpirvShaderTranslator::kConstantBufferCount, writes + 1);
  dfn().vkUpdateDescriptorSets(vk, write_count, writes, 0, nullptr);

  // Submissions.
  for (Submission& s : submissions) {
    VkCommandPoolCreateInfo command_pool = {};
    command_pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    command_pool.queueFamilyIndex = device->queue_family_graphics_compute();
    if (dfn().vkCreateCommandPool(vk, &command_pool, nullptr, &s.pool) != VK_SUCCESS) {
      return false;
    }
    if (dfn().vkCreateCommandPool(vk, &command_pool, nullptr, &s.main_pool) != VK_SUCCESS) {
      return false;
    }
    VkCommandBufferAllocateInfo buffers = {};
    buffers.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffers.commandPool = s.pool;
    buffers.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffers.commandBufferCount = 1;
    if (dfn().vkAllocateCommandBuffers(vk, &buffers, &s.setup) != VK_SUCCESS) {
      return false;
    }
    buffers.commandPool = s.main_pool;
    if (dfn().vkAllocateCommandBuffers(vk, &buffers, &s.main) != VK_SUCCESS) {
      return false;
    }
    VkFenceCreateInfo fence = {};
    fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (dfn().vkCreateFence(vk, &fence, nullptr, &s.fence) != VK_SUCCESS) {
      return false;
    }
  }

  VkQueryPoolCreateInfo query_info = {};
  query_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  query_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
  query_info.queryCount = kSubmissionSlots * 2;
  if (properties.timestampPeriod > 0.0f &&
      dfn().vkCreateQueryPool(vk, &query_info, nullptr, &timestamps) == VK_SUCCESS) {
    timestamp_period = properties.timestampPeriod;
  } else {
    timestamps = VK_NULL_HANDLE;
  }

  VkPipelineCacheCreateInfo cache_info = {};
  cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  dfn().vkCreatePipelineCache(vk, &cache_info, nullptr, &driver_cache);

  if (!textures.Initialize(device, memory)) {
    return false;
  }
  upload.buffer = stream_buffer;
  upload.allocate = [this](size_t size, size_t alignment, VkDeviceSize& offset) {
    uint8_t* result = Allocate(size, alignment, offset);
    // An allocation may have started a new submission.
    upload.setup_commands = current().setup;
    upload.submission = submission_number;
    return result;
  };
  if (!CreateSwapResources()) {
    return false;
  }
  pipeline_thread = std::thread([this]() { PipelineThread(); });
  if (REXCVAR_GET(cod6_native_record_thread)) {
    if(REXCVAR_GET(cod6_native_record_batch_publish)) {
      batch_recorder=std::make_unique<BatchRecorder<DrawCommand>>();
      batch_recorder->Start([this](const DrawCommand& draw){ExecuteDrawCommand(draw);},[] {
#if defined(__SWITCH__)
        /* COD6_COREMASK
         *
         * Antes: prioridad 0x2C, mascara (preferido 1, permitidos 0x3 = nucleos 0 y 1).
         * El hilo de grabacion se peleaba con el hilo del anillo y con el propio juego
         * por los mismos dos nucleos: el registro de la tanda 251 daba CPU 199% de 400%,
         * es decir dos nucleos ocupados y dos enteros sin usar.
         *
         * Ahora: preferido 2, permitidos 0xC (nucleos 2 y 3). Los nucleos 0/1 quedan
         * para el juego. Si el nucleo 2 esta ocupado puede caer al 3, pero no vuelve a
         * los que usa el juego.
         */
        svcSetThreadPriority(CUR_THREAD_HANDLE, 0x2C);
        svcSetThreadCoreMask(CUR_THREAD_HANDLE, 2, 0xC);
#endif
      });
      REXLOG_INFO("[native] batched recorder: {} draws/publication, blocking drain, cores 2-3 (mask 0xC)",
                  BatchRecorder<DrawCommand>::kBatchSize);
    } else {
      record_queue = std::make_unique<DrawCommand[]>(kRecordQueueSize);
      record_thread = std::thread([this]() { RecordThread(); });
      REXLOG_INFO("[native] legacy recorder: per-draw publication and polling drain");
    }
    record_threaded = true;
  }
  worker_translator = std::make_unique<SpirvShaderTranslator>(
      SpirvShaderTranslator::Features(device), true, true, false);
  translation_thread = std::thread([this]() { TranslationThread(); });
  if (!rex::ui::vulkan::util::CreateDedicatedAllocationBuffer(
          device, kDepthScratchSize,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
          rex::ui::vulkan::util::MemoryPurpose::kDeviceLocal, depth_scratch,
          depth_scratch_memory)) {
    depth_scratch = VK_NULL_HANDLE;
    REXLOG_WARN("[native] No scratch buffer for depth resolves");
  }
  if (depth_scratch && !CreateDepthConversion()) {
    REXLOG_WARN("[native] No depth resolve pipeline; depth resolves are not done");
    if (depth_convert_pipeline) dfn().vkDestroyPipeline(vk, depth_convert_pipeline, nullptr);
    depth_convert_pipeline = VK_NULL_HANDLE;
  }
  if (copy_color_pipeline) {
    textures.SetComputeCopy([this](VkCommandBuffer c, VkImageView source, VkImageView dest,
                                   int32_t sx, int32_t sy, int32_t dx, int32_t dy, uint32_t width,
                                   uint32_t height) {
      return ComputeColorCopy(c, source, dest, sx, sy, dx, dy, width, height);
    });
  }
  const bool depth24_requested = REXCVAR_GET(cod6_native_depth24);
  VkFormatProperties depth24_properties = {};
  device->vulkan_instance()->functions().vkGetPhysicalDeviceFormatProperties(
      device->physical_device(), VK_FORMAT_D24_UNORM_S8_UINT, &depth24_properties);
  constexpr VkFormatFeatureFlags kDepth24Required =
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
      VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
      VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
  const VkFormatFeatureFlags depth24_features = depth24_properties.optimalTilingFeatures;
  if (depth24_requested) {
    if ((depth24_features & kDepth24Required) == kDepth24Required &&
        depth_scratch && depth_convert_pipeline) {
      depth24 = true;
    } else {
      REXLOG_WARN("[native] 24-bit depth buffers unavailable; using 32-bit float");
    }
  }
  REXLOG_INFO(
      "[depth-format] schema=1 requested={} enabled={} d24_features={:08X} required={:08X} "
      "unorm={} float={} resolve=scratch-or-compute",
      depth24_requested, depth24, uint32_t(depth24_features), uint32_t(kDepth24Required),
      depth24 ? "D24_UNORM_S8" : "D32_SFLOAT_S8", "D32_SFLOAT");
#if COD6_DIAGNOSTICS
  if (REXCVAR_GET(cod6_native_gpu_profile) && timestamps) {
    VkQueryPoolCreateInfo profile_info = {};
    profile_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    profile_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    profile_info.queryCount = kSubmissionSlots * kProfileQueries;
    if (dfn().vkCreateQueryPool(vk, &profile_info, nullptr, &profile_pool) != VK_SUCCESS) {
      profile_pool = VK_NULL_HANDLE;
    }
  }
#endif
  // The default linear 256-entry gamma ramp (CommandProcessor::Initialize).
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t value = i * 0x3FF / 0xFF;
    gamma_ramp[i] = value | (value << 10) | (value << 20);
  }
  REXLOG_INFO("[native] Renderer ready: {} MB stream, {} shared memory binding(s)",
              kStreamSize >> 20, shared_binding_count);
  return true;
}

bool Renderer::Impl::CreateDepthConversion() {
  VkDescriptorSetLayoutBinding binding = {};
  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  binding.descriptorCount = 1;
  binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &binding;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &depth_convert_set_layout) !=
      VK_SUCCESS) {
    return false;
  }
  VkPushConstantRange push = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 5};
  VkPipelineLayoutCreateInfo pipeline_layout = {};
  pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout.setLayoutCount = 1;
  pipeline_layout.pSetLayouts = &depth_convert_set_layout;
  pipeline_layout.pushConstantRangeCount = 1;
  pipeline_layout.pPushConstantRanges = &push;
  if (dfn().vkCreatePipelineLayout(vk, &pipeline_layout, nullptr, &depth_convert_layout) !=
      VK_SUCCESS) {
    return false;
  }
  depth_convert_pipeline = rex::ui::vulkan::util::CreateComputePipeline(
      device, depth_convert_layout, shaders::kDepthResolveCs, sizeof(shaders::kDepthResolveCs));
  if (!depth_convert_pipeline) {
    return false;
  }
  VkDescriptorPoolSize size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1};
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets = 1;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &size;
  if (dfn().vkCreateDescriptorPool(vk, &pool_info, nullptr, &depth_convert_pool) != VK_SUCCESS) {
    return false;
  }
  VkDescriptorSetAllocateInfo allocate = {};
  allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate.descriptorPool = depth_convert_pool;
  allocate.descriptorSetCount = 1;
  allocate.pSetLayouts = &depth_convert_set_layout;
  if (dfn().vkAllocateDescriptorSets(vk, &allocate, &depth_convert_set) != VK_SUCCESS) {
    return false;
  }
  VkDescriptorBufferInfo buffer = {depth_scratch, 0, kDepthScratchSize};
  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = depth_convert_set;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  write.pBufferInfo = &buffer;
  dfn().vkUpdateDescriptorSets(vk, 1, &write, 0, nullptr);

  VkDescriptorSetLayoutBinding copy_bindings[2] = {};
  copy_bindings[0].binding = 0;
  copy_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  copy_bindings[0].descriptorCount = 1;
  copy_bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  copy_bindings[1] = copy_bindings[0];
  copy_bindings[1].binding = 1;
  copy_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  layout_info.bindingCount = 2;
  layout_info.pBindings = copy_bindings;
  VkPushConstantRange copy_push = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 8};
  pipeline_layout.pSetLayouts = &depth_copy_set_layout;
  pipeline_layout.pPushConstantRanges = &copy_push;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &depth_copy_set_layout) ==
          VK_SUCCESS &&
      dfn().vkCreatePipelineLayout(vk, &pipeline_layout, nullptr, &depth_copy_layout) ==
          VK_SUCCESS) {
    depth_copy_pipeline = rex::ui::vulkan::util::CreateComputePipeline(
        device, depth_copy_layout, shaders::kDepthToFloatCs, sizeof(shaders::kDepthToFloatCs));
  }
  if (!depth_copy_pipeline) {
    REXLOG_WARN("[native] No direct depth resolve; resolving depth through a buffer");
  }

  VkPhysicalDeviceFeatures features = {};
  device->vulkan_instance()->functions().vkGetPhysicalDeviceFeatures(device->physical_device(),
                                                                     &features);
  if (depth_copy_set_layout && features.shaderStorageImageWriteWithoutFormat) {
    VkPushConstantRange color_push = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int32_t) * 6};
    pipeline_layout.pSetLayouts = &depth_copy_set_layout;
    pipeline_layout.pPushConstantRanges = &color_push;
    if (dfn().vkCreatePipelineLayout(vk, &pipeline_layout, nullptr, &copy_color_layout) ==
        VK_SUCCESS) {
      copy_color_pipeline = rex::ui::vulkan::util::CreateComputePipeline(
          device, copy_color_layout, shaders::kCopyColorCs, sizeof(shaders::kCopyColorCs));
    }
  }
  if (!copy_color_pipeline) {
    REXLOG_WARN("[native] No compute color copy; resolving color with vkCmdCopyImage");
  }
  return true;
}

bool Renderer::Impl::ComputeColorCopy(VkCommandBuffer c, VkImageView source, VkImageView dest,
                                      int32_t sx, int32_t sy, int32_t dx, int32_t dy,
                                      uint32_t width, uint32_t height) {
  if (!copy_color_pipeline || !width || !height) return false;
  VkDescriptorSet set = AllocateSet(depth_copy_set_layout);
  if (!set) return false;
  VkDescriptorImageInfo images[2] = {{VK_NULL_HANDLE, source, VK_IMAGE_LAYOUT_GENERAL},
                                     {VK_NULL_HANDLE, dest, VK_IMAGE_LAYOUT_GENERAL}};
  VkWriteDescriptorSet writes[2] = {};
  for (uint32_t i = 0; i < 2; ++i) {
    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[i].dstSet = set;
    writes[i].dstBinding = i;
    writes[i].descriptorCount = 1;
    writes[i].descriptorType =
        i ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writes[i].pImageInfo = &images[i];
  }
  dfn().vkUpdateDescriptorSets(vk, 2, writes, 0, nullptr);
  // Earlier writes of the source (rendering, copies, compute) before the
  // reads, and earlier reads of the destination before the writes.
  VkMemoryBarrier before = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                                VK_ACCESS_SHADER_WRITE_BIT,
                            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
  dfn().vkCmdPipelineBarrier(
      c,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &before, 0, nullptr, 0, nullptr);
  int32_t params[6] = {sx, sy, dx, dy, int32_t(width), int32_t(height)};
  dfn().vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_COMPUTE, copy_color_pipeline);
  dfn().vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_COMPUTE, copy_color_layout, 0, 1, &set,
                                0, nullptr);
  dfn().vkCmdPushConstants(c, copy_color_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params),
                           params);
  dfn().vkCmdDispatch(c, (width + 7) / 8, (height + 7) / 8, 1);
  VkMemoryBarrier after = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                           VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
  dfn().vkCmdPipelineBarrier(
      c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
          VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0, 1, &after, 0, nullptr, 0, nullptr);
  return true;
}

#if COD6_DIAGNOSTICS
void Renderer::Impl::ProfileCollect(uint32_t slot) {
  auto& segments = profile_segments[slot];
  uint32_t& count = profile_segment_counts[slot];
  if (!profile_pool || !profile_closed[slot] || !count) {
    count = 0;
    return;
  }
  profile_closed[slot] = false;
  const uint32_t tick_count = count + 1;
  if (dfn().vkGetQueryPoolResults(vk, profile_pool, slot * kProfileQueries, tick_count,
                                  tick_count * sizeof(uint64_t), profile_ticks.data(), sizeof(uint64_t),
                                  VK_QUERY_RESULT_64_BIT) == VK_SUCCESS) {
    for (uint32_t i = 0; i < count; ++i) {
      if (profile_ticks[i + 1] < profile_ticks[i]) continue;
      double ns = double(profile_ticks[i + 1] - profile_ticks[i]) * timestamp_period;
      ProfileTotal& total = profile_totals[segments[i].key];
      total.ns += ns;
      ++total.count;
      total.draws += segments[i].draws;
      profiled_ns += ns;
    }
  }
  count = 0;
}

void Renderer::Impl::CaptureProfileReport(
    GpuProfileSnapshot<RendererReportSnapshot::kGpuProfileCapacity>& snapshot) {
  snapshot.Reset();
  if (!profile_pool || profiled_frames < 20) return;
  snapshot.frames = profiled_frames;
  snapshot.total_ns = profiled_ns;
  for (const auto& [key, total] : profile_totals) {
    snapshot.Consider({key, total.ns, total.count, total.draws});
  }
  profile_totals.clear();
  profiled_frames = 0;
  profiled_ns = 0;
}

static void FormatGpuProfileReport(
    const GpuProfileSnapshot<RendererReportSnapshot::kGpuProfileCapacity>& snapshot) {
  if (!snapshot.frames) return;
  const double frames = double(snapshot.frames);
  std::string line = fmt::format("[native] GPU by pass, ms per frame (sampled, as the driver "
                                 "reports it): total {:.1f}",
                                 snapshot.total_ns / 1e6 / frames);
  auto target = [](uint64_t bits) -> std::string {
    if (bits == 0xFFFFFF) return "-";
    return fmt::format("t{}p{}{}", (bits >> 12) & 0xFFF, (bits >> 1) & 0x7FF,
                       (bits & 1) ? "f" : "");
  };
  for (uint32_t i = 0; i < snapshot.size; ++i) {
    const GpuProfileEntry& entry = snapshot.entries[i];
    const uint64_t key = entry.key;
    std::string what;
    switch (key >> 60) {
      case 1:
        what = fmt::format("pass c={} d={}", target((key >> 24) & 0xFFFFFF), target(key & 0xFFFFFF));
        break;
      case 2:
        what = fmt::format("resolve{} p{} {}x{}", ((key >> 56) & 1) ? " depth" : "",
                           (key >> 32) & 0x7FF, (key >> 16) & 0xFFFF, key & 0xFFFF);
        break;
      default:
        what = "other";
        break;
    }
    line += fmt::format(" | {}: {:.2f} ({:.1f}x, {:.0f} draws)", what,
                        entry.ns / 1e6 / frames, double(entry.count) / frames,
                        double(entry.draws) / frames);
  }
  REXLOG_INFO("{}", line);
}
#endif

bool Renderer::Impl::CreateSwapResources() {
  // The same layout as VulkanCommandProcessor's apply-gamma compute pipeline:
  // set 0 the ramp (uniform texel buffer), set 1 the source (sampled image),
  // set 2 the destination (storage image), and the size as a push constant.
  VkDescriptorSetLayoutBinding binding = {};
  binding.binding = 0;
  binding.descriptorCount = 1;
  binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &binding;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &swap_layout_ramp) !=
      VK_SUCCESS) {
    return false;
  }
  binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &swap_layout_source) !=
      VK_SUCCESS) {
    return false;
  }
  binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  if (dfn().vkCreateDescriptorSetLayout(vk, &layout_info, nullptr, &swap_layout_dest) !=
      VK_SUCCESS) {
    return false;
  }
  VkDescriptorSetLayout set_layouts[] = {swap_layout_ramp, swap_layout_source, swap_layout_dest};
  VkPushConstantRange push = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 2};
  VkPipelineLayoutCreateInfo pipeline_layout = {};
  pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout.setLayoutCount = 3;
  pipeline_layout.pSetLayouts = set_layouts;
  pipeline_layout.pushConstantRangeCount = 1;
  pipeline_layout.pPushConstantRanges = &push;
  if (dfn().vkCreatePipelineLayout(vk, &pipeline_layout, nullptr, &swap_pipeline_layout) !=
      VK_SUCCESS) {
    return false;
  }
  swap_pipeline = rex::ui::vulkan::util::CreateComputePipeline(
      device, swap_pipeline_layout, shaders::apply_gamma_table_cs,
      sizeof(shaders::apply_gamma_table_cs));
  if (swap_pipeline == VK_NULL_HANDLE) {
    REXLOG_ERROR("[native] Could not create the swap pipeline");
    return false;
  }

  VkDeviceSize ramp_size = 256 * sizeof(uint32_t) * kSubmissionSlots;
  if (!rex::ui::vulkan::util::CreateDedicatedAllocationBuffer(
          device, ramp_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
          rex::ui::vulkan::util::MemoryPurpose::kUpload, ramp_buffer, ramp_memory)) {
    return false;
  }
  void* mapping = nullptr;
  dfn().vkMapMemory(vk, ramp_memory, 0, VK_WHOLE_SIZE, 0, &mapping);
  ramp_mapping = static_cast<uint8_t*>(mapping);

  VkDescriptorPoolSize sizes[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, kSubmissionSlots},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, kSubmissionSlots},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, kSubmissionSlots},
  };
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets = 3 * kSubmissionSlots;
  pool_info.poolSizeCount = 3;
  pool_info.pPoolSizes = sizes;
  if (dfn().vkCreateDescriptorPool(vk, &pool_info, nullptr, &swap_pool) != VK_SUCCESS) {
    return false;
  }
  for (uint32_t i = 0; i < kSubmissionSlots; ++i) {
    VkDescriptorSetAllocateInfo allocate = {};
    allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate.descriptorPool = swap_pool;
    allocate.descriptorSetCount = 3;
    allocate.pSetLayouts = set_layouts;
    if (dfn().vkAllocateDescriptorSets(vk, &allocate, swap_sets[i]) != VK_SUCCESS) {
      return false;
    }
    VkBufferViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    view.buffer = ramp_buffer;
    view.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    view.offset = 256 * sizeof(uint32_t) * i;
    view.range = 256 * sizeof(uint32_t);
    if (dfn().vkCreateBufferView(vk, &view, nullptr, &ramp_views[i]) != VK_SUCCESS) {
      return false;
    }
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = swap_sets[i][0];
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    write.pTexelBufferView = &ramp_views[i];
    dfn().vkUpdateDescriptorSets(vk, 1, &write, 0, nullptr);
  }
  return true;
}

void Renderer::Impl::Shutdown() {
  if (!device) {
    return;
  }
  if(batch_recorder) {
    // NativeGraphicsSystem joins the ring thread before renderer shutdown.
    batch_recorder->AdoptProducer();
    batch_recorder->Stop();batch_recorder.reset();record_threaded=false;
  }
  if (record_thread.joinable()) {
    DrainRecorder();
    record_stop.store(true, std::memory_order_release);
    {
      std::lock_guard<std::mutex> lock(record_mutex);
      record_cv.notify_all();
    }
    record_thread.join();
    record_threaded = false;
  }
  if (driver_cache_writer.joinable()) {
    driver_cache_writer.join();
  }
  if (translation_thread.joinable()) {
    {
      std::lock_guard<std::mutex> lock(translation_mutex);
      translation_stop = true;
    }
    translation_cv.notify_all();
    translation_thread.join();
  }
  if (spirv_cache_file) {
    fclose(spirv_cache_file);
    spirv_cache_file = nullptr;
  }
  if (pipeline_thread.joinable()) {
    {
      std::lock_guard<std::mutex> lock(pipeline_mutex);
      pipeline_stop = true;
    }
    pipeline_cv.notify_all();
    pipeline_thread.join();
  }
  for (VkShaderModule module : prewarm_modules) {
    dfn().vkDestroyShaderModule(vk, module, nullptr);
  }
  prewarm_modules.clear();
  if (pipeline_list_file) {
    fclose(pipeline_list_file);
    pipeline_list_file = nullptr;
  }
  DrainCompletedPipelines();
  Submit();
  for (Submission& s : submissions) {
    WaitSubmission(s);
  }
  if (vkDeviceWaitIdle) vkDeviceWaitIdle(vk);
  textures.Shutdown();
  for (auto& [key, rt] : render_targets) {
    if (rt->sample_view) dfn().vkDestroyImageView(vk, rt->sample_view, nullptr);
    if (rt->view) dfn().vkDestroyImageView(vk, rt->view, nullptr);
    if (rt->image) dfn().vkDestroyImage(vk, rt->image, nullptr);
    if (rt->memory) dfn().vkFreeMemory(vk, rt->memory, nullptr);
  }
  render_targets.clear();
  for (auto& retired : retired_render_targets) {
    if (retired.sample_view) dfn().vkDestroyImageView(vk, retired.sample_view, nullptr);
    dfn().vkDestroyImageView(vk, retired.view, nullptr);
    dfn().vkDestroyImage(vk, retired.image, nullptr);
    dfn().vkFreeMemory(vk, retired.memory, nullptr);
  }
  retired_render_targets.clear();
  if (depth_convert_pipeline) dfn().vkDestroyPipeline(vk, depth_convert_pipeline, nullptr);
  if (depth_convert_layout) dfn().vkDestroyPipelineLayout(vk, depth_convert_layout, nullptr);
  if (depth_copy_pipeline) dfn().vkDestroyPipeline(vk, depth_copy_pipeline, nullptr);
  if (copy_color_pipeline) dfn().vkDestroyPipeline(vk, copy_color_pipeline, nullptr);
  if (copy_color_layout) dfn().vkDestroyPipelineLayout(vk, copy_color_layout, nullptr);
  if (depth_copy_layout) dfn().vkDestroyPipelineLayout(vk, depth_copy_layout, nullptr);
  if (depth_copy_set_layout) {
    dfn().vkDestroyDescriptorSetLayout(vk, depth_copy_set_layout, nullptr);
  }
  if (depth_convert_pool) dfn().vkDestroyDescriptorPool(vk, depth_convert_pool, nullptr);
  if (depth_convert_set_layout) {
    dfn().vkDestroyDescriptorSetLayout(vk, depth_convert_set_layout, nullptr);
  }
#if COD6_DIAGNOSTICS
  if (profile_pool) dfn().vkDestroyQueryPool(vk, profile_pool, nullptr);
#endif
  if (depth_scratch) dfn().vkDestroyBuffer(vk, depth_scratch, nullptr);
  if (depth_scratch_memory) dfn().vkFreeMemory(vk, depth_scratch_memory, nullptr);
  for (auto& [key, pipeline] : pipelines) {
    if (pipeline) dfn().vkDestroyPipeline(vk, pipeline, nullptr);
  }
  pipelines.clear();
  for (auto& [key, module] : geometry_shaders) {
    if (module) dfn().vkDestroyShaderModule(vk, module, nullptr);
  }
  geometry_shaders.clear();
  for (auto& [key, layout] : pipeline_layouts) {
    dfn().vkDestroyPipelineLayout(vk, layout.layout, nullptr);
  }
  pipeline_layouts.clear();
  for (auto& [key, layout] : texture_set_layouts) {
    if (layout != layout_empty) dfn().vkDestroyDescriptorSetLayout(vk, layout, nullptr);
  }
  texture_set_layouts.clear();
  shaders.clear();
  FlushSetCache();
  for (auto& retired : retired_set_pools) {
    for (VkDescriptorPool pool : retired.pools) dfn().vkDestroyDescriptorPool(vk, pool, nullptr);
  }
  retired_set_pools.clear();
  for (Submission& s : submissions) {
    for (VkDescriptorPool pool : s.descriptor_pools) {
      dfn().vkDestroyDescriptorPool(vk, pool, nullptr);
    }
    s.descriptor_pools.clear();
    if (s.fence) dfn().vkDestroyFence(vk, s.fence, nullptr);
    if (s.pool) dfn().vkDestroyCommandPool(vk, s.pool, nullptr);
    if (s.main_pool) dfn().vkDestroyCommandPool(vk, s.main_pool, nullptr);
    s = Submission();
  }
  for (VkBufferView view : ramp_views) {
    if (view) dfn().vkDestroyBufferView(vk, view, nullptr);
  }
  if (swap_pool) dfn().vkDestroyDescriptorPool(vk, swap_pool, nullptr);
  if (swap_pipeline) dfn().vkDestroyPipeline(vk, swap_pipeline, nullptr);
  if (swap_pipeline_layout) dfn().vkDestroyPipelineLayout(vk, swap_pipeline_layout, nullptr);
  for (VkDescriptorSetLayout l : {swap_layout_ramp, swap_layout_source, swap_layout_dest,
                                  layout_empty, layout_shared, layout_constants}) {
    if (l) dfn().vkDestroyDescriptorSetLayout(vk, l, nullptr);
  }
  if (ramp_buffer) dfn().vkDestroyBuffer(vk, ramp_buffer, nullptr);
  if (ramp_memory) dfn().vkFreeMemory(vk, ramp_memory, nullptr);
  if (static_pool) dfn().vkDestroyDescriptorPool(vk, static_pool, nullptr);
#if COD6_INSTANCE_CONSTANTS
  if (instance_buffer) dfn().vkDestroyBuffer(vk, instance_buffer, nullptr);
  if (instance_memory) dfn().vkFreeMemory(vk, instance_memory, nullptr);
#endif
  if (stream_buffer) dfn().vkDestroyBuffer(vk, stream_buffer, nullptr);
  if (stream_memory) dfn().vkFreeMemory(vk, stream_memory, nullptr);
  if (driver_cache) dfn().vkDestroyPipelineCache(vk, driver_cache, nullptr);
  if (timestamps) dfn().vkDestroyQueryPool(vk, timestamps, nullptr);
  device = nullptr;
}

// --- Layouts and pipelines -----------------------------------------------------------

VkDescriptorSetLayout Renderer::Impl::GetTextureSetLayout(bool vertex, uint32_t texture_count,
                                                          uint32_t sampler_count) {
  if (!texture_count && !sampler_count) {
    return layout_empty;
  }
  uint64_t key = (uint64_t(vertex) << 40) | (uint64_t(texture_count) << 20) | sampler_count;
  auto it = texture_set_layouts.find(key);
  if (it != texture_set_layouts.end()) {
    return it->second;
  }
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  VkShaderStageFlags stages = vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
  for (uint32_t i = 0; i < texture_count; ++i) {
    bindings.push_back({i, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, stages, nullptr});
  }
  for (uint32_t i = 0; i < sampler_count; ++i) {
    bindings.push_back({texture_count + i, VK_DESCRIPTOR_TYPE_SAMPLER, 1, stages, nullptr});
  }
  VkDescriptorSetLayoutCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.bindingCount = uint32_t(bindings.size());
  info.pBindings = bindings.data();
  VkDescriptorSetLayout layout = VK_NULL_HANDLE;
  if (dfn().vkCreateDescriptorSetLayout(vk, &info, nullptr, &layout) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }
  texture_set_layouts.emplace(key, layout);
  return layout;
}

const PipelineLayout* Renderer::Impl::GetPipelineLayout(uint32_t tex_vs, uint32_t samp_vs,
                                                        uint32_t tex_ps, uint32_t samp_ps) {
  uint64_t key = (uint64_t(tex_vs) << 48) | (uint64_t(samp_vs) << 32) | (uint64_t(tex_ps) << 16) |
                 samp_ps;
  auto it = pipeline_layouts.find(key);
  if (it != pipeline_layouts.end()) {
    return &it->second;
  }
  PipelineLayout layout;
  layout.textures_vertex = GetTextureSetLayout(true, tex_vs, samp_vs);
  layout.textures_pixel = GetTextureSetLayout(false, tex_ps, samp_ps);
  if (!layout.textures_vertex || !layout.textures_pixel) {
    return nullptr;
  }
  VkDescriptorSetLayout sets[SpirvShaderTranslator::kDescriptorSetCount];
  sets[SpirvShaderTranslator::kDescriptorSetSharedMemoryAndEdram] = layout_shared;
  sets[SpirvShaderTranslator::kDescriptorSetConstants] = layout_constants;
  sets[SpirvShaderTranslator::kDescriptorSetTexturesVertex] = layout.textures_vertex;
  sets[SpirvShaderTranslator::kDescriptorSetTexturesPixel] = layout.textures_pixel;
  VkPipelineLayoutCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.setLayoutCount = SpirvShaderTranslator::kDescriptorSetCount;
  info.pSetLayouts = sets;
  if (dfn().vkCreatePipelineLayout(vk, &info, nullptr, &layout.layout) != VK_SUCCESS) {
    return nullptr;
  }
  return &pipeline_layouts.emplace(key, layout).first->second;
}

// Translation runs on one worker thread with a translator of its own. Being one
// thread matters: the first completed translation of a shader fills the
// shader's binding lists, and every later one leaves them alone, so a
// translation reported done means its shader's bindings are final.
bool Renderer::Impl::EnsureTranslated(VulkanShader::VulkanTranslation* translation,
                                      bool* pending_out) {
  *pending_out = false;
  DrainTranslations();
  auto state = translation_state.find(translation);
  if (state == translation_state.end()) {
    if (!translation->shader().is_ucode_analyzed()) {
      translation->shader().AnalyzeUcode(disasm);
    }
    if (TakeCachedTranslation(translation)) {
      ++stats().shaders_translated;
      ++stats().shaders_from_cache;
      translation_state[translation] = 2;
    } else if (!REXCVAR_GET(cod6_native_async_pipelines) || !translation_thread.joinable()) {
      bool ok = translator->TranslateAnalyzedShader(*translation);
      if (ok) {
        translation->GetOrCreateShaderModule();
        StoreTranslation(translation);
      }
      ++stats().shaders_translated;
      translation_state[translation] = 2;
      if (!ok) {
        REXLOG_ERROR("[native] Shader {:016X} did not translate",
                     translation->shader().ucode_data_hash());
        return false;
      }
    } else {
      translation_state[translation] = 1;
      {
        std::lock_guard<std::mutex> lock(translation_mutex);
        translation_jobs.push_back(translation);
      }
      translation_cv.notify_one();
      *pending_out = true;
      return false;
    }
  } else if (state->second == 1) {
    *pending_out = true;
    return false;
  }
  return translation->is_valid() && translation->shader_module() != VK_NULL_HANDLE;
}

void Renderer::Impl::DrainTranslations() {
  if (!translations_done_count.load(std::memory_order_acquire)) {
    return;
  }
  std::vector<VulkanShader::VulkanTranslation*> done;
  {
    std::lock_guard<std::mutex> lock(translation_mutex);
    done.swap(translations_done);
    translations_done_count.store(0, std::memory_order_release);
  }
  for (auto* t : done) {
    translation_state[t] = 2;
    ++stats().shaders_translated;
  }
}

void Renderer::Impl::TranslationThread() {
#if defined(__SWITCH__)
  svcSetThreadPriority(CUR_THREAD_HANDLE, 0x3D);
#endif
  while (true) {
    VulkanShader::VulkanTranslation* translation;
    {
      std::unique_lock<std::mutex> lock(translation_mutex);
      translation_cv.wait(lock, [&] { return translation_stop || !translation_jobs.empty(); });
      if (translation_stop) return;
      translation = translation_jobs.front();
      translation_jobs.pop_front();
    }
    if (worker_translator->TranslateAnalyzedShader(*translation)) {
      translation->GetOrCreateShaderModule();
      StoreTranslation(translation);
    } else {
      REXLOG_ERROR("[native] Shader {:016X} did not translate",
                   translation->shader().ucode_data_hash());
    }
    std::lock_guard<std::mutex> lock(translation_mutex);
    translations_done.push_back(translation);
    translations_done_count.fetch_add(1, std::memory_order_release);
  }
}

VkPipeline Renderer::Impl::GetPipeline(const PipelineKey& key,
                                       VulkanShader::VulkanTranslation* vs,
                                       VulkanShader::VulkanTranslation* ps,
                                       VkShaderModule geometry, VkPipelineLayout layout,
                                       bool* pending_out) {
  *pending_out = false;
  DrainCompletedPipelines();
  uint64_t hash = XXH3_64bits(&key, sizeof(key));
  auto it = pipelines.find(hash);
  if (it != pipelines.end()) {
    return it->second;
  }
  if (pending_pipelines.count(hash)) {
    *pending_out = true;
    return VK_NULL_HANDLE;
  }
  PipelineJob job;
  job.hash = hash;
  job.key = key;
  job.vertex = vs->shader_module();
  job.pixel = ps ? ps->shader_module() : VK_NULL_HANDLE;
  job.geometry = geometry;
  job.layout = layout;
  if (REXCVAR_GET(cod6_native_async_pipelines) && pipeline_thread.joinable()) {
    // Compiled on the worker. Until it is ready the draws that need it are
    // skipped, a frame or two of missing objects instead of a frozen frame.
    pending_pipelines.insert(hash);
    if (!listed_pipelines.count(hash)) pending_list_keys.emplace(hash, key);
    {
      std::lock_guard<std::mutex> lock(pipeline_mutex);
      pipeline_jobs.push_back(job);
    }
    pipeline_cv.notify_one();
    *pending_out = true;
    return VK_NULL_HANDLE;
  }
  VkPipeline pipeline = CreatePipeline(job);
  if (pipeline) {
    ++stats().pipelines_created;
    last_pipeline_time = std::chrono::steady_clock::now();
    ListPipeline(hash, key);
  }
  pipelines.emplace(hash, pipeline);
  return pipeline;
}

void Renderer::Impl::DrainCompletedPipelines() {
  if (!pipelines_completed_count.load(std::memory_order_acquire)) {
    return;
  }
  std::vector<std::pair<uint64_t, VkPipeline>> done;
  {
    std::lock_guard<std::mutex> lock(pipeline_mutex);
    done.swap(pipelines_completed);
    pipelines_completed_count.store(0, std::memory_order_release);
  }
  for (auto& [hash, pipeline] : done) {
    pending_pipelines.erase(hash);
    pipelines.emplace(hash, pipeline);
    if (pipeline) {
      ++stats().pipelines_created;
      last_pipeline_time = std::chrono::steady_clock::now();
    }
  }
  // The keys of what the draws asked for, to compile at the next boot. Prewarmed ones are already listed.
  if (!pending_list_keys.empty()) {
    for (auto it = pending_list_keys.begin(); it != pending_list_keys.end();) {
      auto made = pipelines.find(it->first);
      if (made == pipelines.end()) {
        ++it;
        continue;
      }
      if (made->second) ListPipeline(it->first, it->second);
      it = pending_list_keys.erase(it);
    }
  }
}

void Renderer::Impl::PipelineThread() {
#if defined(__SWITCH__)
  // Below the game's threads and the ring: compiling is never urgent enough
  // to take a core from them.
  svcSetThreadPriority(CUR_THREAD_HANDLE, 0x3F);
#endif
  while (true) {
    PipelineJob job;
    {
      std::unique_lock<std::mutex> lock(pipeline_mutex);
      pipeline_cv.wait(lock, [&] {
        return pipeline_stop || !pipeline_jobs.empty() || !prewarm_jobs.empty();
      });
      if (pipeline_stop) {
        return;
      }
      // What a draw is waiting for goes first.
      std::deque<PipelineJob>& queue = pipeline_jobs.empty() ? prewarm_jobs : pipeline_jobs;
      job = queue.front();
      queue.pop_front();
    }
    VkPipeline pipeline = CreatePipeline(job);
    std::lock_guard<std::mutex> lock(pipeline_mutex);
    pipelines_completed.emplace_back(job.hash, pipeline);
    pipelines_completed_count.fetch_add(1, std::memory_order_release);
    if (job.prewarm && --prewarm_remaining == 0) {
      for (VkShaderModule module : prewarm_modules) {
        dfn().vkDestroyShaderModule(vk, module, nullptr);
      }
      prewarm_modules.clear();
      REXLOG_INFO("[native] Pipeline prewarm done in {} ms",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - prewarm_start)
                      .count());
    }
  }
}

// Builds the pipeline for a key. Only reads immutable state, so it runs on the
// pipeline worker as well as on the ring thread.
VkPipeline Renderer::Impl::CreatePipeline(const PipelineJob& job) {
  const PipelineKey& key = job.key;
  VkShaderModule geometry = job.geometry;
  VkPipelineLayout layout = job.layout;

  VkSpecializationMapEntry sign_entries[8];
  for (uint32_t i = 0; i < 8; ++i) {
    sign_entries[i] = {i, uint32_t(sizeof(uint32_t) * i), sizeof(uint32_t)};
  }
  VkSpecializationInfo signs = {8, sign_entries, sizeof(key.texture_signs), key.texture_signs};
  const VkSpecializationInfo* specialization = specialize_signs ? &signs : nullptr;

  VkPipelineShaderStageCreateInfo stages[3] = {};
  uint32_t stage_count = 0;
  stages[stage_count++] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                           nullptr,
                           0,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           job.vertex,
                           "main",
                           specialization};
  if (geometry) {
    stages[stage_count++] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                             nullptr,
                             0,
                             VK_SHADER_STAGE_GEOMETRY_BIT,
                             geometry,
                             "main",
                             nullptr};
  }
  if (job.pixel) {
    stages[stage_count++] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                             nullptr,
                             0,
                             VK_SHADER_STAGE_FRAGMENT_BIT,
                             job.pixel,
                             "main",
                             specialization};
  }

  VkPipelineVertexInputStateCreateInfo vertex_input = {};
  vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
#if COD6_INSTANCE_CONSTANTS
  VkVertexInputBindingDescription instance_binding = {};
  VkVertexInputAttributeDescription instance_attributes[5] = {};
  const SpirvShaderTranslator::Modification vertex_modification(key.vs_modification);
  if (vertex_modification.vertex.instance_constants) {
    instance_binding.binding = 0;
    instance_binding.stride = 5 * 4 * sizeof(float);
    instance_binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    for (uint32_t i = 0; i < std::size(instance_attributes); ++i) {
      instance_attributes[i].location = i;
      instance_attributes[i].binding = 0;
      instance_attributes[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      instance_attributes[i].offset = i * 4 * sizeof(float);
    }
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &instance_binding;
    vertex_input.vertexAttributeDescriptionCount = uint32_t(std::size(instance_attributes));
    vertex_input.pVertexAttributeDescriptions = instance_attributes;
  }
#endif
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VkPrimitiveTopology(key.topology);
  input_assembly.primitiveRestartEnable = key.primitive_restart ? VK_TRUE : VK_FALSE;
  VkPipelineViewportStateCreateInfo viewport = {};
  viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport.viewportCount = 1;
  viewport.scissorCount = 1;
  VkPipelineRasterizationStateCreateInfo raster = {};
  raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster.depthClampEnable = key.depth_clamp ? VK_TRUE : VK_FALSE;
  raster.polygonMode = key.polygon_line ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  raster.cullMode = key.cull_mode;
  raster.frontFace =
      key.front_face_clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
  raster.depthBiasEnable = key.depth_format ? VK_TRUE : VK_FALSE;
  raster.lineWidth = 1.0f;
  VkPipelineMultisampleStateCreateInfo multisample = {};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VkSampleCountFlagBits(key.samples);
  VkPipelineDepthStencilStateCreateInfo depth = {};
  depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth.depthTestEnable = key.depth_test ? VK_TRUE : VK_FALSE;
  depth.depthWriteEnable = key.depth_write ? VK_TRUE : VK_FALSE;
  depth.depthCompareOp = VkCompareOp(key.depth_compare);
  depth.stencilTestEnable =
      key.stencil_test && HasStencil(VkFormat(key.depth_format)) ? VK_TRUE : VK_FALSE;
  depth.front.failOp = VkStencilOp(key.stencil_front[0]);
  depth.front.passOp = VkStencilOp(key.stencil_front[1]);
  depth.front.depthFailOp = VkStencilOp(key.stencil_front[2]);
  depth.front.compareOp = VkCompareOp(key.stencil_front[3]);
  depth.back.failOp = VkStencilOp(key.stencil_back[0]);
  depth.back.passOp = VkStencilOp(key.stencil_back[1]);
  depth.back.depthFailOp = VkStencilOp(key.stencil_back[2]);
  depth.back.compareOp = VkCompareOp(key.stencil_back[3]);
  VkPipelineColorBlendAttachmentState attachments[4] = {};
  uint32_t attachment_count = 0;
  VkFormat color_formats[4] = {};
  for (uint32_t i = 0; i < 4; ++i) {
    color_formats[i] = VkFormat(key.color_formats[i]);
    if (key.color_formats[i]) {
      attachment_count = i + 1;
    }
    VkPipelineColorBlendAttachmentState& a = attachments[i];
    uint32_t blend = key.blend[i];
    a.colorWriteMask = (key.write_masks >> (4 * i)) & 0xF;
    if (blend != 0x00010001) {
      reg::RB_BLENDCONTROL control;
      control.value = blend;
      a.blendEnable = VK_TRUE;
      a.srcColorBlendFactor = BlendFactorToVk(control.color_srcblend);
      a.dstColorBlendFactor = BlendFactorToVk(control.color_destblend);
      a.colorBlendOp = BlendOpToVk(control.color_comb_fcn);
      a.srcAlphaBlendFactor = BlendFactorToVk(control.alpha_srcblend);
      a.dstAlphaBlendFactor = BlendFactorToVk(control.alpha_destblend);
      a.alphaBlendOp = BlendOpToVk(control.alpha_comb_fcn);
    }
  }
  VkPipelineColorBlendStateCreateInfo blend = {};
  blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend.attachmentCount = attachment_count;
  blend.pAttachments = attachments;
  VkDynamicState dynamic_states[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,
      VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
  };
  VkPipelineDynamicStateCreateInfo dynamic = {};
  dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic.dynamicStateCount = uint32_t(std::size(dynamic_states));
  dynamic.pDynamicStates = dynamic_states;
  VkPipelineRenderingCreateInfo rendering = {};
  rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering.colorAttachmentCount = attachment_count;
  rendering.pColorAttachmentFormats = color_formats;
  rendering.depthAttachmentFormat = VkFormat(key.depth_format);
  rendering.stencilAttachmentFormat = VkFormat(key.depth_format);

  VkGraphicsPipelineCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info.pNext = &rendering;
  info.stageCount = stage_count;
  info.pStages = stages;
  info.pVertexInputState = &vertex_input;
  info.pInputAssemblyState = &input_assembly;
  info.pViewportState = &viewport;
  info.pRasterizationState = &raster;
  info.pMultisampleState = &multisample;
  info.pDepthStencilState = &depth;
  info.pColorBlendState = &blend;
  info.pDynamicState = &dynamic;
  info.layout = layout;
  VkPipeline pipeline = VK_NULL_HANDLE;
  if (dfn().vkCreateGraphicsPipelines(vk, driver_cache, 1, &info, nullptr, &pipeline) !=
      VK_SUCCESS) {
    REXLOG_ERROR("[native] Pipeline creation failed (VS {:016X}, PS {:016X})", key.vs_hash,
                 key.ps_hash);
    pipeline = VK_NULL_HANDLE;
  }
  return pipeline;
}

// --- Render targets ------------------------------------------------------------------

RenderTarget* Renderer::Impl::GetRenderTarget(const RenderTargetKey& guest_key,
                                               uint32_t min_height,
                                               uint32_t guest_msaa) {
  // cod5-nx: color formats with the same host format share one image, as they share the EDRAM bits on the console.
  // COD5 draws its world into k_8_8_8_8_GAMMA and resolves the same tiles as k_8_8_8_8; with an image per guest
  // format the resolve read an empty image and the world came out black.
  RenderTargetKey key = CanonicalTargetKey(guest_key);
  min_height = std::clamp(min_height, 16u, 2048u);
  RenderTarget* result = GetRenderTargetCanonical(key, min_height);
  if (result) result->guest_msaa_modes.Note(guest_msaa);
  return result;
}

RenderTargetKey Renderer::Impl::CanonicalTargetKey(const RenderTargetKey& guest_key) {
  RenderTargetKey key = guest_key;
  if (!key.is_depth && key.format < 16) {
    static const auto canonical = [] {
      std::array<uint32_t, 16> table{};
      for (uint32_t f = 0; f < 16; ++f) {
        table[f] = f;
        for (uint32_t g = 0; g < f; ++g) {
          if (ColorFormatToVk(xenos::ColorRenderTargetFormat(g)) ==
              ColorFormatToVk(xenos::ColorRenderTargetFormat(f))) {
            table[f] = g;
            break;
          }
        }
      }
      return table;
    }();
    key.format = canonical[key.format];
  }
  return key;
}

// cod5-nx: whether a depth target's first EDRAM tile lies inside another depth target with the same pitch, MSAA and
// format, which on the console would be the same memory.
bool Renderer::Impl::DepthInsideAnother(const RenderTarget& rt) const {
  const RenderTargetKey& k = rt.key;
  const uint32_t pitch_tiles = (k.pitch * (k.msaa >= 2 ? 2 : 1) + 79) / 80;
  const uint32_t rows_per_tile = k.msaa >= 1 ? 8 : 16;
  if (!pitch_tiles) return false;
  for (const auto& [key, other] : render_targets) {
    if (other.get() == &rt || !key.is_depth || key.pitch != k.pitch || key.msaa != k.msaa ||
        key.format != k.format || key.base_tiles >= k.base_tiles) {
      continue;
    }
    const uint32_t end = key.base_tiles + pitch_tiles * ((other->height + rows_per_tile - 1) / rows_per_tile);
    if (k.base_tiles < end) return true;
  }
  return false;
}

RenderTarget* Renderer::Impl::FindClearAlias(const RenderTargetKey& host_key,
                                             uint32_t guest_msaa,
                                             uint32_t& scale_x,
                                             uint32_t& scale_y) {
  // A 4x view at half the pitch covers the same EDRAM tiles as the 2x view (same height) or the 1x view (twice the
  // height) at the full pitch: tiles are 80x16 samples, 4x pixels are 2x2 samples, 2x pixels 1x2.
  if (guest_msaa != 2) return nullptr;
  for (uint32_t alias_guest_msaa : GuestMsaaModes::kFourXFullPitchAliases) {
    RenderTargetKey key = CanonicalTargetKey(host_key);
    key.pitch *= 2;
    key.msaa = HostMsaa(xenos::MsaaSamples(alias_guest_msaa));
    auto it = render_targets.find(key);
    if (it != render_targets.end() && it->second->guest_msaa_modes.Has(alias_guest_msaa)) {
      scale_x = 2;
      scale_y = GuestMsaaModes::AliasScaleY(alias_guest_msaa);
      return it->second.get();
    }
  }
  return nullptr;
}

RenderTarget* Renderer::Impl::GetRenderTargetCanonical(const RenderTargetKey& key, uint32_t min_height) {
  auto it = render_targets.find(key);
  RenderTarget* old = nullptr;
  if (it != render_targets.end()) {
    old = it->second.get();
    old->last_used_frame = frame;
    if (old->height >= min_height) {
      return old;
    }
  }
  auto rt = std::make_unique<RenderTarget>();
  rt->key = key;
  rt->width = std::max(key.pitch, 1u);
  rt->height = rex::align(min_height, 16u);
  rt->samples = VkSampleCountFlagBits(1u << key.msaa);
  rt->format = key.is_depth ? DepthFormatFor(key.format)
                            : ColorFormatToVk(xenos::ColorRenderTargetFormat(key.format));
  rt->last_used_frame = frame;
  const bool fixed_scene_depth =
      key.is_depth && SceneScale::MatchesFixedTarget(
                          key.base_tiles, key.pitch, true, key.msaa);
  // NVK can't allocate a persistent ZCULL plane for a depth image with
  // TRANSFER_DST usage. The fixed scene target is normally created once, so
  // it doesn't need destination copies. If it ever grows, preserve the old
  // contents through the original transfer path and give up ZCULL safely.
  rt->scene_zcull = REXCVAR_GET(cod6_native_scene_zcull) &&
                    fixed_scene_depth && old == nullptr;
  rt->transfer_dst = !rt->scene_zcull;
  if (REXCVAR_GET(cod6_native_scene_zcull) && fixed_scene_depth && old) {
#if COD6_DIAGNOSTICS
    ++stats().scene_zcull_growth_fallbacks;
#endif
    REXLOG_WARN(
        "[scene-zcull] growth fallback: depth:t{}p{} {}x{} -> minimum height {}; "
        "keeping TRANSFER_DST to preserve contents",
        key.base_tiles, key.pitch, old->width, old->height, min_height);
  }
  VkImageCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.format = rt->format;
  info.extent = {rt->width, rt->height, 1};
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.samples = rt->samples;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = (key.is_depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_SAMPLED_BIT
                             : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) |
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if (rt->transfer_dst) {
    info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  if (!rex::ui::vulkan::util::CreateDedicatedAllocationImage(
          device, info, rex::ui::vulkan::util::MemoryPurpose::kDeviceLocal, rt->image,
          rt->memory)) {
    REXLOG_ERROR("[native] Could not create a {}x{} render target", rt->width, rt->height);
    return old;
  }
#if COD6_DIAGNOSTICS
  if (diagnostics) {
    if (!next_render_target_diagnostic_identity) {
      REX_FATAL("[native] Render-target diagnostic identity exhausted");
    }
    rt->diagnostic_identity = next_render_target_diagnostic_identity++;
    rt->diagnostic_content_generation = 1;
  }
#endif
  VkImageAspectFlags aspect =
      key.is_depth ? (HasStencil(rt->format)
                          ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                          : VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT))
                   : VK_IMAGE_ASPECT_COLOR_BIT;
  VkImageViewCreateInfo view = {};
  view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view.image = rt->image;
  view.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view.format = rt->format;
  view.subresourceRange = rex::ui::vulkan::util::InitializeSubresourceRange(aspect);
  dfn().vkCreateImageView(vk, &view, nullptr, &rt->view);
  if (key.is_depth) {
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    dfn().vkCreateImageView(vk, &view, nullptr, &rt->sample_view);
    view.subresourceRange.aspectMask = aspect;
  }

  // Render targets live in GENERAL. A new one starts cleared, or with the
  // content of the smaller one it replaces.
  BeginRecording();
  EndPass();
  VkCommandBuffer c = cmd();
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = rt->scene_zcull
                              ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                              : VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = rt->image;
  barrier.subresourceRange = view.subresourceRange;
  const VkPipelineStageFlags initial_stage =
      rt->scene_zcull ? (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
                      : VK_PIPELINE_STAGE_TRANSFER_BIT;
  dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, initial_stage,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
  if (rt->scene_zcull) {
    VkRect2D full = {{0, 0}, {rt->width, rt->height}};
    if (!ClearDepthTargetInPass(*rt, full, 1.0f, 0)) {
      REX_FATAL("[scene-zcull] Could not clear newly created fixed scene depth target");
    }
#if COD6_DIAGNOSTICS
    ++stats().scene_zcull_targets;
#endif
  } else if (key.is_depth) {
    VkClearDepthStencilValue clear = {1.0f, 0};
    vkCmdClearDepthStencilImage(c, rt->image, VK_IMAGE_LAYOUT_GENERAL, &clear, 1,
                                      &view.subresourceRange);
  } else {
    VkClearColorValue clear = {};
    dfn().vkCmdClearColorImage(c, rt->image, VK_IMAGE_LAYOUT_GENERAL, &clear, 1,
                               &view.subresourceRange);
  }
  if (old) {
    VkImageCopy copy = {};
    copy.srcSubresource = {aspect, 0, 0, 1};
    copy.dstSubresource = copy.srcSubresource;
    copy.extent = {std::min(old->width, rt->width), old->height, 1};
    VkMemoryBarrier memory_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                      VK_ACCESS_TRANSFER_WRITE_BIT |
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                      VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT};
    dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &memory_barrier, 0, nullptr,
                               0, nullptr);
    dfn().vkCmdCopyImage(c, old->image, VK_IMAGE_LAYOUT_GENERAL, rt->image,
                         VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
    // Destroyed once this frame is done on the GPU (waiting for idle here
    // was a visible hitch).
    retired_render_targets.push_back({frame, old->image, old->view, old->sample_view, old->memory});
  }
  VkMemoryBarrier after = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                            rt->scene_zcull
                                ? VkAccessFlags(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
                                : VkAccessFlags(VK_ACCESS_TRANSFER_WRITE_BIT),
                           VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
  dfn().vkCmdPipelineBarrier(cmd(), initial_stage,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                 VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                             0, 1, &after, 0, nullptr, 0, nullptr);
  RenderTarget* result = rt.get();
  render_targets[key] = std::move(rt);
  ++stats().render_targets_created;
  ++rt_generation;
  REXLOG_INFO("[native] Render target {}x{} {} format {} at tile {}, {}x MSAA, "
              "transfer_dst={}, scene_zcull={}",
              result->width, result->height, key.is_depth ? "depth" : "color", key.format,
              key.base_tiles, 1u << key.msaa, result->transfer_dst, result->scene_zcull);
  return result;
}

bool Renderer::Impl::ClearDepthTargetInPass(RenderTarget& target,
                                            const VkRect2D& rect, float depth,
                                            uint32_t stencil) {
  if (!target.key.is_depth || !rect.extent.width || !rect.extent.height ||
      rect.offset.x < 0 || rect.offset.y < 0 ||
      uint64_t(rect.offset.x) + rect.extent.width > target.width ||
      uint64_t(rect.offset.y) + rect.extent.height > target.height) {
    REXLOG_ERROR(
        "[scene-zcull] Invalid attachment clear: depth={} rect={},{} {}x{} target={}x{}",
        target.key.is_depth, rect.offset.x, rect.offset.y, rect.extent.width,
        rect.extent.height, target.width, target.height);
    return false;
  }
  BeginRecording();
  EndPass();
  FlushResolveBarrier();
#if COD6_DIAGNOSTICS
  ProfileMark((kProfilePass << 60) | ProfileTargetBits(&target));
#endif
  VkRenderingAttachmentInfo attachment = {};
  attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  attachment.imageView = target.view;
  attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.clearValue.depthStencil = {depth, stencil};
  VkRenderingInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  info.renderArea = rect;
  info.layerCount = 1;
  info.pDepthAttachment = &attachment;
  info.pStencilAttachment = HasStencil(target.format) ? &attachment : nullptr;
  dfn().vkCmdBeginRendering(cmd(), &info);
  dfn().vkCmdEndRendering(cmd());
  target.last_drawn_frame = frame;
  ++stats().render_passes;
#if COD6_DIAGNOSTICS
  ++stats().scene_zcull_pass_clears;
  MarkRenderTargetWritten(&target);
#endif
  return true;
}

bool Renderer::Impl::BeginPass(const PassTargets& targets) {
  if (in_pass) {
    // Keep the scope if every attachment the draw needs is already bound at
    // the same slot.
    bool compatible = true;
    for (uint32_t i = 0; i < 4; ++i) {
      if (targets.color[i] && targets.color[i] != pass.color[i]) compatible = false;
    }
    if (targets.depth && targets.depth != pass.depth) compatible = false;
    if (compatible) {
      return true;
    }
    EndPass();
  }
  if (trace_frame) {
    TraceEndPass();
    REXLOG_INFO("[trace] pass color {} {} {} {} depth {}", TraceTarget(targets.color[0]),
                TraceTarget(targets.color[1]), TraceTarget(targets.color[2]), TraceTarget(targets.color[3]),
                TraceTarget(targets.depth));
  }
  FlushResolveBarrier();
#if COD6_DIAGNOSTICS
  // Only a newly opened rendering scope changes the candidate context.
  // Compatible draws that remain in the same dynamic-rendering scope retain
  // the serial and may form one adjacent instancing group.
  ++instance_pass_serial;
#endif
  for (RenderTarget* rt : {targets.color[0], targets.color[1], targets.color[2], targets.color[3], targets.depth}) {
    if (rt) rt->last_drawn_frame = frame;
  }
#if COD6_DIAGNOSTICS
  ProfileMark((kProfilePass << 60) | (ProfileTargetBits(targets.color[0]) << 24) |
              ProfileTargetBits(targets.depth));
#endif
  pass = targets;
  VkRenderingAttachmentInfo colors[4] = {};
  uint32_t color_count = 0;
  uint32_t width = UINT32_MAX, height = UINT32_MAX;
  for (uint32_t i = 0; i < 4; ++i) {
    colors[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    if (RenderTarget* rt = targets.color[i]) {
      colors[i].imageView = rt->view;
      colors[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
      colors[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      colors[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color_count = i + 1;
      width = std::min(width, rt->width);
      height = std::min(height, rt->height);
    }
  }
  VkRenderingAttachmentInfo depth = {};
  depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  if (targets.depth) {
    depth.imageView = targets.depth->view;
    depth.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    width = std::min(width, targets.depth->width);
    height = std::min(height, targets.depth->height);
  }
  if (width == UINT32_MAX) {
    return false;
  }
  VkRenderingInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  info.renderArea.extent = {width, height};
  info.layerCount = 1;
  info.colorAttachmentCount = color_count;
  info.pColorAttachments = colors;
  info.pDepthAttachment = targets.depth ? &depth : nullptr;
  info.pStencilAttachment =
      targets.depth && HasStencil(targets.depth->format) ? &depth : nullptr;
  dfn().vkCmdBeginRendering(cmd(), &info);
  in_pass = true;
  pass_extent = {width, height};
  ++stats().render_passes;
  return true;
}

// --- System constants ----------------------------------------------------------------

void Renderer::Impl::UpdateSystemConstants(bool primitive_polygonal,
                                           const draw_util::ViewportInfo& viewport,
                                           uint32_t used_texture_mask, uint32_t,
                                           const VulkanShader* vs, const VulkanShader* ps) {
  // VulkanCommandProcessor::UpdateSystemConstantValues for host render
  // targets, without EDRAM, tessellation or memory export.
  const RegisterFile& r = *regs;
  auto pa_cl_clip_cntl = r.Get<reg::PA_CL_CLIP_CNTL>();
  auto pa_cl_vte_cntl = r.Get<reg::PA_CL_VTE_CNTL>();
  auto rb_colorcontrol = r.Get<reg::RB_COLORCONTROL>();
  auto rb_depth_info = r.Get<reg::RB_DEPTH_INFO>();
  auto rb_surface_info = r.Get<reg::RB_SURFACE_INFO>();
  auto vgt_draw_initiator = r.Get<reg::VGT_DRAW_INITIATOR>();
  SpirvShaderTranslator::SystemConstants& sc = system_constants;

  uint32_t flags = 0;
  if (pa_cl_vte_cntl.vtx_xy_fmt) flags |= SpirvShaderTranslator::kSysFlag_XYDividedByW;
  if (pa_cl_vte_cntl.vtx_z_fmt) flags |= SpirvShaderTranslator::kSysFlag_ZDividedByW;
  if (pa_cl_vte_cntl.vtx_w0_fmt) flags |= SpirvShaderTranslator::kSysFlag_WNotReciprocal;
  if (primitive_polygonal) flags |= SpirvShaderTranslator::kSysFlag_PrimitivePolygonal;
  if (draw_util::IsPrimitiveLine(r)) flags |= SpirvShaderTranslator::kSysFlag_PrimitiveLine;
  flags |= HostMsaa(rb_surface_info.msaa_samples)
           << SpirvShaderTranslator::kSysFlag_MsaaSamples_Shift;
  if (rb_depth_info.depth_format == xenos::DepthRenderTargetFormat::kD24FS8) {
    flags |= SpirvShaderTranslator::kSysFlag_DepthFloat24;
  }
  xenos::CompareFunction alpha_test = rb_colorcontrol.alpha_test_enable
                                          ? rb_colorcontrol.alpha_func
                                          : xenos::CompareFunction::kAlways;
  flags |= uint32_t(alpha_test) << SpirvShaderTranslator::kSysFlag_AlphaPassIfLess_Shift;
  for (uint32_t i = 0; i < 4; ++i) {
    auto color_info = r.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[i]);
    if (color_info.color_format == xenos::ColorRenderTargetFormat::k_8_8_8_8_GAMMA) {
      flags |= SpirvShaderTranslator::kSysFlag_ConvertColor0ToGamma << i;
    }
    // Exponent bias, with 16_16 and 16_16_16_16 stored as -1...1 SNORM
    // instead of -32...32.
    int32_t exp_bias = color_info.color_exp_bias;
    if (color_info.color_format == xenos::ColorRenderTargetFormat::k_16_16 ||
        color_info.color_format == xenos::ColorRenderTargetFormat::k_16_16_16_16) {
      exp_bias -= 5;
    }
    float scale;
    uint32_t scale_bits = UINT32_C(0x3F800000) + (uint32_t(exp_bias) << 23);
    std::memcpy(&scale, &scale_bits, sizeof(scale));
    sc.color_exp_bias[i] = scale;
  }
  sc.flags = flags;
  sc.vertex_index_endian = xenos::Endian::kNone;
  sc.line_loop_closing_index = 0;
  sc.vertex_base_index = r.Get<int32_t>(rex::graphics::XE_GPU_REG_VGT_INDX_OFFSET);
  sc.vertex_index_min = r[rex::graphics::XE_GPU_REG_VGT_MIN_VTX_INDX];
  sc.vertex_index_max = r[rex::graphics::XE_GPU_REG_VGT_MAX_VTX_INDX];
  sc.vertex_index_reset = 0;
  sc.compute_memexport_vertex_count = 0;
  if (!pa_cl_clip_cntl.clip_disable) {
    float* planes = sc.user_clip_planes[0];
    uint32_t remaining = pa_cl_clip_cntl.ucp_ena;
    uint32_t index;
    while (rex::bit_scan_forward(remaining, &index)) {
      remaining &= ~(1u << index);
      std::memcpy(planes, &r[rex::graphics::XE_GPU_REG_PA_CL_UCP_0_X + index * 4],
                  4 * sizeof(float));
      planes += 4;
    }
  }
  for (uint32_t i = 0; i < 3; ++i) {
    sc.ndc_scale[i] = viewport.ndc_scale[i];
    sc.ndc_offset[i] = viewport.ndc_offset[i];
  }
  if (vgt_draw_initiator.prim_type == xenos::PrimitiveType::kPointList) {
    auto minmax = r.Get<reg::PA_SU_POINT_MINMAX>();
    auto size = r.Get<reg::PA_SU_POINT_SIZE>();
    sc.point_vertex_diameter_min = float(minmax.min_size) * (2.0f / 16.0f);
    sc.point_vertex_diameter_max = float(minmax.max_size) * (2.0f / 16.0f);
    sc.point_constant_diameter[0] = float(size.width) * (2.0f / 16.0f);
    sc.point_constant_diameter[1] = float(size.height) * (2.0f / 16.0f);
    sc.point_screen_diameter_to_ndc_radius[0] =
        1.0f / float(std::max(viewport.xy_extent[0], 1u));
    sc.point_screen_diameter_to_ndc_radius[1] =
        1.0f / float(std::max(viewport.xy_extent[1], 1u));
  }
  uint32_t remaining = used_texture_mask;
  uint32_t index;
  while (rex::bit_scan_forward(remaining, &index)) {
    remaining &= ~(1u << index);
    uint32_t& word = sc.texture_swizzled_signs[index >> 2];
    uint32_t shift = 8 * (index & 3);
    uint8_t signs = rex::graphics::texture_util::SwizzleSigns(r.GetTextureFetch(index));
    word = (word & ~(0xFFu << shift)) | (uint32_t(signs) << shift);
  }
  sc.textures_resolution_scaled = 0;
  sc.alpha_test_reference = r.Get<float>(rex::graphics::XE_GPU_REG_RB_ALPHA_REF);
  sc.alpha_to_mask = rb_colorcontrol.alpha_to_mask_enable
                         ? (rb_colorcontrol.value >> 24) | (UINT32_C(1) << 8)
                         : 0;
  (void)vs;
  (void)ps;
}

// --- Draws -------------------------------------------------------------------------------

// The render state of a draw, rebuilt only when a register that can change it
// is written (Renderer::state_generation_), the shaders or the primitive type
// change, or a render target is created. Consecutive draws almost always share
// it, and rebuilding it per draw was most of the renderer's CPU time.
void Renderer::Impl::EmulateMemcpyExport(const reg::VGT_DRAW_INITIATOR& initiator, VulkanShader* vs) {
  TextureTimer memexport_timer(stats().memexport_ns);
  const RegisterFile& r = *regs;
  auto constant_bits = [&](uint32_t n, uint32_t c) { return r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X + n * 4 + c]; };
  auto constant = [&](uint32_t n, uint32_t c) {
    float f;
    const uint32_t bits = constant_bits(n, c);
    std::memcpy(&f, &bits, 4);
    return f;
  };
  static uint32_t logged = 0;
  auto refuse = [&](const char* why) {
    if (logged < 8) {
      ++logged;
      REXLOG_WARN("[native] memexport copy not done: {} (c0 {:08X} {:08X} {:08X} {:08X}, c255 {} {} {} {})", why,
                  constant_bits(0, 0), constant_bits(0, 1), constant_bits(0, 2), constant_bits(0, 3), constant(255, 0),
                  constant(255, 1), constant(255, 2), constant(255, 3));
    }
  };
  if (initiator.prim_type != xenos::PrimitiveType::kPointList ||
      initiator.source_select != xenos::SourceSelect::kAutoIndex) {
    return refuse("not an auto-indexed point list");
  }
  // eA = (base dwords | 0x40000000, index, format info, count) with the last three as floats in [2^23, 2^24).
  const uint32_t ea_x = constant_bits(0, 0), ea_y = constant_bits(0, 1), ea_z = constant_bits(0, 2),
                 ea_w = constant_bits(0, 3);
  if ((ea_x >> 30) != 1 || (ea_y >> 23) != 0x96 || (ea_z >> 23) != 0x96 || (ea_w >> 23) != 0x96) {
    return refuse("unexpected stream constant");
  }
  const float s_x = constant(255, 0), s_y = constant(255, 1), s_z = constant(255, 2), s_w = constant(255, 3);
  if (s_x != 0.0f || s_y != 1.0f || s_z != 4.0f || s_w != 8.0f) return refuse("unexpected c255");
  const uint32_t format_info = ea_z & 0x7FFFFF;
  const uint32_t dest_base = (ea_x << 2) & 0x1FFFFFFF;
  const uint32_t index0 = ea_y & 0x7FFFFF, count_limit = ea_w & 0x7FFFFF;
  if (!vs->is_ucode_analyzed()) {
    rex::string::StringBuffer disasm;
    vs->AnalyzeUcode(disasm);
  }
  if (vs->vertex_bindings().empty()) return refuse("no vertex binding");
  const uint32_t fetch_index = vs->vertex_bindings()[0].fetch_constant;
  xenos::xe_gpu_vertex_fetch_t fetch = r.GetVertexFetch(fetch_index);
  if (fetch.type != xenos::FetchConstantType::kVertex) return refuse("the vertex fetch is not a vertex buffer");
  const uint32_t source = fetch.address << 2;
  const uint32_t vertices = initiator.num_indices;
  const uint64_t bytes = uint64_t(vertices) * 64;
  if (uint64_t(source) + bytes > 0x20000000u || uint64_t(index0 + vertices * 8) > count_limit ||
      uint64_t(dest_base) + uint64_t(index0 + vertices * 8) * 8 > 0x20000000u) {
    return refuse("out of bounds");
  }
  if (logged < 8 && !memexport_copies) {
    ++logged;
    REXLOG_INFO("[native] memexport copy: {} bytes from {:08X} (fetch endian {}) to {:08X} + {} x 8 (format info "
                "{:06X}: endian {}, format {}, swap {})",
                bytes, source, uint32_t(fetch.endian), dest_base, index0, format_info, format_info & 7,
                (format_info >> 8) & 0x3F, (format_info >> 19) & 1);
  }
  // A byte copy: the 16-bit fetches and the 16-bit export use the same layout, so the swizzles and swaps cancel.
  cod6::writes::WriteScope write({dest_base+index0*8,uint32_t(bytes),true});
  std::memcpy(memory->TranslatePhysical<uint8_t*>(dest_base + index0 * 8),
              memory->TranslatePhysical<const uint8_t*>(source), size_t(bytes));
  ++memexport_copies;
  ++stats().memexport_copies;
  memexport_bytes += bytes;
  textures.InvalidateRange(dest_base + index0 * 8, uint32_t(bytes));
}

bool Renderer::Impl::BuildDrawSetup(uint32_t initiator_value) {
  DrawSetup& st = setup;
  st = DrawSetup();
  st.valid = true;
  st.state_generation = owner->state_generation_;
  st.fetch_generation = owner->fetch_generations_.all();
  st.rt_generation = rt_generation;
  st.active_vs = active_vs;
  st.active_ps = active_ps;
  st.initiator_state = initiator_value & 0xFFFF;
  st.skip_reason = UINT32_MAX;

  const RegisterFile& r = *regs;
  const auto& properties = device->properties();
  auto fail = [&](uint32_t reason) {
    st.skip_reason = reason;
    return false;
  };

  xenos::EdramMode edram_mode = r.Get<reg::RB_MODECONTROL>().edram_mode;
  auto surface_info = r.Get<reg::RB_SURFACE_INFO>();
  if (!surface_info.surface_pitch) return fail(0);
  VulkanShader* vs = active_vs;
  if (!vs) return fail(1);
  if (!vs->is_ucode_analyzed()) vs->AnalyzeUcode(disasm);
  if (vs->memexport_eM_written()) {
    /* COD6_MEMEXPORT_CPU
     *
     * Estos dibujos se tiraban sin mirar el cvar: el "cod6_native_memexport_copy"
     * prometia en su descripcion hacer en la CPU el copiado de memoria que hace
     * un vertex shader con memexport, y su texto dice para que sirve:
     *   "its texture streamer copies the low mips into place with it"
     * El NRO de BO2 lo confirma para su propio port:
     *   "BO2 copies streamed texture data (mip levels) this way."
     *
     * Y medido aqui: en 12 texturas DXT, el nivel 0 tiene entropia 6.6-7.6
     * (imagen real) y TODOS los niveles 1 en adelante 0.00 (ceros). Como el
     * juego copia los mips con uno de estos dibujos, al tirarlos los mips nunca
     * se escriben y el muestreo con sesgo de LOD cae en memoria vacia: negro,
     * basura, o la textura de otro objeto.
     *
     * Este shader concreto (4970836A4129BE7D) es un copiado puro, segun su
     * desensamblado: vfetch_full de 16 bytes con stride 16 y escritura por
     * memexport. Asi que el copiado se puede hacer en la CPU: leer el rango de
     * origen del vertex fetch 0 y escribirlo en los rangos que declara el shader.
     *
     * Con red: si el origen o el destino no se pueden traducir, o el tamano no
     * cuadra, no se escribe nada y se deja constancia. El cvar esta apagado por
     * defecto.
     */
    static std::unordered_set<uint64_t> logged;
    const bool querer_copia = REXCVAR_GET(cod6_native_memexport_copy);
    auto init = r.Get<reg::VGT_DRAW_INITIATOR>();
    std::vector<draw_util::MemExportRange> rangos;
    draw_util::AddMemExportRanges(r, *vs, rangos);
    if (querer_copia && !rangos.empty()) {
      const xenos::xe_gpu_vertex_fetch_t origen = r.GetVertexFetch(0);
      const uint32_t origen_dir = origen.address << 2;
      const uint8_t* origen_ptr = memory->TranslatePhysical<const uint8_t*>(origen_dir);
      /* COD6_V314_MEMEXPORT
       *
       * One vertex in, 64 bytes out, straight through: the eight fetch offsets of
       * the disassembly are the 64 contiguous bytes of a vertex with a 64-byte
       * stride, and the two export allocations write exactly 2 x 4 x 8 = 64 bytes.
       *
       * So the ranges are filled IN ORDER from consecutive vertices: the source
       * offset advances by each range's size. The previous version copied from the
       * start of the vertex buffer for every range and never advanced it (so every
       * range got the first vertices again), and never checked how much source
       * there was, which is how it ran off into unmapped memory and wrote over
       * guest memory.
       */
      uint64_t copiados = 0;
      uint32_t fallos = 0;
      uint64_t source_offset = 0;
      const uint64_t source_limit =
          origen_dir < 0x20000000u ? uint64_t(0x20000000u) - origen_dir : 0;
      for (const draw_util::MemExportRange& rango : rangos) {
        const uint32_t destino_dir = rango.base_address_dwords << 2;
        if (!rango.size_bytes) continue;
        if (!origen_ptr || source_offset + rango.size_bytes > source_limit ||
            uint64_t(destino_dir) + rango.size_bytes > 0x20000000u) {
          ++fallos;
          continue;
        }
        uint8_t* destino = memory->TranslatePhysical<uint8_t*>(destino_dir);
        if (!destino) {
          ++fallos;
          continue;
        }
        /* COD6_MEMEXPORT_COMPARE (2026-11-03)
         *
         * These copies run thousands of times per frame, and almost always carry the
         * same bytes as the previous frame. v314 copied unconditionally and invalidated
         * the textures over the destination every time, which forced thousands of
         * re-uploads per frame and dropped the game to 1 fps.
         *
         * Now the bytes are compared first (a few sampled words plus the length); when
         * they match there is nothing to publish and no reason to invalidate.
         */
        const uint8_t* origen_bytes = origen_ptr + source_offset;
        bool distintas = false;
        {
          const uint32_t n = rango.size_bytes;
          if (n < 32) {
            distintas = std::memcmp(destino, origen_bytes, n) != 0;
          } else {
            const uint32_t medio = (n / 2) & ~uint32_t(15);
            distintas = std::memcmp(destino, origen_bytes, 16) != 0 ||
                        std::memcmp(destino + medio, origen_bytes + medio, 16) != 0 ||
                        std::memcmp(destino + n - 16, origen_bytes + n - 16, 16) != 0;
          }
        }
        if (!distintas) {
          source_offset += rango.size_bytes;
          continue;
        }
        std::memcpy(destino, origen_bytes, size_t(rango.size_bytes));
        source_offset += rango.size_bytes;
        copiados += rango.size_bytes;
        // Whatever samples these bytes has to see them, and the write has to be
        // visible to the write-stamp accounting like any other guest write.
        textures.InvalidateRange(destino_dir, rango.size_bytes);
      }
      if (copiados) {
        cod6::writes::WriteScope write({rangos.front().base_address_dwords << 2,
                                        uint32_t(copiados), true});
      }
      if (logged.size() < 12 && logged.insert(vs->ucode_data_hash()).second) {
        REXLOG_INFO("[memexport] {} (prim {}, {} indices): origen {:08X}, {} rangos, "
                    "copiados {} bytes, {} fallos",
                    vs->ucode_data_hash(), uint32_t(init.prim_type), uint32_t(init.num_indices),
                    origen_dir, unsigned(rangos.size()), copiados, fallos);
      }
      return fail(2);
    }
    /* COD6_V313_DISASM
     *
     * The instruction list is needed in full to reproduce this shader's gather on
     * the CPU (the textures it writes are the ones that come out as confetti and
     * diagonal stripes). Logged, it comes out interleaved with other threads and is
     * cut off after a few instructions, so it goes to its own file.
     */
    {
      static std::unordered_set<uint64_t> disassembled;
      const uint64_t shader_hash = vs->ucode_data_hash();
      if (disassembled.insert(shader_hash).second) {
        const std::string dump_dir = REXCVAR_GET(cod6_debug_capture_dir);
        if (!dump_dir.empty()) {
          std::error_code dir_ec;
          std::filesystem::create_directories(dump_dir, dir_ec);
          const std::string dump_path =
              fmt::format("{}/memexport_{:016X}.txt", dump_dir, shader_hash);
          if (std::FILE* dump = std::fopen(dump_path.c_str(), "wb")) {
            const auto text = vs->ucode_disassembly();
            std::fwrite(text.data(), 1, text.size(), dump);
            std::fclose(dump);
            REXLOG_INFO("[memexport] shader {:016X} written to {}", shader_hash, dump_path);
          }
        }
      }
    }
    if (logged.size() < 6 && logged.insert(vs->ucode_data_hash()).second) {
      REXLOG_INFO("[native] memexport vertex shader {:016X} (prim {}, {} indices), skipped; "
                  "{} rangos, copia {} (cod6_native_memexport_copy). Disassembly:\n{}",
                  vs->ucode_data_hash(), uint32_t(init.prim_type), uint32_t(init.num_indices),
                  unsigned(rangos.size()), querer_copia ? "on" : "off", vs->ucode_disassembly());
    }
    return fail(2);
  }
  bool primitive_polygonal = draw_util::IsPrimitivePolygonal(r);
  if (!draw_util::IsRasterizationPotentiallyDone(r, primitive_polygonal)) return fail(3);
  VulkanShader* ps = nullptr;
  if (edram_mode == xenos::EdramMode::kColorDepth && active_ps) {
    ps = active_ps;
    if (!ps->is_ucode_analyzed()) ps->AnalyzeUcode(disasm);
    if (ps->memexport_eM_written()) return fail(2);
    if (!draw_util::IsPixelShaderNeededWithRasterization(*ps, r)) ps = nullptr;
  }
  reg::RB_DEPTHCONTROL depth_control = draw_util::GetNormalizedDepthControl(r);
  uint32_t color_mask = draw_util::GetNormalizedColorMask(r, ps ? ps->writes_color_targets() : 0);

  reg::VGT_DRAW_INITIATOR initiator;
  initiator.value = initiator_value;
  xenos::PrimitiveType guest_type = initiator.prim_type;
  VkPrimitiveTopology topology;
  PipelineGeometryShader geometry = PipelineGeometryShader::kNone;
  switch (guest_type) {
    case xenos::PrimitiveType::kPointList:
      topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      geometry = PipelineGeometryShader::kPointList;
      break;
    case xenos::PrimitiveType::kLineList:
      topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      break;
    case xenos::PrimitiveType::kLineStrip:
      topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      st.strip = true;
      break;
    case xenos::PrimitiveType::kLineLoop:
      topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      st.convert_loop = true;
      break;
    case xenos::PrimitiveType::kTriangleList:
      topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      break;
    case xenos::PrimitiveType::kTriangleStrip:
      topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
      st.strip = true;
      break;
    case xenos::PrimitiveType::kTriangleFan:
    case xenos::PrimitiveType::kPolygon:
      topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      st.convert_fan = true;
      break;
    case xenos::PrimitiveType::kRectangleList:
      topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      geometry = PipelineGeometryShader::kRectangleList;
      break;
    case xenos::PrimitiveType::kQuadList:
      topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
      geometry = PipelineGeometryShader::kQuadList;
      break;
    default:
      return fail(4);
  }
  if (geometry != PipelineGeometryShader::kNone && !properties.geometryShader) return fail(5);
  bool indexed = initiator.source_select != xenos::SourceSelect::kAutoIndex;
  st.reset_enabled =
      indexed && st.strip && r.Get<reg::PA_SU_SC_MODE_CNTL>().multi_prim_ib_ena;

#if COD6_INSTANCE_CONSTANTS
  const Shader::ConstantRegisterMap& setup_vs_constants =
      vs->constant_register_map();
  bool uses_c4_c8 = false;
  for (uint32_t constant = 4; constant <= 8; ++constant) {
    uses_c4_c8 |=
        (setup_vs_constants.float_bitmap[constant / 64] &
         (UINT64_C(1) << (constant % 64))) != 0;
  }
  bool unblended = true;
  for (uint32_t target = 0; target < 4; ++target) {
    if (((color_mask >> (target * 4)) & 0xF) &&
        (r[reg::RB_BLENDCONTROL::rt_register_indices[target]] & 0x1FFF1FFF) !=
            0x00010001) {
      unblended = false;
    }
  }
  // Keep the first behavioral version to opaque indexed triangles that write
  // depth. This excludes UI, particles, blended surfaces and post-processing;
  // equality checks later may narrow it further but never widen it.
  st.instance_constants =
      REXCVAR_GET(cod6_native_instance_batching) && uses_c4_c8 &&
      !setup_vs_constants.float_dynamic_addressing && indexed && unblended &&
      depth_control.z_enable && depth_control.z_write_enable &&
      (guest_type == xenos::PrimitiveType::kTriangleList ||
       guest_type == xenos::PrimitiveType::kTriangleStrip);
#endif

  // Shader modifications (VulkanPipelineCache::GetCurrent*ShaderModification).
  uint32_t ps_param_gen_pos = UINT32_MAX;
  uint32_t interpolator_mask =
      ps ? (vs->writes_interpolators() &
            ps->GetInterpolatorInputMask(r.Get<reg::SQ_PROGRAM_CNTL>(),
                                         r.Get<reg::SQ_CONTEXT_MISC>(), ps_param_gen_pos))
         : 0;
  SpirvShaderTranslator::Modification vs_mod(translator->GetDefaultVertexShaderModification(
      vs->GetDynamicAddressableRegisterCount(r.Get<reg::SQ_PROGRAM_CNTL>().vs_num_reg),
      Shader::HostVertexShaderType::kVertex));
  vs_mod.vertex.interpolator_mask = interpolator_mask;
  auto pa_cl_clip_cntl = r.Get<reg::PA_CL_CLIP_CNTL>();
  uint32_t user_clip_planes = pa_cl_clip_cntl.clip_disable ? 0 : pa_cl_clip_cntl.ucp_ena;
  if (user_clip_planes) {
    if (pa_cl_clip_cntl.ucp_cull_only_ena && properties.shaderCullDistance) {
      vs_mod.vertex.user_clip_plane_cull = 1;
    } else if (!properties.shaderClipDistance && properties.shaderCullDistance) {
      vs_mod.vertex.user_clip_plane_cull = 1;
    } else if (!properties.shaderClipDistance) {
      user_clip_planes = 0;
    }
  }
  vs_mod.vertex.user_clip_plane_count = rex::bit_count(user_clip_planes);
  vs_mod.vertex.point_ps_ucp_mode = pa_cl_clip_cntl.ps_ucp_mode;
  vs_mod.vertex.output_point_parameters =
      uint32_t((vs->writes_point_size_edge_flag_kill_vertex() & 0b001) &&
               guest_type == xenos::PrimitiveType::kPointList);
  vs_mod.vertex.vertex_kill_and =
      uint32_t(properties.shaderCullDistance &&
               (vs->writes_point_size_edge_flag_kill_vertex() & 0b100) &&
               !pa_cl_clip_cntl.vtx_kill_or);
#if COD6_INSTANCE_CONSTANTS
  vs_mod.vertex.instance_constants = st.instance_constants ? 1 : 0;
#endif
  SpirvShaderTranslator::Modification ps_mod(0);
  if (ps) {
    ps_mod = SpirvShaderTranslator::Modification(translator->GetDefaultPixelShaderModification(
        ps->GetDynamicAddressableRegisterCount(r.Get<reg::SQ_PROGRAM_CNTL>().ps_num_reg)));
    ps_mod.pixel.interpolator_mask = interpolator_mask;
    ps_mod.pixel.interpolators_centroid =
        interpolator_mask &
        ~xenos::GetInterpolatorSamplingPattern(surface_info.msaa_samples,
                                               r.Get<reg::SQ_CONTEXT_MISC>().sc_sample_cntl,
                                               r.Get<reg::SQ_INTERPOLATOR_CNTL>().sampling_pattern);
    if (ps_param_gen_pos < xenos::kMaxInterpolators) {
      ps_mod.pixel.param_gen_enable = 1;
      ps_mod.pixel.param_gen_interpolator = ps_param_gen_pos;
      ps_mod.pixel.param_gen_point = uint32_t(guest_type == xenos::PrimitiveType::kPointList);
    }
    using DepthStencilMode = SpirvShaderTranslator::Modification::DepthStencilMode;
    if (ps->implicit_early_z_write_allowed() &&
        (!ps->writes_color_target(0) ||
         !draw_util::DoesCoverageDependOnAlpha(r.Get<reg::RB_COLORCONTROL>()))) {
      ps_mod.pixel.depth_stencil_mode = DepthStencilMode::kEarlyHint;
    } else {
      ps_mod.pixel.depth_stencil_mode = DepthStencilMode::kNoModifiers;
    }
  }
  auto* vs_translation =
      static_cast<VulkanShader::VulkanTranslation*>(vs->GetOrCreateTranslation(vs_mod.value));
  auto* ps_translation =
      ps ? static_cast<VulkanShader::VulkanTranslation*>(ps->GetOrCreateTranslation(ps_mod.value))
         : nullptr;
  /* COD6_V248_SALTAR_PS: un pixel shader de la lista no se dibuja. Se compara
   * por hash de microcodigo, que es estable y sale en el trace del fotograma. */
  if (ps) {
    static std::string lista_cache;
    static std::vector<uint64_t> hashes;
    const std::string& lista = REXCVAR_GET(cod6_native_skip_ps);
    if (lista != lista_cache) {
      lista_cache = lista;
      hashes.clear();
      size_t pos = 0;
      while (pos < lista.size()) {
        while (pos < lista.size() && (lista[pos] == ' ' || lista[pos] == ',')) ++pos;
        size_t ini = pos;
        while (pos < lista.size() && lista[pos] != ' ' && lista[pos] != ',') ++pos;
        if (ini == pos) break;
        hashes.push_back(uint64_t(std::strtoull(lista.substr(ini, pos - ini).c_str(), nullptr, 16)));
      }
    }
    {
      /* COD6_PSHIST: apuntar uso por shader. */
      const uint64_t h = ps->ucode_data_hash();
      /* 0x00010001 = mezcla desactivada, igual que en la clave de pipeline. */
      const bool mezcla = (r[reg::RB_BLENDCONTROL::rt_register_indices[0]] != 0x00010001u);
      RexPSAnotar(h, mezcla);
    }
    if (!hashes.empty()) {
      const uint64_t ps_hash = ps->ucode_data_hash();
      if (std::find(hashes.begin(), hashes.end(), ps_hash) != hashes.end()) return fail(17);
    }
  }
  {
    bool vs_pending = false, ps_pending = false;
    bool vs_ok = EnsureTranslated(vs_translation, &vs_pending);
    bool ps_ok = !ps_translation || EnsureTranslated(ps_translation, &ps_pending);
    if (!vs_ok || !ps_ok) {
      // Still being translated: look again on the next draw.
      if (vs_pending || ps_pending) st.valid = false;
      return fail(6);
    }
  }

  // Render targets: the color targets the draw writes and the depth buffer
  // if it is tested or written.
  bool depth_used = depth_control.z_enable || depth_control.stencil_enable;
  // The guest's sample count sizes the EDRAM below; the host's keys the targets.
  uint32_t guest_msaa = uint32_t(surface_info.msaa_samples);
  uint32_t msaa = HostMsaa(surface_info.msaa_samples);
  draw_util::GetScissor(r, st.scissor);
  draw_util::GetHostViewportInfo(r, 1, 1, false, properties.maxViewportDimensions[0],
                                 properties.maxViewportDimensions[1], true, depth_control, false,
                                 true, ps && ps->writes_depth(), st.viewport);
  // Rows the draw can touch: the viewport, cut by the scissor. The scissor
  // alone is often the 8192-pixel default and made every target 2048 tall.
  uint32_t needed_height = st.scissor.offset[1] + st.scissor.extent[1];
  if (st.viewport.xy_extent[1]) {
    needed_height =
        std::min(needed_height, st.viewport.xy_offset[1] + st.viewport.xy_extent[1]);
  }
  PassTargets& targets = st.targets;
  // cod5-nx: the Xbox 360 Direct3D clears a 2x (or 1x) surface by drawing one rectangle into a 4x view of it at half
  // the pitch, which covers the same EDRAM tiles. With an image per view the clear was lost (COD5's scene and shadow
  // depth stayed uncleared and the world drew black). Such a rectangle goes to the existing full-pitch target, with
  // its viewport and scissor scaled.
  bool clear_redirected = false;
  if (initiator.prim_type == xenos::PrimitiveType::kRectangleList && guest_msaa == 2) {
    uint32_t sx = 0, sy = 0;
    bool all_found = true, any = false;
    PassTargets redirected;
    for (uint32_t i = 0; i < 4 && all_found; ++i) {
      if (!((color_mask >> (4 * i)) & 0xF)) continue;
      auto color_info = r.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[i]);
      RenderTargetKey key = {color_info.color_base, surface_info.surface_pitch,
                             uint32_t(color_info.color_format), msaa, 0};
      uint32_t x = 0, y = 0;
      redirected.color[i] = FindClearAlias(key, guest_msaa, x, y);
      if (!redirected.color[i] || (any && (x != sx || y != sy))) all_found = false;
      sx = x;
      sy = y;
      any = true;
    }
    if (all_found && depth_used) {
      auto depth_info = r.Get<reg::RB_DEPTH_INFO>();
      RenderTargetKey key = {depth_info.depth_base, surface_info.surface_pitch,
                             uint32_t(depth_info.depth_format), msaa, 1};
      uint32_t x = 0, y = 0;
      redirected.depth = FindClearAlias(key, guest_msaa, x, y);
      if (!redirected.depth || (any && (x != sx || y != sy))) all_found = false;
      sx = x;
      sy = y;
      any = true;
    }
    if (all_found && any) {
      targets = redirected;
      clear_redirected = true;
      for (uint32_t a = 0; a < 2; ++a) {
        const uint32_t scale = a ? sy : sx;
        st.viewport.xy_offset[a] *= scale;
        st.viewport.xy_extent[a] *= scale;
        st.scissor.offset[a] *= scale;
        st.scissor.extent[a] *= scale;
      }
      ++clear_redirects;
      if (trace_frame) REXLOG_INFO("[trace]     clear redirected to the full-pitch target ({}x{} scale)", sx, sy);
    }
  }
  for (uint32_t i = 0; i < 4 && !clear_redirected; ++i) {
    if (!((color_mask >> (4 * i)) & 0xF)) continue;
    auto color_info = r.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[i]);
    RenderTargetKey key = {};
    key.base_tiles = color_info.color_base;
    key.pitch = surface_info.surface_pitch;
    key.format = uint32_t(color_info.color_format);
    key.msaa = msaa;
    // The EDRAM below the base limits how tall the target can be.
    uint32_t pitch_tiles = (surface_info.surface_pitch * (guest_msaa >= 2 ? 2 : 1) + 79) / 80 *
                           (IsColorFormat64bpp(color_info.color_format) ? 2 : 1);
    uint32_t rows = pitch_tiles ? (2048 - std::min(key.base_tiles, 2047u)) / pitch_tiles *
                                      (guest_msaa >= 1 ? 8 : 16)
                                : 2048;
    targets.color[i] = GetRenderTarget(key, std::min(needed_height, rows), guest_msaa);
  }
  if (depth_used && !clear_redirected) {
    auto depth_info = r.Get<reg::RB_DEPTH_INFO>();
    RenderTargetKey key = {};
    key.base_tiles = depth_info.depth_base;
    key.pitch = surface_info.surface_pitch;
    key.format = uint32_t(depth_info.depth_format);
    key.msaa = msaa;
    key.is_depth = 1;
    uint32_t pitch_tiles = (surface_info.surface_pitch * (guest_msaa >= 2 ? 2 : 1) + 79) / 80;
    uint32_t rows = pitch_tiles ? (2048 - std::min(key.base_tiles, 2047u)) / pitch_tiles *
                                      (guest_msaa >= 1 ? 8 : 16)
                                : 2048;
    targets.depth = GetRenderTarget(key, std::min(needed_height, rows), guest_msaa);
  }
  // Creating a render target above bumped rt_generation; this setup is built
  // against the new ones.
  st.rt_generation = rt_generation;
  if (targets.Empty()) return fail(7);
  /* COD6_V241_MEZCLA: si algun destino de color tiene mezcla activa, el dibujo
   * es un efecto (o la interfaz). Se descarta antes de rasterizar. */
  if (REXCVAR_GET(cod6_native_skip_blended_draws)) {
    for (uint32_t v241 = 0; v241 < 4; ++v241) {
      if (targets.color[v241] &&
          (r[reg::RB_BLENDCONTROL::rt_register_indices[v241]] & 0x1FFF1FFF) != 0x00010001) {
        return fail(16);
      }
    }
  }

  /* COD6_V240_SOMBRAS: marcar la profundidad que va acompanada de color. Esa es
   * la de la escena y no se toca nunca. */
  const bool v240_depth_only = targets.depth && !targets.color[0] && !targets.color[1] &&
                               !targets.color[2] && !targets.color[3];
  if (targets.depth && !v240_depth_only) {
    targets.depth->used_with_color = true;
  }
  /* Descartar SOLO los mapas de sombra: destino de solo profundidad, que nunca
   * se ha usado con color, y alto (los del registro son de 2048; la escena es
   * de 608 o 720). Los dos candados juntos evitan dejar el juego sin
   * profundidad, que fue el negro de la v238. */
  if (v240_depth_only && REXCVAR_GET(cod6_native_skip_shadow_passes) &&
      !targets.depth->used_with_color && targets.depth->height > 1024) {
    return fail(15);
  }
  if (targets.depth && !targets.color[0] && !targets.color[1] && !targets.color[2] && !targets.color[3] &&
      !targets.depth->ever_resolved && REXCVAR_GET(cod6_native_skip_aliased_depth) &&
      DepthInsideAnother(*targets.depth)) {
    return fail(14);
  }

  // Keep guest coordinates and EDRAM identity unchanged, but rasterize the
  // fixed playable-scene targets into a smaller top-left rectangle. Resolves
  // below expand that rectangle back to the guest-requested dimensions, so
  // texture consumers and presentation retain their original coordinate
  // space. Exact target matching keeps shadow maps and the 960x544 UI/video
  // path out of this candidate.
  st.scene_scaled = IsScenePass(targets);
  if (st.scene_scaled) {
    for (uint32_t axis = 0; axis < 2; ++axis) {
      scene_scale.ScaleInterval(st.viewport.xy_offset[axis],
                                st.viewport.xy_extent[axis]);
      scene_scale.ScaleInterval(st.scissor.offset[axis],
                                st.scissor.extent[axis]);
    }
  }

  // Layout.
  st.vs = vs;
  st.ps = ps;
  const VulkanShader* stage_shaders[2] = {vs, ps};
  for (uint32_t stage = 0; stage < 2; ++stage) {
    const VulkanShader* shader = stage_shaders[stage];
    if (!shader) continue;
    st.counts[stage * 2] = uint32_t(shader->GetTextureBindingsAfterTranslation().size());
    st.counts[stage * 2 + 1] = uint32_t(shader->GetSamplerBindingsAfterTranslation().size());
    if (binding_key_memo) {
      for (const auto& binding : shader->GetTextureBindingsAfterTranslation()) {
        st.texture_slot_mask |= 1u << binding.fetch_constant;
      }
      for (const auto& binding : shader->GetSamplerBindingsAfterTranslation()) {
        st.texture_slot_mask |= 1u << binding.fetch_constant;
      }
    }
  }
  if constexpr (cod6::diagnostics::kEnabled) {
    if (binding_key_memo) ++stats().binding_key_slot_mask_scans;
  }
  if (binding_key_memo && REXCVAR_GET(cod6_native_bind_check)) {
    uint32_t shader_mask = 0;
    for (uint32_t stage = 0; stage < 2; ++stage) {
      const VulkanShader* shader = stage_shaders[stage];
      if (!shader) continue;
      for (const auto& b : shader->GetTextureBindingsAfterTranslation()) {
        shader_mask |= 1u << b.fetch_constant;
      }
      for (const auto& b : shader->GetSamplerBindingsAfterTranslation()) {
        shader_mask |= 1u << b.fetch_constant;
      }
    }
    ++stats().binding_key_mask_checked;
    const uint32_t uncovered = shader_mask & ~st.texture_slot_mask;
    if (uncovered) {
      ++stats().binding_key_mask_missing;
      static uint32_t bind_missing_logged = 0;
      if (bind_missing_logged < 8) {
        ++bind_missing_logged;
        REXLOG_ERROR("[bindcheck] slot(s) {:08X} sampled by shader {} / {} but absent from the "
                     "binding key mask {:08X}",
                     uncovered, vs ? vs->ucode_data_hash() : 0, ps ? ps->ucode_data_hash() : 0,
                     st.texture_slot_mask);
      }
    }
  }
  st.layout = GetPipelineLayout(st.counts[0], st.counts[1], st.counts[2], st.counts[3]);
  if (!st.layout) return fail(10);

  // Pipeline.
  PipelineKey key = {};
  key.vs_hash = vs->ucode_data_hash();
  key.vs_modification = vs_mod.value;
  key.ps_hash = ps ? ps->ucode_data_hash() : 0;
  key.ps_modification = ps ? ps_mod.value : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key.color_formats[i] = targets.color[i] ? uint32_t(targets.color[i]->format) : 0;
  }
  key.depth_format = targets.depth ? uint32_t(targets.depth->format) : 0;
  key.samples = 1u << msaa;
  key.topology = uint32_t(topology);
  key.primitive_restart = st.reset_enabled ? 1 : 0;
  auto sc_mode = r.Get<reg::PA_SU_SC_MODE_CNTL>();
  if (primitive_polygonal) {
    key.cull_mode = (sc_mode.cull_front ? VK_CULL_MODE_FRONT_BIT : 0) |
                    (sc_mode.cull_back ? VK_CULL_MODE_BACK_BIT : 0);
    key.front_face_clockwise = sc_mode.face != 0;
    if (properties.fillModeNonSolid &&
        sc_mode.poly_mode == xenos::PolygonModeEnable::kDualMode) {
      xenos::PolygonType type = xenos::PolygonType::kTriangles;
      if (!sc_mode.cull_front) type = std::min(type, sc_mode.polymode_front_ptype);
      if (!sc_mode.cull_back) type = std::min(type, sc_mode.polymode_back_ptype);
      key.polygon_line = type != xenos::PolygonType::kTriangles;
    }
  }
  key.depth_clamp = properties.depthClamp && pa_cl_clip_cntl.clip_disable;
  if (targets.depth) {
    if (depth_control.z_enable) {
      key.depth_test = 1;
      key.depth_write = depth_control.z_write_enable;
      key.depth_compare = uint32_t(VK_COMPARE_OP_NEVER) + uint32_t(depth_control.zfunc);
    }
    if (depth_control.stencil_enable) {
      key.stencil_test = 1;
      key.stencil_front[0] = uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilfail);
      key.stencil_front[1] = uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilzpass);
      key.stencil_front[2] = uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilzfail);
      key.stencil_front[3] = uint32_t(VK_COMPARE_OP_NEVER) + uint32_t(depth_control.stencilfunc);
      if (primitive_polygonal && depth_control.backface_enable) {
        key.stencil_back[0] =
            uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilfail_bf);
        key.stencil_back[1] =
            uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilzpass_bf);
        key.stencil_back[2] =
            uint32_t(VK_STENCIL_OP_KEEP) + uint32_t(depth_control.stencilzfail_bf);
        key.stencil_back[3] =
            uint32_t(VK_COMPARE_OP_NEVER) + uint32_t(depth_control.stencilfunc_bf);
      } else {
        std::memcpy(key.stencil_back, key.stencil_front, sizeof(key.stencil_back));
      }
    }
  }
  for (uint32_t i = 0; i < 4; ++i) {
    uint32_t mask = (color_mask >> (4 * i)) & 0xF;
    if (!targets.color[i]) mask = 0;
    key.write_masks |= mask << (4 * i);
    uint32_t blend = r[reg::RB_BLENDCONTROL::rt_register_indices[i]] & 0x1FFF1FFF;
    key.blend[i] = mask ? blend : 0x00010001;
  }
  if (specialize_signs) {
    for (const VulkanShader* shader : stage_shaders) {
      if (!shader) continue;
      for (const auto& binding : shader->texture_bindings()) {
        uint32_t fetch = binding.fetch_constant;
        uint8_t signs = rex::graphics::texture_util::SwizzleSigns(r.GetTextureFetch(fetch));
        uint32_t& word = key.texture_signs[fetch >> 2];
        uint32_t shift = 8 * (fetch & 3);
        word = (word & ~(0xFFu << shift)) | (uint32_t(signs) << shift);
      }
    }
  }
  key.layout = (uint64_t(st.counts[0]) << 48) | (uint64_t(st.counts[1]) << 32) |
               (uint64_t(st.counts[2]) << 16) | st.counts[3];
  VkShaderModule geometry_module = VK_NULL_HANDLE;
  if (geometry != PipelineGeometryShader::kNone) {
    GeometryShaderKey gs_key;
    gs_key.type = geometry;
    gs_key.interpolator_count = rex::bit_count(vs_mod.vertex.interpolator_mask);
    gs_key.user_clip_plane_count = vs_mod.vertex.user_clip_plane_count;
    gs_key.user_clip_plane_cull = vs_mod.vertex.user_clip_plane_cull;
    gs_key.has_vertex_kill_and = vs_mod.vertex.vertex_kill_and;
    gs_key.has_point_size = vs_mod.vertex.output_point_parameters;
    gs_key.has_point_coordinates = ps ? ps_mod.pixel.param_gen_point : 0;
    gs_key.point_ps_ucp_mode = vs_mod.vertex.point_ps_ucp_mode;
    key.geometry_shader = gs_key.key;
    auto it = geometry_shaders.find(gs_key.key);
    if (it == geometry_shaders.end()) {
      it = geometry_shaders.emplace(gs_key.key, CreateGeometryShader(device, gs_key)).first;
    }
    geometry_module = it->second;
    if (!geometry_module) return fail(12);
  }
  bool pending = false;
  st.pipeline = GetPipeline(key, vs_translation, ps_translation, geometry_module,
                            st.layout->layout, &pending);
  if (!st.pipeline) {
    // A pipeline being compiled: look again on the next draw.
    if (pending) st.valid = false;
    return fail(13);
  }
  if (bound_pipeline != st.pipeline) {
    PreloadNvkGraphicsPipeline(st.pipeline);
  }

  // Dynamic state (VulkanCommandProcessor::UpdateDynamicState): all of it
  // comes from state registers.
  st.primitive_polygonal = primitive_polygonal;
  st.depth_control = depth_control;
  if (st.viewport.xy_extent[0] && st.viewport.xy_extent[1]) {
    st.vp.x = float(st.viewport.xy_offset[0]);
    st.vp.y = float(st.viewport.xy_offset[1]);
    st.vp.width = float(st.viewport.xy_extent[0]);
    st.vp.height = float(st.viewport.xy_extent[1]);
  } else {
    st.vp.x = -1.0f;
    st.vp.y = -1.0f;
    st.vp.width = 1.0f;
    st.vp.height = 1.0f;
  }
  st.vp.minDepth = st.viewport.z_min;
  st.vp.maxDepth = st.viewport.z_max;
  draw_util::GetPreferredFacePolygonOffset(r, primitive_polygonal, st.bias_slope,
                                           st.bias_constant);
  st.bias_constant *= r.Get<reg::RB_DEPTH_INFO>().depth_format ==
                              xenos::DepthRenderTargetFormat::kD24S8
                          ? draw_util::kD3D10PolygonOffsetFactorUnorm24
                          : draw_util::kD3D10PolygonOffsetFactorFloat24;
  st.bias_slope *= xenos::kPolygonOffsetScaleSubpixelUnit;
  st.blend_constants[0] = r.Get<float>(rex::graphics::XE_GPU_REG_RB_BLEND_RED);
  st.blend_constants[1] = r.Get<float>(rex::graphics::XE_GPU_REG_RB_BLEND_GREEN);
  st.blend_constants[2] = r.Get<float>(rex::graphics::XE_GPU_REG_RB_BLEND_BLUE);
  st.blend_constants[3] = r.Get<float>(rex::graphics::XE_GPU_REG_RB_BLEND_ALPHA);
  auto front = r.Get<reg::RB_STENCILREFMASK>();
  auto back = front;
  if (primitive_polygonal && depth_control.backface_enable) {
    back = r.Get<reg::RB_STENCILREFMASK>(rex::graphics::XE_GPU_REG_RB_STENCILREFMASK_BF);
  }
  uint32_t stencil[6] = {front.stencilmask,      back.stencilmask, front.stencilwritemask,
                         back.stencilwritemask, front.stencilref, back.stencilref};
  std::memcpy(st.stencil, stencil, sizeof(stencil));
  return true;
}

void Renderer::Impl::Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words) {
  cod6::writes::InvalidateBindingsOnWrites(observed_write_serial,vertex_memory_epoch,fetch_resolved_frame);
#if COD6_DIAGNOSTICS
  struct PreparationScope {
    bool& active;
    bool previous;
    PreparationScope(bool& flag, bool enabled) : active(flag), previous(flag) { active = enabled; }
    ~PreparationScope() { active = previous; }
  } preparation(preparing_draw, diagnostics);
#endif
  const RegisterFile& r = *regs;
  // The reasons, in order: 0 no surface pitch, 1 no vertex shader, 2 memory
  // export, 3 no rasterization, 4 primitive type, 5 no geometry shaders,
  // 6 translation, 7 no render target, 8 index data, 9 stream full,
  // 10 layout, 11 descriptors, 12 geometry shader, 13 pipeline, 14 pass,
  // 15 immediate indices.
  auto skip = [&](uint32_t reason) {
    ++stats().draws_skipped;
    ++stats().skipped_by_reason[reason & 15];
#if COD6_DIAGNOSTICS
    if (reason == 7) NoteShadow(1, r.Get<reg::VGT_DRAW_INITIATOR>().value);
#endif
    if (trace_frame) {
      ++trace_pass_skips[reason & 15];
      if (trace_skip_logged < 300) {
        ++trace_skip_logged;
        auto dc = r.Get<reg::RB_DEPTHCONTROL>();
        auto init = r.Get<reg::VGT_DRAW_INITIATOR>();
        REXLOG_INFO("[trace]     skip {}: prim {} count {} z {} zw {} zf {} st {} colormask {:04X} modecontrol {}",
                    reason, uint32_t(init.prim_type), uint32_t(init.num_indices), uint32_t(dc.z_enable),
                    uint32_t(dc.z_write_enable), uint32_t(dc.zfunc), uint32_t(dc.stencil_enable),
                    r[rex::graphics::XE_GPU_REG_RB_COLOR_MASK] & 0xFFFF,
                    uint32_t(r.Get<reg::RB_MODECONTROL>().edram_mode));
      }
    }
  };

  auto initiator = r.Get<reg::VGT_DRAW_INITIATOR>();
  uint32_t guest_count = initiator.num_indices;
  if (!guest_count) {
    return;
  }
  // cod5-nx: every draw of the GPU memcpy shader is done on the CPU (before the setup cache, which would replay
  // the cached skip without copying).
  if (active_vs && active_vs->ucode_data_hash() == kCod5MemcpyVs && REXCVAR_GET(cod6_native_memexport_copy)) {
    EmulateMemcpyExport(initiator, active_vs);
    return skip(2);
  }
  // The render state: the same as the previous draw (nothing changed), or
  // one seen before (looked up by the hash of the state registers, the
  // shaders and the primitive setup), or built now.
  if (!(setup.valid && setup.state_generation == owner->state_generation_ &&
        setup.active_vs == active_vs && setup.active_ps == active_ps &&
        setup.initiator_state == (initiator.value & 0xFFFF) &&
        setup.rt_generation == rt_generation &&
        (!specialize_signs || setup.fetch_generation == owner->fetch_generations_.all()))) {
    if (setup_cache_rt_generation != rt_generation) {
      // Setups hold render target pointers.
      ClearMapMeasured(setup_cache, 0, setup_cache.bucket_count());
      setup_cache_rt_generation = rt_generation;
    }
    uint64_t key = owner->state_hash_ ^
                   Renderer::Mix(0x10000, uint32_t(reinterpret_cast<uintptr_t>(active_vs))) ^
                   Renderer::Mix(0x10001, uint32_t(reinterpret_cast<uintptr_t>(active_vs) >> 32)) ^
                   Renderer::Mix(0x10002, uint32_t(reinterpret_cast<uintptr_t>(active_ps))) ^
                   Renderer::Mix(0x10003, uint32_t(reinterpret_cast<uintptr_t>(active_ps) >> 32)) ^
                   Renderer::Mix(0x10004, (initiator.value & 0xFFFF) | 0x80000000u);
    if (specialize_signs) {
      // The pipeline depends on the signs of the textures the shaders read.
      for (VulkanShader* shader : {active_vs, active_ps}) {
        if (!shader || !shader->is_ucode_analyzed()) continue;
        for (const auto& binding : shader->texture_bindings()) {
          key ^= Renderer::Mix(
              0x10010 + binding.fetch_constant,
              0x100u | rex::graphics::texture_util::SwizzleSigns(
                           r.GetTextureFetch(binding.fetch_constant)));
        }
      }
    }
    auto found = setup_cache.find(key);
    if (found != setup_cache.end() && found->second.active_vs == active_vs &&
        found->second.active_ps == active_ps &&
        found->second.initiator_state == (initiator.value & 0xFFFF)) {
      setup = found->second;
      setup.state_generation = owner->state_generation_;
      setup.fetch_generation = owner->fetch_generations_.all();
      ++setup_generation;
    } else {
      ++stats().setups_built;
      ++setup_generation;
      BuildDrawSetup(initiator.value);
      setup.identity = AllocateSetupIdentity();
      if (setup.valid) {
        if (setup_cache.size() >= 4096) {
          ClearMapMeasured(setup_cache, 0, setup_cache.bucket_count());
        }
        setup_cache[key] = setup;
      }
    }
  }
  DrawSetup& st = setup;
  if (st.skip_reason != UINT32_MAX) {
    return skip(st.skip_reason);
  }
  VulkanShader* vs = st.vs;
  VulkanShader* ps = st.ps;
  const PassTargets& targets = st.targets;
  const PipelineLayout* layout = st.layout;

  // Indices.
  BeginRecording();
#if COD6_INSTANCE_CONSTANTS
  if (st.instance_constants &&
      instance_record_cursors[submission_number % kSubmissionSlots] >=
          kInstanceRecordsPerSubmission) {
    // Split only before this draw has allocated or recorded anything. The
    // reused submission slot has already waited for its fence in
    // BeginRecording, so resetting its cursor cannot overwrite GPU input.
    Submit();
    BeginRecording();
  }
#endif
  auto dma_size = r.Get<reg::VGT_DMA_SIZE>();
  bool indexed = initiator.source_select != xenos::SourceSelect::kAutoIndex;
  bool index32 = initiator.index_size == xenos::IndexFormat::kInt32;
  bool reset_enabled = st.reset_enabled;
  bool convert_fan = st.convert_fan, convert_loop = st.convert_loop;
  uint32_t reset_index = r.Get<reg::VGT_MULTI_PRIM_IB_RESET_INDX>().reset_indx;
  const uint8_t* guest_indices = nullptr;
  xenos::Endian index_endian = dma_size.swap_mode;
  uint32_t dma_base = 0;
  if (indexed) {
    if (initiator.source_select == xenos::SourceSelect::kDMA) {
      dma_base = r[rex::graphics::XE_GPU_REG_VGT_DMA_BASE] & 0x1FFFFFFF;
      uint32_t bytes = guest_count * (index32 ? 4 : 2);
      if (uint64_t(dma_base) + bytes > 0x20000000u) {
        return skip(8);
      }
      guest_indices = memory->TranslatePhysical<const uint8_t*>(dma_base);
    } else if (immediate_indices) {
      guest_indices = reinterpret_cast<const uint8_t*>(immediate_indices);
      index_endian = xenos::Endian::kNone;
      guest_count = std::min(guest_count, immediate_index_words * (index32 ? 1u : 2u));
    } else {
      return skip(15);
    }
  }
  auto read_index = [&](uint32_t i) -> uint32_t {
    if (!indexed) {
      return i;
    }
    if (index32) {
      uint32_t v;
      std::memcpy(&v, guest_indices + size_t(i) * 4, 4);
      return xenos::GpuSwap(v, index_endian);
    }
    uint16_t v;
    std::memcpy(&v, guest_indices + size_t(i) * 2, 2);
    return xenos::GpuSwap(v, index_endian);
  };
  uint32_t draw_count = guest_count;
  bool host_indexed = indexed || convert_fan || convert_loop;
  VkIndexType index_type = index32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
  VkDeviceSize index_offset = 0;
  if (host_indexed) {
    if (convert_fan) {
      if (guest_count < 3) return;
      draw_count = (guest_count - 2) * 3;
    } else if (convert_loop) {
      if (guest_count < 2) return;
      draw_count = guest_count + 1;
    }
    if (!indexed) {
      index_type = VK_INDEX_TYPE_UINT32;
    }
    uint32_t index_size = index_type == VK_INDEX_TYPE_UINT32 ? 4 : 2;
    // Index buffers in guest memory are usually static: reuse the converted
    // copy while the guest data is the same (checked once per frame).
    bool reused = false;
    VertexRangeKey index_key = {};
    if (dma_base && !convert_fan && !convert_loop) {
      uint32_t bytes = guest_count * (index32 ? 4 : 2);
      VertexHash& known = vertex_hashes[(uint64_t(dma_base) << 32) | bytes];
      RangeHash(known, guest_indices, bytes, dma_base);
      // The conversion also depends on the byte order and the reset index.
      index_key = {dma_base, bytes | 0x80000000u,
                   known.hash ^ (uint64_t(index_endian) << 60) ^
                       (reset_enabled ? (uint64_t(reset_index) << 8) | 1 : 0)};
      uint64_t found = FindRange(index_key);
      if (found != UINT64_MAX && stream_head - found < kStreamSize / 2) {
        index_offset = VkDeviceSize(found % kStreamSize);
        current().stream_oldest = std::min(current().stream_oldest, found);
        reused = true;
      }
    }
    if (!reused) {
      uint8_t* out = Allocate(VkDeviceSize(draw_count) * index_size, 4, index_offset);
      if (!out) {
        return skip(9);
      }
      if (index_key.address) {
        AddRange(index_key, stream_head - VkDeviceSize(draw_count) * index_size);
      }
      uint32_t host_reset = index_size == 4 ? 0xFFFFFFFFu : 0xFFFFu;
      auto put = [&](uint32_t i, uint32_t v) {
        if (reset_enabled && v == reset_index) {
          v = host_reset;
        }
        if (index_size == 4) {
          std::memcpy(out + size_t(i) * 4, &v, 4);
        } else {
          uint16_t h = uint16_t(v);
          std::memcpy(out + size_t(i) * 2, &h, 2);
        }
      };
      if (convert_fan) {
        uint32_t first = read_index(0);
        for (uint32_t t = 0; t + 2 < guest_count; ++t) {
          put(t * 3 + 0, first);
          put(t * 3 + 1, read_index(t + 1));
          put(t * 3 + 2, read_index(t + 2));
        }
      } else if (convert_loop) {
        for (uint32_t i = 0; i < guest_count; ++i) put(i, read_index(i));
        put(guest_count, read_index(0));
      } else if (!reset_enabled && index_endian == xenos::Endian::kNone) {
        std::memcpy(out, guest_indices, size_t(draw_count) * index_size);
      } else if (!reset_enabled && index_size == 2 && index_endian == xenos::Endian::k8in16) {
        rex::memory::copy_and_swap_16_unaligned(out, guest_indices, draw_count);
      } else if (!reset_enabled && index_size == 4 && index_endian == xenos::Endian::k8in32) {
        rex::memory::copy_and_swap_32_unaligned(out, guest_indices, draw_count);
      } else {
        for (uint32_t i = 0; i < draw_count; ++i) put(i, read_index(i));
      }
    }
  }

  // cod5-nx: the guest vertex indices this draw reads, after the base index, the 24-bit mask and the clamp the
  // translated vertex shader applies, for copies limited to that range.
  bool have_bounds = false;
  uint32_t bound_lo = 0, bound_hi = 0;
  const VertexShaderRanges* ranges = nullptr;
  if (REXCVAR_GET(cod6_native_ranged_vertices)) {
    ranges = &GetVertexShaderRanges(vs);
    if (!ranges->index_safe) ranges = nullptr;
  }
  if (ranges && guest_count) {
    uint32_t vmin = UINT32_MAX, vmax = 0;
    auto scan = [&] {
      for (uint32_t i = 0; i < guest_count; ++i) {
        uint32_t v = read_index(i);
        if (reset_enabled && v == reset_index) continue;
        v &= xenos::kVertexIndexMask;
        vmin = std::min(vmin, v);
        vmax = std::max(vmax, v);
      }
    };
    if (!indexed) {
      vmin = 0;
      vmax = guest_count - 1;
    } else if (dma_base) {
      const uint32_t bytes = guest_count * (index32 ? 4 : 2);
      const uint64_t key = (uint64_t(dma_base) << 32) | bytes;
      const uint64_t hash = RangeHash(vertex_hashes[key], guest_indices, bytes, dma_base) ^
                            (uint64_t(index_endian) << 60) ^
                            (reset_enabled ? (uint64_t(reset_index) << 8) | 1 : 0);
      IndexBounds& known = index_bounds[key];
      if (known.hash == hash && hash) {
        vmin = known.min;
        vmax = known.max;
      } else {
        scan();
        known = {hash, vmin, vmax};
      }
      if (index_bounds.size() > 65536) {
        ClearMapMeasured(index_bounds, 1, index_bounds.capacity());
      }
    } else {
      scan();
    }
    if (!indexed) {
      /*
       * COD6_AUTOINDEX_RANGE (2026-11-03)
       *
       * Auto-index draws take their vertex index from the vertex number alone: there is
       * no index buffer to add an offset to, and VGT_MIN_VTX_INDX / VGT_MAX_VTX_INDX only
       * clamp guest indices. Falling through to the indexed path applied both, with the
       * clamp registers still holding the values of the last INDEXED draw - so the range
       * got cut at whatever that draw left behind. The draw then read somebody else's
       * vertices: that is the confetti on big flat surfaces (the loadout table, the
       * container panels) that disappears when ranged copies are turned off.
       */
      bound_lo = 0;
      bound_hi = guest_count - 1;
      have_bounds = true;
    } else if (vmin <= vmax) {
      const uint32_t base = uint32_t(r.Get<int32_t>(rex::graphics::XE_GPU_REG_VGT_INDX_OFFSET));
      const uint64_t lo = uint64_t(vmin) + base, hi = uint64_t(vmax) + base;
      const uint32_t clamp_min = r[rex::graphics::XE_GPU_REG_VGT_MIN_VTX_INDX];
      const uint32_t clamp_max = r[rex::graphics::XE_GPU_REG_VGT_MAX_VTX_INDX];
      // Monotonic only if the 24-bit mask doesn't wrap inside the range.
      if ((lo >> 24) == (hi >> 24) && clamp_min <= clamp_max) {
        bound_lo = std::clamp(uint32_t(lo) & xenos::kVertexIndexMask, clamp_min, clamp_max);
        bound_hi = std::clamp(uint32_t(hi) & xenos::kVertexIndexMask, clamp_min, clamp_max);
        have_bounds = true;
      }
    }
  }

  // Fetch constants: the vertex buffers the shader reads are pointed at their
  // copies in the stream. Only slots written since they were last resolved,
  // or not yet resolved this frame, are looked at again.
  const Shader::ConstantRegisterMap& vs_constants = vs->constant_register_map();
  if (fetch_words_frame != frame || fetch_words_submission != submission_number) {
    // New frame: vertex data is checked for changes again. New submission:
    // copies in older submissions may be reused, but the bookkeeping is
    // simpler starting over.
    fetch_resolved_frame.fill(0);
    fetch_words_frame = frame;
    fetch_words_submission = submission_number;
  }
  uint32_t dirty_slots = owner->vertex_fetch_dirty_;
  owner->vertex_fetch_dirty_ = 0;
  if (dirty_slots) {
    for (uint32_t i = 0; i < 96; ++i) {
      if (dirty_slots & (1u << (i / 3))) fetch_resolved_frame[i] = 0;
    }
    // Texture words are copied as they are; vertex words get rewritten below.
    for (uint32_t slot = 0; slot < 32; ++slot) {
      if (dirty_slots & (1u << slot)) {
        std::memcpy(&fetch_words[slot * 6],
                    &r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + slot * 6],
                    6 * sizeof(uint32_t));
        if (!resolve_exp_bias.empty()) {
          xenos::xe_gpu_texture_fetch_t f;
          std::memcpy(&f, &fetch_words[slot * 6], sizeof(f));
          if (f.type == xenos::FetchConstantType::kTexture) {
            auto it = resolve_exp_bias.find((f.base_address << 12) & 0x1FFFF000);
            if (it != resolve_exp_bias.end()) {
              f.exp_adjust = std::clamp(int32_t(f.exp_adjust) + it->second, -32, 31);
              std::memcpy(&fetch_words[slot * 6], &f, sizeof(f));
            }
          }
        }
      }
    }
  }
  for (uint32_t word = 0; word < 3; ++word) {
    uint32_t bits = vs_constants.vertex_fetch_bitmap[word];
    uint32_t bit;
    while (rex::bit_scan_forward(bits, &bit)) {
      bits &= ~(1u << bit);
      uint32_t fetch_index = word * 32 + bit;
      // cod5-nx: ranged copy, when the shader fetches by vertex index and the draw's bounds are known.
      if (have_bounds) {
        const VertexShaderRanges::Binding* binding = nullptr;
        for (const auto& b : ranges->bindings) {
          if (b.fetch_constant == fetch_index) {
            binding = &b;
            break;
          }
        }
        uint32_t guest_dword0 =
            r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + fetch_index * 2];
        uint32_t dword1 =
            r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + fetch_index * 2 + 1];
        uint32_t address = guest_dword0 & 0x1FFFFFFC;
        uint32_t bytes = std::min(((dword1 >> 2) & 0xFFFFFF) * 4, 16u << 20);
        if (binding && binding->usable && bytes && uint64_t(address) + bytes <= 0x20000000u) {
          /* COD6_VERTEX_CACHE_WHOLE (2026-11-03)
           *
           * BO2 does not crop the vertex range per draw: bo2zm_native_vertex_cache keeps
           * the bytes keyed by content and reuses them across every draw that reads the
           * same buffer (with bo2zm_native_vertex_cache_by_writes to drop them when the
           * pages are written). This file already had that machinery - vertex_hashes,
           * FindRange, AddRange - but it was being fed the PER-DRAW sub-range
           * [bound_lo*stride, bound_hi*stride + extent), so:
           *
           *   - a wrong bound produced wrong vertices (the confetti on flat surfaces),
           *   - and no two draws sharing a buffer could ever hit the same key.
           *
           * Now the whole binding is cached, keyed by (address, length, content hash).
           * Nothing is cropped, so there is no bound to get wrong, and every draw that
           * reads the same buffer reuses one copy. cod6_native_ranged_vertices no longer
           * selects a strategy here; the range is always the whole binding.
           */
          /* COD6_RANGE_BOUNDED (2026-11-04)
           *
           * Caching the WHOLE binding (the previous attempt at "what BO2 does") made the
           * stream copy 61.7 MB per frame with a 46 ms hash: FindRange almost never hits,
           * the stream buffer runs out and wraps, and a wrap overwrites copies that draws
           * already recorded still point at. The GPU then reads somebody else's vertices -
           * the texture-on-the-wrong-quad corruption, made worse instead of fixed.
           *
           * So the per-draw range comes back (0.8 MB per frame, as before), but WITHOUT
           * the truncation to the size the fetch constant declares, which is what dropped
           * the tail of the range and left stale vertices in the stream.
           */
          uint64_t start = uint64_t(bound_lo) * binding->stride_bytes;
          uint64_t end = uint64_t(bound_hi) * binding->stride_bytes + binding->vertex_extent;
          end = std::min<uint64_t>(end, 0x20000000u - uint64_t(address));
          const uint32_t len = uint32_t(end - start);
          if (start < end) {
            uint32_t& dword0 = fetch_words[fetch_index * 2];
            // A later draw with an unsafe shader must look at this slot again.
            fetch_resolved_frame[fetch_index] = 0;
            SlotRange& known_range = slot_ranges[fetch_index];
            if (known_range.frame == frame && known_range.submission == submission_number &&
                known_range.memory_epoch == vertex_memory_epoch &&
                known_range.base == address && known_range.bytes == bytes && start >= known_range.start &&
                end <= known_range.end) {
              dword0 = known_range.dword0;
              continue;
            }
            const uint32_t range_address = address + uint32_t(start);
            const uint8_t* source = memory->TranslatePhysical<const uint8_t*>(range_address);
            VertexHash& known = vertex_hashes[(uint64_t(range_address) << 32) | len];
            RangeHash(known, source, len, range_address);
            VertexRangeKey range_key = {range_address, len, known.hash};
            uint64_t found = FindRange(range_key);
            VkDeviceSize range_offset = 0;
            bool placed = false;
            if (found != UINT64_MAX && stream_head - found < kStreamSize / 2 &&
                found % kStreamSize >= start) {
              range_offset = VkDeviceSize(found % kStreamSize);
              current().stream_oldest = std::min(current().stream_oldest, found);
              placed = true;
            } else {
              // The fetch base becomes range_offset - start, so the copy must sit at least start bytes into the
              // stream: near the start of a lap, skip ahead first.
              if (rex::align(stream_head, uint64_t(16)) % kStreamSize < start) {
                VkDeviceSize pad_offset;
                Allocate(start, 16, pad_offset);
              }
              uint8_t* out = Allocate(len, 16, range_offset);
              if (out && range_offset >= start) {
                CopyVertices(out, source, len, false);
                AddRange(range_key, stream_head - len);
                placed = true;
              }
            }
            if (placed) {
              const uint32_t patched_dword0 =
                  (guest_dword0 & 0x3) | uint32_t(range_offset - start);
              dword0 = patched_dword0;
              known_range = {frame, submission_number, address, bytes, uint32_t(start), uint32_t(end), dword0,
                             vertex_memory_epoch};
              continue;
            }
          }
        }
      }
      if (fetch_resolved_frame[fetch_index] == frame) {
        continue;
      }
      fetch_resolved_frame[fetch_index] = frame;
      uint32_t guest_dword0 =
          r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + fetch_index * 2];
      uint32_t dword1 =
          r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + fetch_index * 2 + 1];
      uint32_t& dword0 = fetch_words[fetch_index * 2];
      dword0 = guest_dword0;
      uint32_t address = guest_dword0 & 0x1FFFFFFC;
      uint32_t size = (dword1 >> 2) & 0xFFFFFF;  // in words
      uint32_t bytes = std::min(size * 4, 16u << 20);
      if (!bytes || uint64_t(address) + bytes > 0x20000000u) {
        continue;
      }
      const uint8_t* source = memory->TranslatePhysical<const uint8_t*>(address);
      VertexHash& known = vertex_hashes[(uint64_t(address) << 32) | bytes];
      RangeHash(known, source, bytes, address);
      VertexRangeKey range_key = {address, bytes, known.hash};
      uint64_t found = FindRange(range_key);
      VkDeviceSize range_offset;
      if (found != UINT64_MAX && stream_head - found < kStreamSize / 2) {
        range_offset = VkDeviceSize(found % kStreamSize);
        current().stream_oldest = std::min(current().stream_oldest, found);
      } else {
        uint8_t* out = Allocate(bytes, 16, range_offset);
        if (!out) continue;
        CopyVertices(out, source, bytes, true);
        AddRange(range_key, stream_head - bytes);
      }
      const uint32_t patched_dword0 = (guest_dword0 & 0x3) | uint32_t(range_offset);
      dword0 = patched_dword0;
    }
  }
  if (vertex_ranges.size() > 65536) {
    ClearMapMeasured(vertex_ranges, 2, vertex_ranges.capacity());
  }
  if (vertex_hashes.size() > 65536) {
    ClearMapMeasured(vertex_hashes, 3, vertex_hashes.capacity());
  }
  VkDeviceSize fetch_offset;
  if (last_fetch.submission == submission_number &&
      !std::memcmp(last_fetch_words, fetch_words.data(), sizeof(last_fetch_words))) {
    fetch_offset = last_fetch.offset;
  } else {
    uint8_t* fetch_out = Allocate(kFetchConstantsBytes, ubo_alignment, fetch_offset);
    if (!fetch_out) return skip(9);
    std::memcpy(fetch_out, fetch_words.data(), kFetchConstantsBytes);
    std::memcpy(last_fetch_words, fetch_words.data(), sizeof(last_fetch_words));
    last_fetch = {submission_number, 0, nullptr, frame, fetch_offset};
  }

  auto write_float_constants = [&](const Shader::ConstantRegisterMap& map,
                                   uint32_t base_register, VkDeviceSize& offset,
                                   bool instance_vertex = false) -> bool {
    // Only the constants the shader reads: the binding's fixed range reads on
    // into whatever follows (never used), which the stream's guard keeps
    // inside the buffer. Allocating the full 4 KB per draw filled the stream
    // and made it wait for the GPU.
    VkDeviceSize bytes = VkDeviceSize(std::max(map.float_count, 1u)) * 4 * sizeof(float);
    std::array<uint8_t, kFloatConstantsBytes> normalized = {};
    uint8_t* allocation = nullptr;
    uint8_t* out = normalized.data();
#if COD6_INSTANCE_CONSTANTS
    if (!instance_vertex) {
#endif
      allocation = Allocate(bytes, ubo_alignment, offset);
      if (!allocation) return false;
      out = allocation;
#if COD6_INSTANCE_CONSTANTS
    }
#else
    (void)instance_vertex;
#endif
    for (uint32_t i = 0; i < 4; ++i) {
      uint64_t bits = map.float_bitmap[i];
      uint32_t bit;
      while (rex::bit_scan_forward(bits, &bit)) {
        bits &= ~(uint64_t(1) << bit);
        const uint32_t constant = (i << 6) + bit;
#if COD6_INSTANCE_CONSTANTS
        if (instance_vertex && constant >= 4 && constant <= 8) {
          std::memset(out, 0, 4 * sizeof(float));
        } else
#endif
        {
          std::memcpy(out, &r[base_register + (constant << 2)],
                      4 * sizeof(float));
        }
        out += 4 * sizeof(float);
      }
    }
#if COD6_INSTANCE_CONSTANTS
    if (instance_vertex) {
      if (fixed_instance_vs_upload.submission == submission_number &&
          fixed_instance_vs_upload.shader == vs &&
          fixed_instance_vs_upload.bytes == bytes &&
          !std::memcmp(fixed_instance_vs_upload.contents.data(), normalized.data(),
                       size_t(bytes))) {
        offset = fixed_instance_vs_upload.offset;
        return true;
      }
      allocation = Allocate(bytes, ubo_alignment, offset);
      if (!allocation) return false;
      std::memcpy(allocation, normalized.data(), size_t(bytes));
      fixed_instance_vs_upload.submission = submission_number;
      fixed_instance_vs_upload.shader = vs;
      fixed_instance_vs_upload.offset = offset;
      fixed_instance_vs_upload.bytes = bytes;
      std::memcpy(fixed_instance_vs_upload.contents.data(), normalized.data(),
                  size_t(bytes));
    }
#endif
    return true;
  };
  VkDeviceSize vs_float_offset, ps_float_offset, bool_loop_offset, system_offset;
#if COD6_INSTANCE_CONSTANTS
  if (st.instance_constants) {
    if (!write_float_constants(vs_constants,
                               rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X,
                               vs_float_offset, true)) {
      return skip(9);
    }
  } else
#endif
  if (Reusable(last_vs_float, owner->vs_float_generation_, vs)) {
    vs_float_offset = last_vs_float.offset;
  } else {
    if (!write_float_constants(vs_constants, rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X,
                               vs_float_offset)) {
      return skip(9);
    }
    last_vs_float = {submission_number, owner->vs_float_generation_, vs, frame, vs_float_offset};
  }
  if (ps) {
    if (Reusable(last_ps_float, owner->ps_float_generation_, ps)) {
      ps_float_offset = last_ps_float.offset;
    } else {
      if (!write_float_constants(ps->constant_register_map(),
                                 rex::graphics::XE_GPU_REG_SHADER_CONSTANT_256_X,
                                 ps_float_offset)) {
        return skip(9);
      }
      last_ps_float = {submission_number, owner->ps_float_generation_, ps, frame,
                       ps_float_offset};
    }
  } else {
    ps_float_offset = vs_float_offset;
  }
  if (Reusable(last_bool_loop, owner->bool_loop_generation_, nullptr)) {
    bool_loop_offset = last_bool_loop.offset;
  } else {
    uint8_t* bool_loop = Allocate(kBoolLoopConstantsBytes, ubo_alignment, bool_loop_offset);
    if (!bool_loop) return skip(9);
    std::memcpy(bool_loop, &r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_BOOL_000_031],
                kBoolLoopConstantsBytes);
    last_bool_loop = {submission_number, owner->bool_loop_generation_, nullptr, frame,
                      bool_loop_offset};
  }

  // Textures and samplers, reused while no fetch slot they come from was
  // written, within one frame (content changes are looked for once a frame)
  // and one submission (uploads go in its setup command buffer).
  upload.setup_commands = current().setup;
  upload.submission = submission_number;
  const VulkanShader* stage_shaders[2] = {vs, ps};
  // Keyed by the shaders and the fetch constants of the slots they read,
  // valid for a frame: materials repeat across the draws of a frame, and a hit
  // skips every texture and sampler lookup.
  if constexpr (cod6::diagnostics::kEnabled) ++stats().binding_key_queries;
  uint32_t slot_mask = st.texture_slot_mask;
  if (!binding_key_memo) {
    if constexpr (cod6::diagnostics::kEnabled) ++stats().binding_key_slot_mask_scans;
    for (const VulkanShader* shader : stage_shaders) {
      if (!shader) continue;
      for (const auto& b : shader->GetTextureBindingsAfterTranslation()) {
        slot_mask |= 1u << b.fetch_constant;
      }
      for (const auto& b : shader->GetSamplerBindingsAfterTranslation()) {
        slot_mask |= 1u << b.fetch_constant;
      }
    }
  }
  const BindingKeyIdentity binding_identity = {
      uint64_t(reinterpret_cast<uintptr_t>(vs)),
      uint64_t(reinterpret_cast<uintptr_t>(ps)),
      uint64_t(reinterpret_cast<uintptr_t>(layout)),
      owner->fetch_generations_.ForMask(slot_mask), slot_mask};
  uint64_t tb_key = 0;
  if (binding_key_memo && binding_key_memo->Lookup(binding_identity, tb_key)) {
    if constexpr (cod6::diagnostics::kEnabled) ++stats().binding_key_memo_hits;
  } else {
    if constexpr (cod6::diagnostics::kEnabled) ++stats().binding_key_hashes;
    tb_key = XXH3_64bits_withSeed(&layout, sizeof(layout),
                                  uint64_t(reinterpret_cast<uintptr_t>(vs)) ^
                                      (uint64_t(reinterpret_cast<uintptr_t>(ps)) << 1));
    uint32_t words[6 * 32];
    uint32_t count = 0;
    uint32_t remaining = slot_mask;
    uint32_t slot;
    while (rex::bit_scan_forward(remaining, &slot)) {
      remaining &= ~(1u << slot);
      std::memcpy(&words[count],
                  &r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + slot * 6],
                  6 * sizeof(uint32_t));
      count += 6;
    }
    tb_key = XXH3_64bits_withSeed(words, count * sizeof(uint32_t), tb_key ^ slot_mask);
    if (binding_key_memo) {
      const bool collision = binding_key_memo->Store(binding_identity, tb_key);
      if constexpr (cod6::diagnostics::kEnabled) {
        if (collision) ++stats().binding_key_collisions;
      }
    }
  }
  if (tb_cache_frame != frame) {
    if (tb_cache.size() > 8192) {
      ClearMapMeasured(tb_cache, 4, tb_cache.capacity());
    }
    tb_cache_frame = frame;
  }
  TextureBindings& tb = tb_cache[tb_key];
  // Any view destroyed anywhere moves views_epoch, which while a level
  // streams in is most frames: instead of rebuilding everything then, the
  // bindings check that their own views are still there.
  auto views_alive = [&]() {
    if (tb.views_epoch == textures.views_epoch()) {
#if COD6_DIAGNOSTICS
      if (diagnostics) ++stats().view_epoch_fast_hits;
#endif
      return true;
    }
#if COD6_DIAGNOSTICS
    if (diagnostics) ++stats().view_alive_batches;
#endif
    if (tb.view_count > 24) {
#if COD6_DIAGNOSTICS
      if (diagnostics) ++stats().view_alive_oversized;
#endif
      return false;
    }
    for (uint32_t i = 0; i < tb.view_count; ++i) {
#if COD6_DIAGNOSTICS
      if (diagnostics) ++stats().view_alive_queries;
      uint64_t probes = 0;
#endif
      const bool alive = textures.ViewAlive(tb.views[i], tb.view_ids[i]
#if COD6_DIAGNOSTICS
                                            , diagnostics ? &probes : nullptr
#endif
                                            );
#if COD6_DIAGNOSTICS
      if (diagnostics) {
        stats().view_alive_probes += probes;
        stats().view_alive_probe_max = std::max(stats().view_alive_probe_max, probes);
      }
#endif
      if (!alive) {
#if COD6_DIAGNOSTICS
        if (diagnostics) ++stats().view_alive_failures;
#endif
        return false;
      }
    }
    tb.views_epoch = textures.views_epoch();
#if COD6_DIAGNOSTICS
    if (diagnostics) ++stats().view_alive_revalidated;
#endif
    return true;
  };
  bool tb_hit = tb.valid && tb.vs == vs && tb.ps == ps && tb.layout == layout &&
                tb.set_generation == set_cache_generation &&
                (tb.answers_epoch == textures.answers_epoch() ||
                 textures.CanReuseAnswers(tb.answers_epoch,
                    &r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0], slot_mask)) && views_alive() &&
                ((tb.frame == frame && tb.submission == submission_number) ||
                 tb.valid_until >= frame);
  if (tb_hit) tb.answers_epoch = textures.answers_epoch();
  if (tb_hit && (tb.frame != frame || tb.submission != submission_number)) {
    for (uint32_t i = 0; i < tb.texture_count; ++i) {
      textures.Touch(tb.textures[i], submission_number);
    }
    tb.frame = frame;
    tb.submission = submission_number;
  }
  if (!tb_hit) {
    TextureTimer texture_timer(frame_texture_ns);
    ++stats().texture_bindings_built;
    // Reserve room for both shader stages BEFORE choosing either set. Rotating
    // pools between stages would retain the first stage's old-pool set while
    // stamping the whole binding with the new generation below. That binding
    // could then survive retirement of the first stage's pool.
    if (texture_set_cache.size() > 16384 - 2) {
      if constexpr (cod6::diagnostics::kEnabled) {
        if (diagnostics) {
          REXLOG_INFO("[native-diag] descriptor pool rotation before stages: frame={} sets={} generation={}",
                      frame, texture_set_cache.size(), set_cache_generation);
        }
      }
      FlushSetCache();
    }
    tb = TextureBindings();
    tb.valid_until = UINT64_MAX;
    for (uint32_t stage = 0; stage < 2; ++stage) {
      const VulkanShader* shader = stage_shaders[stage];
      if (!shader || (!st.counts[stage * 2] && !st.counts[stage * 2 + 1])) continue;
      const auto& texture_bindings = shader->GetTextureBindingsAfterTranslation();
      const auto& sampler_bindings = shader->GetSamplerBindingsAfterTranslation();
      VkDescriptorImageInfo infos[64];
      uint32_t info_count = 0;
      for (const auto& binding : texture_bindings) {
        if (info_count >= 64) break;
        tb.used_mask |= 1u << binding.fetch_constant;
        VkDescriptorImageInfo& info = infos[info_count++];
        info.sampler = VK_NULL_HANDLE;
        info.imageView = SlotView(binding.fetch_constant, binding.dimension,
                                  bool(binding.is_signed));
        info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        tb.valid_until = std::min(tb.valid_until, slot_valid_until);
        if (tb.view_count < 24) {
          tb.views[tb.view_count] = info.imageView;
          tb.view_ids[tb.view_count] = textures.ViewId(info.imageView);
        }
        ++tb.view_count;
        if (slot_texture) {
          if (tb.texture_count < 24) {
            tb.textures[tb.texture_count++] = slot_texture;
          } else {
            tb.valid_until = frame;
          }
        }
      }
      for (const auto& binding : sampler_bindings) {
        if (info_count >= 64) break;
        tb.slots |= 1u << binding.fetch_constant;
        VkDescriptorImageInfo& info = infos[info_count++];
        info.sampler = SlotSampler(binding);
        info.imageView = VK_NULL_HANDLE;
        info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      }
      VkDescriptorSetLayout set_layout =
          stage ? layout->textures_pixel : layout->textures_vertex;
      // Keyed by view ids, never reused, rather than handles, which are: the
      // cache was dropped whenever any view was destroyed, which while a level
      // streams in is most frames, and every material then allocated and
      // wrote new sets (10-50 ms in the frames that dropped).
      uint64_t key_words[64];
      for (uint32_t i = 0; i < info_count; ++i) {
        key_words[i] = infos[i].imageView ? textures.ViewId(infos[i].imageView)
                                          : (uint64_t(1) << 63) ^
                                                uint64_t(reinterpret_cast<uintptr_t>(
                                                    infos[i].sampler));
      }
      uint64_t set_key =
          XXH3_64bits_withSeed(key_words, sizeof(uint64_t) * info_count, uint64_t(set_layout));
      auto cached = texture_set_cache.find(set_key);
      if (cached != texture_set_cache.end()) {
        tb.sets[stage] = cached->second;
        continue;
      }
      VkDescriptorSet set = AllocatePersistentSet(set_layout);
      if (!set) return skip(11);
      texture_set_cache.emplace(set_key, set);
      VkWriteDescriptorSet writes[2] = {};
      uint32_t write_count = 0;
      if (!texture_bindings.empty()) {
        VkWriteDescriptorSet& w = writes[write_count++];
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = set;
        w.dstBinding = 0;
        w.descriptorCount = uint32_t(texture_bindings.size());
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        w.pImageInfo = infos;
      }
      if (!sampler_bindings.empty()) {
        VkWriteDescriptorSet& w = writes[write_count++];
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = set;
        w.dstBinding = uint32_t(texture_bindings.size());
        w.descriptorCount = uint32_t(sampler_bindings.size());
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        w.pImageInfo = infos + texture_bindings.size();
      }
      dfn().vkUpdateDescriptorSets(vk, write_count, writes, 0, nullptr);
      tb.sets[stage] = set;
    }
    tb.slots |= tb.used_mask;
    tb.valid = true;
    tb.frame = frame;
    tb.submission = submission_number;
    tb.vs = vs;
    tb.ps = ps;
    tb.layout = layout;
    tb.views_epoch = textures.views_epoch();
    tb.set_generation = set_cache_generation;
    tb.answers_epoch = textures.answers_epoch();
    // Only the slots these bindings read are cleared: another draw's
    // bindings may still need to see the others as written.
    owner->texture_fetch_dirty_ &= ~tb.slots;
  }

  // System constants come from the render state, the texture fetch
  // constants and the vertex index registers.
  const SystemConstantInputIdentity current_system_inputs = {
      SelectSystemSetupToken(stable_setup_identity, st.identity, setup_generation),
      owner->fetch_generations_.ForMask(tb.used_mask), owner->index_generation_, tb.used_mask};
  if constexpr (cod6::diagnostics::kEnabled) ++stats().system_constant_queries;
  bool system_same = last_system.submission == submission_number &&
                     system_inputs == current_system_inputs;
  if constexpr (cod6::diagnostics::kEnabled) {
    if (system_same) ++stats().system_constant_input_hits;
  }
  if (!system_same) {
    if constexpr (cod6::diagnostics::kEnabled) ++stats().system_constant_recomputes;
    UpdateSystemConstants(st.primitive_polygonal, st.viewport, tb.used_mask, 0, vs, ps);
    system_inputs = current_system_inputs;
    system_same = last_system.submission == submission_number &&
                  !std::memcmp(&last_system_constants, &system_constants,
                               sizeof(system_constants));
    if constexpr (cod6::diagnostics::kEnabled) {
      if (system_same) ++stats().system_constant_content_hits;
    }
  }
  if (system_same) {
    system_offset = last_system.offset;
  } else {
    uint8_t* system_out =
        Allocate(sizeof(SpirvShaderTranslator::SystemConstants), ubo_alignment, system_offset);
    if (!system_out) return skip(9);
    std::memcpy(system_out, &system_constants, sizeof(system_constants));
    std::memcpy(&last_system_constants, &system_constants, sizeof(system_constants));
    last_system = {submission_number, 0, nullptr, frame, system_offset};
    if constexpr (cod6::diagnostics::kEnabled) ++stats().system_constant_uploads;
  }

  // Record.
#if COD6_INSTANCE_CONSTANTS
  VkDeviceSize instance_offset = 0;
  if (st.instance_constants) {
    const uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
    const uint32_t record = instance_record_cursors[slot]++;
    instance_offset = VkDeviceSize(slot) * kInstanceSubmissionBytes +
                      VkDeviceSize(record) * sizeof(InstanceConstantRecord);
    InstanceConstantRecord constants = {};
    for (uint32_t i = 0; i < 5; ++i) {
      std::memcpy(constants.values[i],
                  &r[rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X +
                     ((i + 4) << 2)],
                  sizeof(constants.values[i]));
    }
    std::memcpy(instance_mapping + instance_offset, &constants,
                sizeof(constants));
  }
#endif
  if (!BeginPass(targets)) return skip(14);
  DrawCommand d = {};
  d.commands = current().main;
  d.flags = 0;
  d.instance_count = 1;
  d.layout = layout->layout;
#if COD6_INSTANCE_CONSTANTS
  if (st.instance_constants) {
    d.flags |= kDrawInstanceBuffer;
    d.instance_buffer = instance_buffer;
    d.instance_offset = instance_offset;
  }
#endif
  if (bound_pipeline != st.pipeline) {
    d.flags |= kDrawPipeline;
    d.pipeline = st.pipeline;
    bound_pipeline = st.pipeline;
  }
  if (bound_layout != layout->layout || !bound_static_sets) {
    d.flags |= kDrawSharedSet;
    d.set_shared = set_shared;
    bound_layout = layout->layout;
    bound_static_sets = true;
    bound_offsets_valid = false;
    bound_texture_sets[0] = bound_texture_sets[1] = VK_NULL_HANDLE;
  }
  uint32_t* dynamic_offsets = d.offsets;
  dynamic_offsets[SpirvShaderTranslator::kConstantBufferSystem] = uint32_t(system_offset);
  dynamic_offsets[SpirvShaderTranslator::kConstantBufferFloatVertex] = uint32_t(vs_float_offset);
  dynamic_offsets[SpirvShaderTranslator::kConstantBufferFloatPixel] = uint32_t(ps_float_offset);
  dynamic_offsets[SpirvShaderTranslator::kConstantBufferBoolLoop] = uint32_t(bool_loop_offset);
  dynamic_offsets[SpirvShaderTranslator::kConstantBufferFetch] = uint32_t(fetch_offset);
  if (!bound_offsets_valid || std::memcmp(bound_offsets, d.offsets, sizeof(d.offsets))) {
    d.flags |= kDrawConstants;
    d.set_constants = set_constants;
    std::memcpy(bound_offsets, d.offsets, sizeof(d.offsets));
    bound_offsets_valid = true;
  }
  if (tb.sets[0] && tb.sets[0] != bound_texture_sets[0]) {
    d.flags |= kDrawTexturesVertex;
    d.texture_sets[0] = tb.sets[0];
    bound_texture_sets[0] = tb.sets[0];
  }
  if (tb.sets[1] && tb.sets[1] != bound_texture_sets[1]) {
    d.flags |= kDrawTexturesPixel;
    d.texture_sets[1] = tb.sets[1];
    bound_texture_sets[1] = tb.sets[1];
  }

  // Dynamic state.
  if (std::memcmp(&st.vp, &bound_viewport, sizeof(st.vp))) {
    d.flags |= kDrawViewport;
    d.viewport = st.vp;
    bound_viewport = st.vp;
  }
  VkRect2D sr;
  sr.offset.x = int32_t(std::min(st.scissor.offset[0], pass_extent.width));
  sr.offset.y = int32_t(std::min(st.scissor.offset[1], pass_extent.height));
  sr.extent.width = std::min(st.scissor.extent[0], pass_extent.width - uint32_t(sr.offset.x));
  sr.extent.height = std::min(st.scissor.extent[1], pass_extent.height - uint32_t(sr.offset.y));
  if (std::memcmp(&sr, &bound_scissor, sizeof(sr))) {
    d.flags |= kDrawScissor;
    d.scissor = sr;
    bound_scissor = sr;
  }
  if (std::memcmp(&st.bias_constant, &bound_depth_bias[0], sizeof(float)) ||
      std::memcmp(&st.bias_slope, &bound_depth_bias[1], sizeof(float))) {
    d.flags |= kDrawDepthBias;
    d.bias_constant = st.bias_constant;
    d.bias_slope = st.bias_slope;
    bound_depth_bias[0] = st.bias_constant;
    bound_depth_bias[1] = st.bias_slope;
  }
  if (std::memcmp(st.blend_constants, bound_blend_constants, sizeof(st.blend_constants))) {
    d.flags |= kDrawBlend;
    std::memcpy(d.blend, st.blend_constants, sizeof(d.blend));
    std::memcpy(bound_blend_constants, st.blend_constants, sizeof(st.blend_constants));
  }
  std::memcpy(d.stencil, st.stencil, sizeof(d.stencil));
  if (st.stencil[0] != bound_stencil[0] || st.stencil[1] != bound_stencil[1]) {
    d.flags |= kDrawStencilCompare;
  }
  if (st.stencil[2] != bound_stencil[2] || st.stencil[3] != bound_stencil[3]) {
    d.flags |= kDrawStencilWrite;
  }
  if (st.stencil[4] != bound_stencil[4] || st.stencil[5] != bound_stencil[5]) {
    d.flags |= kDrawStencilReference;
  }
  std::memcpy(bound_stencil, st.stencil, sizeof(st.stencil));

  d.count = draw_count;
  if (host_indexed) {
    d.flags |= kDrawIndexed;
    if (index_offset != bound_index_offset || index_type != bound_index_type ||
        !bound_index_valid) {
      d.flags |= kDrawIndexBuffer;
      d.index_buffer = stream_buffer;
      d.index_offset = index_offset;
      d.index_type = index_type;
      bound_index_offset = index_offset;
      bound_index_type = index_type;
      bound_index_valid = true;
    }
  }
#if COD6_DIAGNOSTICS
  if (diagnostics) {
    // COD4 batches adjacent rigid/static model draws whose complete render
    // identity is equal while VS c4-c8 vary per instance. Measure that exact
    // opportunity before adding an instanced shader ABI. This is deliberately
    // record-only: a hash collision cannot alter a command or the image.
    auto object_bits = [](const auto& object) {
      uint64_t bits = 0;
      static_assert(sizeof(object) <= sizeof(bits));
      std::memcpy(&bits, &object, sizeof(object));
      return bits;
    };
    auto hash_part = [](uint64_t hash, const void* data, size_t size, uint64_t tag) {
      return XXH3_64bits_withSeed(data, size,
                                  hash ^ (tag * UINT64_C(0x9E3779B97F4A7C15)));
    };
    auto hash_float_map = [&](const Shader::ConstantRegisterMap& map,
                              uint32_t base_register, bool select_instance,
                              bool& selected_any, uint64_t seed) {
      uint64_t hash = seed;
      for (uint32_t i = 0; i < 4; ++i) {
        uint64_t bits = map.float_bitmap[i];
        uint32_t bit;
        while (rex::bit_scan_forward(bits, &bit)) {
          bits &= ~(uint64_t(1) << bit);
          const uint32_t register_index = (i << 6) + bit;
          const bool is_instance = register_index >= 4 && register_index <= 8;
          if (is_instance != select_instance) continue;
          selected_any = true;
          hash = hash_part(hash, &r[base_register + (register_index << 2)],
                           4 * sizeof(uint32_t), register_index + 1);
        }
      }
      return hash;
    };

    bool instance_constants_used = false;
    bool fixed_vs_constants_used = false;
    uint64_t instance_constants = hash_float_map(
        vs_constants, rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X, true,
        instance_constants_used, UINT64_C(0xC4C8C4C8C4C8C4C8));
    const uint64_t fixed_vs_constants = hash_float_map(
        vs_constants, rex::graphics::XE_GPU_REG_SHADER_CONSTANT_000_X, false,
        fixed_vs_constants_used, UINT64_C(0x5653464958454401));
    (void)fixed_vs_constants_used;

    // The remaining constant buffers are immutable at an offset for the life
    // of this submission. Equal offsets therefore prove equal uploaded bytes;
    // different offsets conservatively break a group instead of rehashing
    // large PS/system blocks on every diagnostic draw.
    struct FixedConstantIdentity {
      uint64_t fixed_vs;
      uint64_t ps_offset;
      uint64_t bool_loop_offset;
      uint64_t system_offset;
    } fixed_identity{fixed_vs_constants, uint64_t(ps_float_offset),
                     uint64_t(bool_loop_offset), uint64_t(system_offset)};
    const uint64_t fixed_constants =
        XXH3_64bits(&fixed_identity, sizeof(fixed_identity));

    uint64_t target_words[5] = {};
    for (uint32_t i = 0; i < 4; ++i) {
      if (targets.color[i]) {
        target_words[i] = targets.color[i]->diagnostic_identity;
      }
    }
    if (targets.depth) target_words[4] = targets.depth->diagnostic_identity;
    const uint64_t target_hash = XXH3_64bits(target_words, sizeof(target_words));

    uint64_t texture_words[2] = {object_bits(tb.sets[0]), object_bits(tb.sets[1])};
    const uint64_t texture_hash = XXH3_64bits(texture_words, sizeof(texture_words));

    struct GeometryIdentity {
      uint64_t fetch;
      uint64_t index_offset;
      uint32_t count;
      uint32_t index_type;
      uint32_t indexed;
      uint32_t reserved;
    } geometry{};
    // Like the other stream allocations, a fetch UBO offset is immutable in a
    // submission. Reuse is exact; a duplicate copy only causes undercounting.
    geometry.fetch = uint64_t(fetch_offset);
    geometry.index_offset = uint64_t(index_offset);
    geometry.count = draw_count;
    geometry.index_type = uint32_t(index_type);
    geometry.indexed = host_indexed ? 1u : 0u;
    const uint64_t geometry_hash = XXH3_64bits(&geometry, sizeof(geometry));

    uint64_t dynamic_hash = XXH3_64bits(&st.vp, sizeof(st.vp));
    dynamic_hash = hash_part(dynamic_hash, &sr, sizeof(sr), 1);
    dynamic_hash = hash_part(dynamic_hash, &st.bias_constant,
                             sizeof(st.bias_constant), 2);
    dynamic_hash = hash_part(dynamic_hash, &st.bias_slope,
                             sizeof(st.bias_slope), 3);
    dynamic_hash = hash_part(dynamic_hash, st.blend_constants,
                             sizeof(st.blend_constants), 4);
    dynamic_hash = hash_part(dynamic_hash, st.stencil, sizeof(st.stencil), 5);

    InstanceCandidateSignature candidate{};
    candidate.frame = frame;
    candidate.submission = submission_number;
    candidate.pass = instance_pass_serial;
    candidate.pipeline = object_bits(st.pipeline);
    candidate.layout = object_bits(layout->layout);
    candidate.targets = target_hash;
    candidate.geometry = geometry_hash;
    candidate.textures = texture_hash;
    candidate.fixed_constants = fixed_constants;
    candidate.dynamic_state = dynamic_hash;
    candidate.instance_constants = instance_constants;
    // A dynamically addressed VS may select c4-c8 through the address
    // register even when a static bitmap looks compatible. Until an
    // instanced shader proves that addressing transformation, fail closed.
    candidate.uses_instance_constants =
        instance_constants_used && !vs_constants.float_dynamic_addressing;
    instance_candidate_tracker.Observe(candidate);

    const auto& counters = instance_candidate_tracker.counters();
    stats().instance_candidate_draws = counters.draws;
    stats().instance_candidate_eligible = counters.eligible;
    stats().instance_candidate_groups = counters.groups;
    stats().instance_candidate_reducible = counters.reducible;
    stats().instance_candidate_changed = counters.instance_changed;
    stats().instance_candidate_same = counters.instance_same;
    stats().instance_break_context = counters.break_context;
    stats().instance_break_no_instance = counters.break_no_instance;
    stats().instance_break_pipeline_state = counters.break_pipeline_state;
    stats().instance_break_geometry = counters.break_geometry;
    stats().instance_break_textures = counters.break_textures;
    stats().instance_break_fixed_constants = counters.break_fixed_constants;
    stats().instance_break_dynamic_state = counters.break_dynamic_state;
    stats().instance_candidate_max_group = counters.max_group;
  }
#endif
#if COD6_INSTANCE_CONSTANTS
  if (st.instance_constants) {
    InstanceDrawSignature signature{};
    signature.pipeline = st.pipeline;
    signature.layout = layout->layout;
    for (uint32_t i = 0; i < 4; ++i) signature.targets[i] = targets.color[i];
    signature.targets[4] = targets.depth;
    signature.textures[0] = tb.sets[0];
    signature.textures[1] = tb.sets[1];
    std::memcpy(signature.constant_offsets, d.offsets, sizeof(d.offsets));
    signature.viewport = st.vp;
    signature.scissor = sr;
    signature.bias[0] = st.bias_constant;
    signature.bias[1] = st.bias_slope;
    std::memcpy(signature.blend, st.blend_constants, sizeof(signature.blend));
    std::memcpy(signature.stencil, st.stencil, sizeof(signature.stencil));
    signature.index_offset = index_offset;
    signature.index_type = index_type;
    signature.count = draw_count;
    signature.indexed = host_indexed;
    instance_group.Offer(
        d, signature, instance_offset,
        [this](const DrawCommand& first, const VkDeviceSize* offsets,
               uint32_t count) {
          EmitInstanceGroup(first, offsets, count);
        });
  } else {
    FlushInstanceGroup();
    SubmitDraw(d);
  }
#else
  SubmitDraw(d);
#endif
#if COD6_DIAGNOSTICS
  ProfileDraw();
#endif
#if COD6_DIAGNOSTICS
  for (RenderTarget* target : {targets.color[0], targets.color[1], targets.color[2],
                               targets.color[3], targets.depth}) {
    // Conservatively treat every bound attachment as written. This may lose
    // candidates for read-only depth or zero color masks, but cannot report a
    // repeat while a draw may have changed the source.
    MarkRenderTargetWritten(target);
  }
#endif
  if (draw_table_left && draw_table_skip_left) {
    --draw_table_skip_left;
  } else if (draw_table_left) {
    --draw_table_left;
    std::string slots;
    uint32_t slot_mask = tb.used_mask;
    uint32_t slot;
    while (rex::bit_scan_forward(slot_mask, &slot)) {
      slot_mask &= ~(1u << slot);
      const xenos::xe_gpu_texture_fetch_t f = r.GetTextureFetch(slot);
      slots += fmt::format(" {}:{:08X}/f{}/{}x{}", slot, f.base_address << 12,
                           uint32_t(f.format), f.size_2d.width + 1, f.size_2d.height + 1);
    }
    REXLOG_INFO("[dtab] {:5} ps={:016X} vs={:016X} vp={}x{} n={} p{} m={:08X}{}",
                draw_table_index, ps ? ps->ucode_data_hash() : 0,
                vs ? vs->ucode_data_hash() : 0, st.viewport.xy_extent[0],
                st.viewport.xy_extent[1], draw_count, uint32_t(st.primitive_polygonal),
                tb.used_mask, slots);
    ++draw_table_index;
  }
  ++frame_draw_count;
  ++stats().draws;
#if COD6_DIAGNOSTICS
  if (diagnostics && targets.depth) {
    bool color = false;
    for (const auto* target : targets.color) color |= target != nullptr;
    NoteShadow(0, uint64_t(targets.depth->width) | (uint64_t(targets.depth->height) << 32),
               uint64_t(targets.depth->format), uint64_t(color));
  }
#endif
  if (trace_frame) {
    ++trace_pass_draws;
    // Details of the draws of small passes (post-processing): shaders, target exponent bias, sampled textures.
    if (trace_pass_draws <= 2 && trace_draws_detailed < 120 &&
        (!targets.depth || targets.Empty() || st.ps)) {
      ++trace_draws_detailed;
      auto ci = r.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[0]);
      std::string tex;
      if (ps) {
        for (const auto& tb : ps->texture_bindings()) {
          xenos::xe_gpu_texture_fetch_t f = r.GetTextureFetch(tb.fetch_constant);
          xenos::xe_gpu_texture_fetch_t patched;
          std::memcpy(&patched, &fetch_words[tb.fetch_constant * 6], sizeof(patched));
          tex += fmt::format(" t{}:{:08X} fmt {} {}x{} exp {}->{} signs {}{}{}{}", tb.fetch_constant,
                             f.base_address << 12, uint32_t(f.format), f.size_2d.width + 1, f.size_2d.height + 1,
                             int32_t(f.exp_adjust), int32_t(patched.exp_adjust),
                             uint32_t(f.sign_x), uint32_t(f.sign_y),
                             uint32_t(f.sign_z), uint32_t(f.sign_w));
        }
      }
      REXLOG_INFO("[trace]     draw vs {:016X} ps {:016X} rt0 fmt {} exp_bias {} blend {:08X}{}",
                  vs ? vs->ucode_data_hash() : 0, ps ? ps->ucode_data_hash() : 0, uint32_t(ci.color_format),
                  int32_t(ci.color_exp_bias), r[rex::graphics::XE_GPU_REG_RB_BLENDCONTROL0], tex);
      if (ps && trace_disassembled.insert(ps->ucode_data_hash()).second) {
        REXLOG_INFO("[trace]     ps {:016X} disassembly:\n{}", ps->ucode_data_hash(), ps->ucode_disassembly());
      }
    }
  }
  for (RenderTarget* rt : targets.color) {
    if (rt) rt->last_used_frame = frame;
  }
}

// --- Resolves ------------------------------------------------------------------------

bool Renderer::Impl::ResolveDownsampled(RenderTarget* full, int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                                        xenos::CopyCommand command) {
  const RegisterFile& r = *regs;
  const uint32_t dest_base = r[rex::graphics::XE_GPU_REG_RB_COPY_DEST_BASE];
  if (!dest_base || (command != xenos::CopyCommand::kRaw && command != xenos::CopyCommand::kConvert)) return false;
  const auto dest_info = r.Get<reg::RB_COPY_DEST_INFO>();
  const uint32_t height = std::min(uint32_t(y1 - y0), full->height / 2 - std::min(uint32_t(y0), full->height / 2));
  const uint32_t width = std::min(uint32_t(x1 - x0), full->width / 2 - std::min(uint32_t(x0), full->width / 2));
  if (!width || !height) return false;
  if (trace_frame) {
    TraceEndPass();
    REXLOG_INFO("[trace] resolve (4x view of {}) rect {},{}-{},{} -> {:08X}: 2x2 downsample", TraceTarget(full), x0, y0,
                x1, y1, dest_base);
  }
  ++stats().resolves;
  BeginRecording();
  EndPass();
  VkCommandBuffer c = cmd();
  VkMemoryBarrier to_transfer = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
                                 VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT};
  dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &to_transfer, 0, nullptr, 0, nullptr);
  uint32_t host_swizzle = uint32_t(xenos::XE_GPU_TEXTURE_SWIZZLE_RGBA);
  if (dest_info.copy_dest_swap) host_swizzle = XE_GPU_MAKE_TEXTURE_SWIZZLE(B, G, R, A);
  ResolveDestination* dest = textures.GetResolveDestination(
      dest_base & 0x1FFFFFFF, std::min(uint32_t(x0) + width, 8192u), std::min(uint32_t(y0) + height, 8192u),
      full->format, host_swizzle, c);
  if (!dest) return false;
  if (trace_frame) trace_last_dest = dest;
  NoteResolveExpBias(dest_base, xenos::ColorRenderTargetFormat(full->key.format));
  VkImageBlit blit = {};
  blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blit.dstSubresource = blit.srcSubresource;
  blit.srcOffsets[0] = {x0 * 2, y0 * 2, 0};
  blit.srcOffsets[1] = {int32_t(x0 + width) * 2, int32_t(y0 + height) * 2, 1};
  blit.dstOffsets[0] = {x0, y0, 0};
  blit.dstOffsets[1] = {int32_t(x0 + width), int32_t(y0 + height), 1};
  // Linear filtering at exactly half the size averages each 2x2 block: the 4 samples of the 4x pixel.
  dfn().vkCmdBlitImage(c, full->image, VK_IMAGE_LAYOUT_GENERAL, dest->image, VK_IMAGE_LAYOUT_GENERAL, 1, &blit,
                       VK_FILTER_LINEAR);
  textures.CopyToSized(*dest, c);
#if COD6_DIAGNOSTICS
  NoteSuccessfulResolve(full, dest, uint32_t(x0), uint32_t(y0), width, height,
                        r.Get<reg::RB_COPY_CONTROL>().value, dest_info.value,
                        kRepeatDownsample);
#endif
  resolve_barrier_pending = true;
  return true;
}

void Renderer::Impl::Resolve() {
  const RegisterFile& r = *regs;
  auto copy_control = r.Get<reg::RB_COPY_CONTROL>();
  auto surface_info = r.Get<reg::RB_SURFACE_INFO>();

  // The rectangle (draw_util::GetResolveInfo): Direct3D 9 puts its three
  // vertices in vertex fetch constant 0.
  xenos::xe_gpu_vertex_fetch_t fetch = r.GetVertexFetch(0);
  if (fetch.type != xenos::FetchConstantType::kVertex || fetch.size != 3 * 2) {
    return;
  }
  const float* vertices = memory->TranslatePhysical<const float*>(fetch.address << 2);
  float half_pixel =
      r.Get<reg::PA_SU_VTX_CNTL>().pix_center == xenos::PixelCenter::kD3DZero ? 0.5f : 0.0f;
  int32_t v[6];
  for (uint32_t i = 0; i < 6; ++i) {
    float f = xenos::GpuSwap(vertices[i], fetch.endian) + half_pixel;
    v[i] = int32_t(f * 256.0f);
  }
  int32_t x0 = (std::min({v[0], v[2], v[4]}) + 127) >> 8;
  int32_t y0 = (std::min({v[1], v[3], v[5]}) + 127) >> 8;
  int32_t x1 = (std::max({v[0], v[2], v[4]}) + 127) >> 8;
  int32_t y1 = (std::max({v[1], v[3], v[5]}) + 127) >> 8;
  if (r.Get<reg::PA_SU_SC_MODE_CNTL>().vtx_window_offset_enable) {
    auto offset = r.Get<reg::PA_SC_WINDOW_OFFSET>();
    x0 += offset.window_x_offset;
    x1 += offset.window_x_offset;
    y0 += offset.window_y_offset;
    y1 += offset.window_y_offset;
  }
  draw_util::Scissor scissor;
  draw_util::GetScissor(r, scissor, false);
  x0 = std::clamp(x0, int32_t(scissor.offset[0]), int32_t(scissor.offset[0] + scissor.extent[0]));
  x1 = std::clamp(x1, int32_t(scissor.offset[0]), int32_t(scissor.offset[0] + scissor.extent[0]));
  y0 = std::clamp(y0, int32_t(scissor.offset[1]), int32_t(scissor.offset[1] + scissor.extent[1]));
  y1 = std::clamp(y1, int32_t(scissor.offset[1]), int32_t(scissor.offset[1] + scissor.extent[1]));
  x0 &= ~7;
  y0 &= ~7;
  x1 = rex::align(x1, 8);
  y1 = rex::align(y1, 8);
  x1 = std::min(x1, int32_t(surface_info.surface_pitch));
  if (x0 >= x1 || y0 >= y1) {
    return;
  }
  uint32_t guest_msaa = uint32_t(surface_info.msaa_samples);
  uint32_t msaa = HostMsaa(surface_info.msaa_samples);
  bool is_depth = copy_control.copy_src_select >= 4;
  ++stats().resolves;

  BeginRecording();
  EndPass();
#if COD6_DIAGNOSTICS
  ProfileMark((kProfileResolve << 60) | (uint64_t(is_depth) << 56) |
              (uint64_t(surface_info.surface_pitch & 0x7FF) << 32) |
              (uint64_t(uint32_t(x1 - x0) & 0xFFFF) << 16) | uint32_t(y1 - y0));
#endif
  VkCommandBuffer c = cmd();

  RenderTargetKey source_key = {};
  source_key.pitch = surface_info.surface_pitch;
  source_key.msaa = msaa;
  if (is_depth) {
    auto depth_info = r.Get<reg::RB_DEPTH_INFO>();
    source_key.base_tiles = depth_info.depth_base;
    source_key.format = uint32_t(depth_info.depth_format);
    source_key.is_depth = 1;
  } else {
    auto color_info =
        r.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[copy_control.copy_src_select]);
    source_key.base_tiles = color_info.color_base;
    source_key.format = uint32_t(color_info.color_format);
  }
  // cod5-nx: a resolve through a 4x view at half the pitch of a 1x surface (nothing drawn into the 4x view
  // itself): on the console each resolved pixel averages the 4 samples, which are a 2x2 block of the 1x surface, so
  // COD5 uses it as a free half-size downsample (its bloom/exposure chain). The empty 4x image read as zeros, the
  // exposure came out 0 and the whole frame black.
  if (!is_depth && guest_msaa == 2) {
    uint32_t sx = 0, sy = 0;
    RenderTarget* full = FindClearAlias(source_key, guest_msaa, sx, sy);
    if (full) {
      auto own = render_targets.find(CanonicalTargetKey(source_key));
      if (own != render_targets.end() && own->second->last_drawn_frame >= full->last_drawn_frame) full = nullptr;
    }
    if (full && sy == 2 && full->samples == VK_SAMPLE_COUNT_1_BIT) {
      if (ResolveDownsampled(full, x0, y0, x1, y1, copy_control.copy_command)) return;
    }
  }
  RenderTarget* source = GetRenderTarget(source_key, uint32_t(y1), guest_msaa);
  if (source && !source->ever_resolved) {
    source->ever_resolved = true;
    ++rt_generation;  // setups that skipped draws into it are rebuilt
  }
  if (trace_frame) {
    TraceEndPass();
    auto trace_dest = r.Get<reg::RB_COPY_DEST_INFO>();
    REXLOG_INFO("[trace] resolve {} rect {},{}-{},{} -> {:08X} cmd {} dest fmt {} swap {} exp {} clear {}{}",
                TraceTarget(source), x0, y0, x1, y1, r[rex::graphics::XE_GPU_REG_RB_COPY_DEST_BASE],
                uint32_t(copy_control.copy_command), uint32_t(trace_dest.copy_dest_format),
                uint32_t(trace_dest.copy_dest_swap), int32_t(trace_dest.copy_dest_exp_bias),
                copy_control.color_clear_enable ? "C" : "", copy_control.depth_clear_enable ? "D" : "");
  }
  if (!source) return;
  c = cmd();
  uint32_t width = uint32_t(x1 - x0);
  uint32_t height = std::min(uint32_t(y1 - y0), source->height - std::min(uint32_t(y0), source->height));
  if (!height) return;
  const bool scene_scaled_source = IsSceneTarget(source);
  const int32_t scaled_x0 = scene_scaled_source ? int32_t(scene_scale.Floor(uint32_t(x0))) : x0;
  const int32_t scaled_y0 = scene_scaled_source ? int32_t(scene_scale.Floor(uint32_t(y0))) : y0;
  const int32_t scaled_x1 = scene_scaled_source ? int32_t(scene_scale.Ceil(uint32_t(x1))) : x1;
  const int32_t scaled_y1 = scene_scaled_source ? int32_t(scene_scale.Ceil(uint32_t(y0) + height))
                                                 : int32_t(uint32_t(y0) + height);
  const uint32_t scaled_width = uint32_t(scaled_x1 - scaled_x0);
  const uint32_t scaled_height = uint32_t(scaled_y1 - scaled_y0);
  if (scene_scaled_source) {
    static bool logged_scaled_resolve[2] = {};
    const uint32_t kind = is_depth ? 1u : 0u;
    if (!logged_scaled_resolve[kind]) {
      logged_scaled_resolve[kind] = true;
      REXLOG_INFO(
          "[scene-scale] active kind={} target=t{}p{} guest={},{} {}x{} raster={},{} {}x{}",
          is_depth ? "depth" : "color", source->key.base_tiles,
          source->key.pitch, x0, y0, width, height, scaled_x0, scaled_y0,
          scaled_width, scaled_height);
    }
  }

  // Copy.
  uint32_t dest_base = r[rex::graphics::XE_GPU_REG_RB_COPY_DEST_BASE];
  auto dest_info = r.Get<reg::RB_COPY_DEST_INFO>();
  bool copy = dest_base != 0 &&
              (copy_control.copy_command == xenos::CopyCommand::kRaw ||
               copy_control.copy_command == xenos::CopyCommand::kConvert);
  if (copy) {
    VkMemoryBarrier to_transfer = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                       VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT};
    dfn().vkCmdPipelineBarrier(c,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &to_transfer, 0, nullptr,
                               0, nullptr);
    // Sized by what the resolve covers, not by the destination pitch (aligned
    // to 32): textures sample the image across its whole size, and the game
    // samples its full-screen resolves at 1040 wide, where the pitch made the
    // image 1056 and every one of them needed a corrected copy each frame.
    // Smaller textures sampled from a corner still get one (CopyToSized).
    uint32_t dest_width = uint32_t(x1);
    uint32_t dest_height = uint32_t(y0) + height;
    dest_width = std::min(dest_width, 8192u);
    dest_height = std::min(dest_height, 8192u);
    VkFormat dest_format = is_depth ? VK_FORMAT_R32_SFLOAT : source->format;
    uint32_t host_swizzle = is_depth ? uint32_t(xenos::XE_GPU_TEXTURE_SWIZZLE_RRRR)
                                     : uint32_t(xenos::XE_GPU_TEXTURE_SWIZZLE_RGBA);
    if (!is_depth && dest_info.copy_dest_swap) {
      host_swizzle = XE_GPU_MAKE_TEXTURE_SWIZZLE(B, G, R, A);
    }
    ResolveDestination* dest = textures.GetResolveDestination(
        dest_base & 0x1FFFFFFF, dest_width, dest_height, dest_format, host_swizzle, c);
#if COD6_DIAGNOSTICS
    if (is_depth) {
      // An attempt with a destination, not proof that its GPU contents are correct.
      NoteShadow(2, dest_base & 0x1FFFFFFF,
                 uint64_t(dest_width) | (uint64_t(dest_height) << 32),
                 uint64_t(source->format) | (uint64_t(dest != nullptr) << 32) |
                     (uint64_t(source->samples) << 40));
    }
#endif
    if (trace_frame) trace_last_dest = dest;
    if (!is_depth) NoteResolveExpBias(dest_base, xenos::ColorRenderTargetFormat(source_key.format));
    if (dest) {
#if COD6_DIAGNOSTICS
      bool diagnostic_resolve_written = false;
      ResolveRepeatPath diagnostic_resolve_path = kRepeatColorTransfer;
#endif
      if (is_depth) {
        // Depth cannot be copied into a color image directly. It goes through
        // a scratch buffer, where a compute pass makes both forms a game can
        // read a depth resolve back as: floats, into this R32_SFLOAT image for
        // k_24_8 fetches, and the words the resolve leaves in Xbox 360
        // memory, (depth24 << 8) | stencil, into a second image for fetches
        // as any other format (see TextureCache::kDepthWordsTag). Games read
        // the stencil byte that way to tell the player from the scene: with
        // depth alone in every channel, a post effect meant to skip the skater
        // drew him doubled with a light halo.
        const uint64_t texels = uint64_t(width) * height;
        const VkDeviceSize stencil_offset = rex::align(VkDeviceSize(texels * 4), VkDeviceSize(256));
        const VkDeviceSize packed_offset =
            rex::align(stencil_offset + VkDeviceSize(texels), VkDeviceSize(256));
        const bool has_stencil = HasStencil(source->format);
        const bool unorm_guest = xenos::DepthRenderTargetFormat(source->key.format) ==
                                 xenos::DepthRenderTargetFormat::kD24S8;
        const bool direct_depth =
            msaa == 0 && depth_copy_pipeline && source->sample_view &&
            !textures.WantsDepthWords(dest_base & 0x1FFFFFFF);
        if (scene_scaled_source && !direct_depth) {
          static bool logged_scene_depth_fallback = false;
          if (!logged_scene_depth_fallback) {
            logged_scene_depth_fallback = true;
            REXLOG_ERROR("[scene-scale] scaled depth resolve reached the legacy scratch path; "
                         "depth-word consumers are not an accepted scene-scale configuration");
          }
        }
        if (direct_depth) {
          // The usual case: the game samples the resolve only as depth.
          VkMemoryBarrier to_compute = {
              VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                  VK_ACCESS_SHADER_WRITE_BIT,
              VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
          dfn().vkCmdPipelineBarrier(
              c,
              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                  VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &to_compute, 0, nullptr, 0, nullptr);
          VkDescriptorSet set = AllocateSet(depth_copy_set_layout);
          if (set) {
            VkDescriptorImageInfo images[2] = {
                {VK_NULL_HANDLE, source->sample_view, VK_IMAGE_LAYOUT_GENERAL},
                {VK_NULL_HANDLE, dest->view, VK_IMAGE_LAYOUT_GENERAL}};
            VkWriteDescriptorSet writes[2] = {};
            for (uint32_t i = 0; i < 2; ++i) {
              writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
              writes[i].dstSet = set;
              writes[i].dstBinding = i;
              writes[i].descriptorCount = 1;
              writes[i].descriptorType =
                  i ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
              writes[i].pImageInfo = &images[i];
            }
            dfn().vkUpdateDescriptorSets(vk, 2, writes, 0, nullptr);
            int32_t params[8] = {
                scaled_x0, scaled_y0, x0, y0, int32_t(scaled_width),
                int32_t(scaled_height), int32_t(width), int32_t(height)};
            dfn().vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_COMPUTE, depth_copy_pipeline);
            dfn().vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_COMPUTE, depth_copy_layout, 0,
                                          1, &set, 0, nullptr);
            dfn().vkCmdPushConstants(c, depth_copy_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                     sizeof(params), params);
            dfn().vkCmdDispatch(c, (width + 7) / 8, (height + 7) / 8, 1);
#if COD6_DIAGNOSTICS
            diagnostic_resolve_written = true;
            diagnostic_resolve_path = kRepeatDepthDirect;
#endif
          }
          // Readers: the draws, CopyToSized below, and the next depth test.
          VkMemoryBarrier after_compute = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                                           VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
          dfn().vkCmdPipelineBarrier(
              c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
              VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                  VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              0, 1, &after_compute, 0, nullptr, 0, nullptr);
        } else if (msaa == 0 && depth_scratch && depth_convert_pipeline &&
                   packed_offset + texels * 4 <= kDepthScratchSize) {
          VkBufferImageCopy regions[2] = {};
          regions[0].imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
          regions[0].imageOffset = {x0, y0, 0};
          regions[0].imageExtent = {width, height, 1};
          regions[1] = regions[0];
          regions[1].bufferOffset = stencil_offset;
          regions[1].imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
          dfn().vkCmdCopyImageToBuffer(c, source->image, VK_IMAGE_LAYOUT_GENERAL, depth_scratch,
                                       has_stencil ? 2 : 1, regions);
          VkMemoryBarrier to_compute = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
          dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &to_compute, 0,
                                     nullptr, 0, nullptr);
          uint32_t count = uint32_t(texels);
          uint32_t groups = (count + 63) / 64;
          constexpr uint32_t kGroupsPerRow = 4096;
          uint32_t params[5] = {count, kGroupsPerRow * 64, uint32_t(stencil_offset / 4),
                                uint32_t(packed_offset / 4),
                                (source->format == VK_FORMAT_D24_UNORM_S8_UINT ? 1u : 0u) |
                                    (has_stencil ? 2u : 0u)};
          dfn().vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_COMPUTE, depth_convert_pipeline);
          dfn().vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_COMPUTE, depth_convert_layout,
                                        0, 1, &depth_convert_set, 0, nullptr);
          dfn().vkCmdPushConstants(c, depth_convert_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                   sizeof(params), params);
          dfn().vkCmdDispatch(c, std::min(groups, kGroupsPerRow),
                              (groups + kGroupsPerRow - 1) / kGroupsPerRow, 1);
          VkMemoryBarrier to_copy = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                     VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT};
          dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &to_copy, 0, nullptr,
                                     0, nullptr);
          VkBufferImageCopy out = regions[0];
          out.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          dfn().vkCmdCopyBufferToImage(c, depth_scratch, dest->image, VK_IMAGE_LAYOUT_GENERAL, 1,
                                       &out);
#if COD6_DIAGNOSTICS
          diagnostic_resolve_written = true;
          diagnostic_resolve_path = kRepeatDepthScratch;
#endif
          // The words only for unorm depth: float24 would need its own
          // encoding, and the game samples those buffers (shadow maps) as depth.
          if (unorm_guest) {
            ResolveDestination* words = textures.GetResolveDestination(
                (dest_base & 0x1FFFFFFF) | TextureCache::kDepthWordsTag, dest_width, dest_height,
                VK_FORMAT_R8G8B8A8_UNORM, uint32_t(xenos::XE_GPU_TEXTURE_SWIZZLE_RGBA), c);
            if (words) {
              out.bufferOffset = packed_offset;
              dfn().vkCmdCopyBufferToImage(c, depth_scratch, words->image,
                                           VK_IMAGE_LAYOUT_GENERAL, 1, &out);
              textures.CopyToSized(*words, c);
            }
#if COD6_DIAGNOSTICS
            // A later retry may create the required words image. Do not call
            // an incomplete multi-output depth resolve a repeat candidate.
            if (!words) diagnostic_resolve_written = false;
#endif
          }
        } else {
          static bool logged = false;
          if (!logged) {
            logged = true;
            REXLOG_WARN("[native] Depth resolve not done: {}x{}, {}x MSAA", width, height,
                        1u << msaa);
          }
        }
      } else if (scene_scaled_source && source->samples == VK_SAMPLE_COUNT_1_BIT) {
        VkImageBlit blit = {};
        blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        blit.dstSubresource = blit.srcSubresource;
        blit.srcOffsets[0] = {scaled_x0, scaled_y0, 0};
        blit.srcOffsets[1] = {scaled_x1, scaled_y1, 1};
        blit.dstOffsets[0] = {x0, y0, 0};
        blit.dstOffsets[1] = {x1, int32_t(uint32_t(y0) + height), 1};
        dfn().vkCmdBlitImage(c, source->image, VK_IMAGE_LAYOUT_GENERAL, dest->image,
                             VK_IMAGE_LAYOUT_GENERAL, 1, &blit, VK_FILTER_LINEAR);
#if COD6_DIAGNOSTICS
        diagnostic_resolve_written = true;
        diagnostic_resolve_path = kRepeatColorTransfer;
#endif
      } else if (msaa) {
        VkImageResolve resolve = {};
        resolve.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        resolve.dstSubresource = resolve.srcSubresource;
        resolve.srcOffset = {x0, y0, 0};
        resolve.dstOffset = {x0, y0, 0};
        resolve.extent = {width, height, 1};
        vkCmdResolveImage(c, source->image, VK_IMAGE_LAYOUT_GENERAL, dest->image,
                                VK_IMAGE_LAYOUT_GENERAL, 1, &resolve);
#if COD6_DIAGNOSTICS
        diagnostic_resolve_written = true;
        diagnostic_resolve_path = kRepeatColorMsaa;
#endif
      } else if (source->samples == VK_SAMPLE_COUNT_1_BIT && textures.StorageCapable(dest->format) &&
                 ComputeColorCopy(c, source->view, dest->view, x0, y0, x0, y0, width, height)) {
        // Done by compute (see copy_color.cs.glsl).
#if COD6_DIAGNOSTICS
        diagnostic_resolve_written = true;
        diagnostic_resolve_path = kRepeatColorCompute;
#endif
      } else {
        VkImageCopy region = {};
        region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.dstSubresource = region.srcSubresource;
        region.srcOffset = {x0, y0, 0};
        region.dstOffset = {x0, y0, 0};
        region.extent = {width, height, 1};
        dfn().vkCmdCopyImage(c, source->image, VK_IMAGE_LAYOUT_GENERAL, dest->image,
                             VK_IMAGE_LAYOUT_GENERAL, 1, &region);
#if COD6_DIAGNOSTICS
        diagnostic_resolve_written = true;
        diagnostic_resolve_path = kRepeatColorTransfer;
#endif
      }
      textures.CopyToSized(*dest, c);
#if COD6_DIAGNOSTICS
      if (diagnostic_resolve_written) {
        NoteSuccessfulResolve(source, dest, uint32_t(x0), uint32_t(y0), width, height,
                              copy_control.value, dest_info.value, diagnostic_resolve_path);
      }
#endif
    }
  }

  // Clear (RenderTargetCache::PerformTransfersAndResolveClears).
  bool clear_depth = copy_control.depth_clear_enable != 0;
  bool clear_color = !is_depth && copy_control.color_clear_enable != 0;
  if (clear_depth || clear_color) {
    // After a copy the clear must wait for it (it reads what the clear
    // overwrites). Without one, the clear only follows earlier rendering,
    // which the pending resolve barrier or pass order already covers.
    if (copy) {
      VkMemoryBarrier to_clear = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                  VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
      dfn().vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                 0, 1, &to_clear, 0, nullptr, 0, nullptr);
    }
    PassTargets clear_targets;
    RenderTarget* depth_target = nullptr;
    RenderTarget* color_target = nullptr;
    VkClearDepthStencilValue depth_clear_value = {};
    if (clear_depth) {
      auto depth_info = r.Get<reg::RB_DEPTH_INFO>();
      RenderTargetKey key = {};
      key.base_tiles = depth_info.depth_base;
      key.pitch = surface_info.surface_pitch;
      key.format = uint32_t(depth_info.depth_format);
      key.msaa = msaa;
      key.is_depth = 1;
      depth_target = GetRenderTarget(key, uint32_t(y1), guest_msaa);
      clear_targets.depth = depth_target;
      uint32_t clear = r[rex::graphics::XE_GPU_REG_RB_DEPTH_CLEAR];
      uint32_t depth24 = clear >> 8;
      depth_clear_value.depth =
          xenos::DepthRenderTargetFormat(depth_target->key.format) ==
                  xenos::DepthRenderTargetFormat::kD24FS8
              ? xenos::Float20e4To32(depth24)
              : xenos::UNorm24To32(depth24);
      depth_clear_value.stencil = clear & 0xFF;
    }
    if (clear_color) {
      color_target = source;
      clear_targets.color[0] = color_target;
    }
    EndPass();

    // Compute the guest clear rectangle once before a ZCULL depth clear may
    // be split from the color clear. Both fixed scene attachments have the
    // same scaled extent; other targets retain the original unscaled path.
    uint32_t clear_bound_width = UINT32_MAX;
    uint32_t clear_bound_height = UINT32_MAX;
    for (const RenderTarget* target : {color_target, depth_target}) {
      if (target) {
        clear_bound_width = std::min(clear_bound_width, target->width);
        clear_bound_height = std::min(clear_bound_height, target->height);
      }
    }
    uint32_t clear_x = uint32_t(x0);
    uint32_t clear_y = uint32_t(y0);
    uint32_t clear_width = width;
    uint32_t clear_height = height;
    if (IsScenePass(clear_targets)) {
      scene_scale.ScaleInterval(clear_x, clear_width);
      scene_scale.ScaleInterval(clear_y, clear_height);
    }
    VkClearRect rect = {};
    rect.rect.offset = {int32_t(clear_x), int32_t(clear_y)};
    rect.rect.extent = {
        std::min(clear_width,
                 clear_bound_width - std::min(clear_x, clear_bound_width)),
        std::min(clear_height,
                 clear_bound_height - std::min(clear_y, clear_bound_height))};
    rect.layerCount = 1;

    if (depth_target && depth_target->scene_zcull && rect.rect.extent.width &&
        rect.rect.extent.height) {
      if (!ClearDepthTargetInPass(*depth_target, rect.rect,
                                  depth_clear_value.depth,
                                  depth_clear_value.stencil)) {
        REX_FATAL("[scene-zcull] Fixed scene depth clear failed");
      }
      clear_targets.depth = nullptr;
    }

    if (!clear_targets.Empty() && BeginPass(clear_targets)) {
      c = cmd();
      VkClearAttachment attachments[2] = {};
      uint32_t count = 0;
      if (clear_targets.depth) {
        VkClearAttachment& a = attachments[count++];
        a.aspectMask = HasStencil(depth_target->format)
                           ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                           : VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT);
        a.clearValue.depthStencil = depth_clear_value;
      }
      if (color_target) {
        VkClearAttachment& a = attachments[count++];
        a.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        a.colorAttachment = 0;
        uint64_t value = r[rex::graphics::XE_GPU_REG_RB_COLOR_CLEAR] |
                         (uint64_t(r[rex::graphics::XE_GPU_REG_RB_COLOR_CLEAR_LO]) << 32);
        using F = xenos::ColorRenderTargetFormat;
        switch (F(color_target->key.format)) {
          case F::k_8_8_8_8:
          case F::k_8_8_8_8_GAMMA:
            for (uint32_t j = 0; j < 4; ++j) {
              a.clearValue.color.float32[j] = float((value >> (j * 8)) & 0xFF) / 255.0f;
            }
            break;
          case F::k_2_10_10_10:
          case F::k_2_10_10_10_AS_10_10_10_10:
            for (uint32_t j = 0; j < 3; ++j) {
              a.clearValue.color.float32[j] = float((value >> (j * 10)) & 0x3FF) / 1023.0f;
            }
            a.clearValue.color.float32[3] = float((value >> 30) & 0x3) / 3.0f;
            break;
          case F::k_2_10_10_10_FLOAT:
          case F::k_2_10_10_10_FLOAT_AS_16_16_16_16:
            for (uint32_t j = 0; j < 3; ++j) {
              a.clearValue.color.float32[j] = xenos::Float7e3To32((value >> (j * 10)) & 0x3FF);
            }
            a.clearValue.color.float32[3] = float((value >> 30) & 0x3) / 3.0f;
            break;
          case F::k_16_16_FLOAT:
          case F::k_16_16_16_16_FLOAT:
            for (uint32_t j = 0; j < 4; ++j) {
              a.clearValue.color.float32[j] =
                  rex::xenos_half_to_float(uint16_t((value >> (j * 16)) & 0xFFFF));
            }
            break;
          case F::k_16_16:
          case F::k_16_16_16_16:
            for (uint32_t j = 0; j < 4; ++j) {
              a.clearValue.color.float32[j] = std::max(
                  float(int16_t((value >> (j * 16)) & 0xFFFF)) * (1.0f / 32767.0f), -1.0f);
            }
            break;
          case F::k_32_FLOAT: {
            uint32_t bits = uint32_t(value);
            std::memcpy(&a.clearValue.color.float32[0], &bits, 4);
          } break;
          case F::k_32_32_FLOAT: {
            uint32_t bits[2] = {uint32_t(value), uint32_t(value >> 32)};
            std::memcpy(a.clearValue.color.float32, bits, 8);
          } break;
          default:
            break;
        }
      }
      if (rect.rect.extent.width && rect.rect.extent.height) {
        dfn().vkCmdClearAttachments(c, count, attachments, 1, &rect);
#if COD6_DIAGNOSTICS
        if (count) {
          MarkRenderTargetWritten(clear_targets.depth);
          MarkRenderTargetWritten(color_target);
        }
#endif
      }
      EndPass();
    }
  }

  // The barrier that makes the copies and clears visible is recorded before
  // the next rendering or present, so a run of resolves shares one (each
  // barrier is a full wait for idle on this GPU).
  resolve_barrier_pending = true;
}

void Renderer::Impl::FlushResolveBarrier() {
  if (!resolve_barrier_pending) {
    return;
  }
  resolve_barrier_pending = false;
  VkMemoryBarrier after = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                           VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                           VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_TRANSFER_READ_BIT};
  dfn().vkCmdPipelineBarrier(cmd(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT |
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                 VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                             VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                 VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             0, 1, &after, 0, nullptr, 0, nullptr);
}

// --- Swap ------------------------------------------------------------------------------

void Renderer::Impl::Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer, uint32_t width,
                          uint32_t height) {
  // VdSwap describes the frontbuffer in texture fetch constant 0, swizzle
  // included; the SDK presents from it too (RequestSwapTexture). Without the
  // swizzle, red and blue come out swapped.
  xenos::xe_gpu_texture_fetch_t fetch = regs->GetTextureFetch(0);
  uint32_t address = frontbuffer & 0x1FFFFFFF;
  uint32_t guest_swizzle = xenos::XE_GPU_TEXTURE_SWIZZLE_RGBA;
  if (fetch.type == xenos::FetchConstantType::kTexture && fetch.base_address) {
    address = fetch.base_address << 12;
    guest_swizzle = fetch.swizzle;
  }
  uint32_t source_width = 0, source_height = 0;
  VkImageView source_view =
      textures.GetResolveDestinationView2D(address, guest_swizzle, &source_width, &source_height);
  static uint32_t missing_logged = 0;
  if (!source_view && missing_logged < 8) {
    ++missing_logged;
    REXLOG_WARN("[native] Nothing was resolved to the frontbuffer {:08X} ({}x{})", address, width,
                height);
  }
  static bool swap_logged = false;
  if (source_view && !swap_logged) {
    swap_logged = true;
    REXLOG_INFO("[native] Frontbuffer {:08X} {}x{}, swizzle {:03X}", address, width, height,
                guest_swizzle);
  }
  if (presenter && source_view) {
    width = std::min(width ? width : source_width, source_width);
    height = std::min(height ? height : source_height, source_height);
    rex::system::X_VIDEO_MODE mode;
    rex::kernel::xboxkrnl::VdQueryVideoMode(&mode);
    uint32_t display_width = std::max(1u, uint32_t(mode.display_width));
    uint32_t display_height = std::max(1u, uint32_t(mode.display_height));
    const uint64_t presentation_started = NowNs();
    presenter->RefreshGuestOutput(
        width, height, display_width, display_height,
        [&](rex::ui::Presenter::GuestOutputRefreshContext& context) -> bool {
          auto& vulkan_context = static_cast<
              rex::ui::vulkan::VulkanPresenter::VulkanGuestOutputRefreshContext&>(context);
          BeginRecording();
          EndPass();
          uint32_t slot = uint32_t(submission_number % kSubmissionSlots);
          std::memcpy(ramp_mapping + 256 * sizeof(uint32_t) * slot, gamma_ramp,
                      sizeof(gamma_ramp));
          VkDescriptorImageInfo source_info = {VK_NULL_HANDLE, source_view,
                                               VK_IMAGE_LAYOUT_GENERAL};
          VkDescriptorImageInfo dest_info = {VK_NULL_HANDLE, vulkan_context.image_view(),
                                             VK_IMAGE_LAYOUT_GENERAL};
          VkWriteDescriptorSet writes[2] = {};
          writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          writes[0].dstSet = swap_sets[slot][1];
          writes[0].descriptorCount = 1;
          writes[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
          writes[0].pImageInfo = &source_info;
          writes[1] = writes[0];
          writes[1].dstSet = swap_sets[slot][2];
          writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
          writes[1].pImageInfo = &dest_info;
          dfn().vkUpdateDescriptorSets(vk, 2, writes, 0, nullptr);
          FlushResolveBarrier();
          VkCommandBuffer c = cmd();
          VkImageMemoryBarrier barrier = {};
          barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
          barrier.srcAccessMask =
              vulkan_context.image_ever_written_previously()
                  ? rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalAccessMask
                  : 0;
          barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
          barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
          barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
          barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          barrier.image = vulkan_context.image();
          barrier.subresourceRange = rex::ui::vulkan::util::InitializeSubresourceRange();
          dfn().vkCmdPipelineBarrier(
              c,
              vulkan_context.image_ever_written_previously()
                  ? rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalStageMask
                  : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
          dfn().vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_COMPUTE, swap_pipeline);
          dfn().vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_COMPUTE, swap_pipeline_layout, 0,
                                        3, swap_sets[slot], 0, nullptr);
          uint32_t size[2] = {width, height};
          dfn().vkCmdPushConstants(c, swap_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                   sizeof(size), size);
          dfn().vkCmdDispatch(c, (width + 15) / 16, (height + 7) / 8, 1);
          barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
          barrier.dstAccessMask = rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalAccessMask;
          barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
          barrier.newLayout = rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalLayout;
          dfn().vkCmdPipelineBarrier(
              c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
              rex::ui::vulkan::VulkanPresenter::kGuestOutputInternalStageMask, 0, 0, nullptr, 0,
              nullptr, 1, &barrier);
          // Submitted before returning, so the presenter's own work comes
          // after it on the queue.
          Submit();
          context.SetIs8bpc(true);
          return true;
        });
    stats().presentation_call_ns += NowNs() - presentation_started;
  }
  Submit();
  if (trace_frame && REXCVAR_GET(cod6_debug_dump_swaps)) {
    dump_swaps_left = REXCVAR_GET(cod6_debug_dump_swaps);
  }
  if (dump_swaps_left) {
    --dump_swaps_left;
    if (const ResolveDestination* d = textures.FindResolveDestination(address)) {
      DebugDumpImage(d->image, d->width, d->height, d->format, fmt::format("swap_{:05d}", frame));
    }
  }
  if (!album_queue.empty()) {
    std::string dumped;
    for (const AlbumEntry& e : album_queue) {
      DebugDumpImage(e.image, e.width, e.height, e.format,
                     fmt::format("tex_f{:06d}_{:08X}", frame, e.address));
      dumped += fmt::format(" {:08X} fmt{} {}x{}", e.address, int(e.format), e.width, e.height);
    }
    REXLOG_INFO("[album] frame {}: {} textures of the frame: {}", frame, album_queue.size(), dumped);
    album_queue.clear();
    album_armed = false;
    ++album_frames_done;
  }
  if (trace_frame) {
    for (const auto& dump : trace_texture_dumps) {
      DebugDumpImage(dump.image, dump.width, dump.height, dump.format, dump.name);
      // All mip levels of the first DXT1 512x512 (the noise looked like bad small mips).
      if (dump.format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK && dump.width == 512 && !trace_mips_dumped) {
        trace_mips_dumped = true;
        for (uint32_t level = 1; level <= 9; ++level) {
          DebugDumpImage(dump.image, std::max(512u >> level, 1u), std::max(512u >> level, 1u), dump.format,
                         fmt::format("{}_mip{}", dump.name, level), level);
        }
      }
    }
    trace_texture_dumps.clear();
    // Raw guest memory of the first 512x512 DXT1: 128 KB of base and 64 KB from its mip address.
    const std::string dir = REXCVAR_GET(cod6_debug_capture_dir);
    if (trace_raw_mip_address) {
      REXLOG_INFO("[trace] raw texture: base {:08X} (host {}) mips {:08X} (host {}); membase virtual {} physical {}",
                  trace_raw_base_address,
                  static_cast<const void*>(memory->TranslatePhysical<const uint8_t*>(trace_raw_base_address)),
                  trace_raw_mip_address,
                  static_cast<const void*>(memory->TranslatePhysical<const uint8_t*>(trace_raw_mip_address)),
                  static_cast<const void*>(memory->virtual_membase()),
                  static_cast<const void*>(memory->physical_membase()));
    }
    if (trace_raw_mip_address && !dir.empty()) {
      for (auto [address, size, tag] : {std::tuple{trace_raw_base_address, 0x20000u, "base"},
                                        std::tuple{trace_raw_mip_address, 0x10000u, "mips"}}) {
        if (std::FILE* fp = std::fopen(fmt::format("{}/raw_{}_{:08X}.bin", dir, tag, address).c_str(), "wb")) {
          std::fwrite(memory->TranslatePhysical<const uint8_t*>(address), 1, size, fp);
          std::fclose(fp);
        }
      }
      trace_raw_mip_address = 0;
    }
  }
  // Frame boundary.
  auto now = std::chrono::steady_clock::now();
  if (last_swap_time.time_since_epoch().count()) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_swap_time).count();
    if (ms > 50) {
      RendererStats& st = stats();
      ++st.frames_over_50ms;
      if (ms > 100) ++st.frames_over_100ms;
      st.slow_frame_pipelines += st.pipelines_created - at_last_swap.pipelines_created;
      st.slow_frame_shaders += st.shaders_translated - at_last_swap.shaders_translated;
      st.slow_frame_uploads += textures.stats().uploads - at_last_swap.textures_uploaded;
      st.slow_frame_render_targets +=
          st.render_targets_created - at_last_swap.render_targets_created;
    }
  }
  last_swap_time = now;
  at_last_swap = stats();
  at_last_swap.textures_uploaded = textures.stats().uploads;
  // Save the driver's pipeline cache once new pipelines stop appearing for a
  // while, off this thread: the file is tens of megabytes on the SD card.
  if (!driver_cache_path.empty() && stats().pipelines_created != pipelines_at_last_save &&
      now - last_pipeline_time > std::chrono::seconds(5) && !driver_cache_writing.load()) {
    SaveDriverCache();
  }
  stats().textures_uploaded = textures.stats().uploads;
  stats().resolve_swizzle_changes = textures.stats().resolve_swizzle_changes;
  stats().resolve_answer_checks = textures.stats().resolve_answer_checks;
  stats().resolve_answer_reuses = textures.stats().resolve_answer_reuses;
  stats().resolve_answer_fallbacks = textures.stats().resolve_answer_fallbacks;
  stats().texture_bytes_uploaded = textures.stats().upload_bytes;
  stats().textures_created = textures.stats().created;
  stats().textures_streamed = textures.stats().streamed;
  stats().textures_pending = textures.stats().pending;
  stats().texture_memory_bytes = textures.stats().texture_memory_bytes;
  stats().texture_budget_evictions = textures.stats().budget_evictions;
#if COD6_DIAGNOSTICS
  stats().texture_eviction_checks = textures.stats().eviction_checks;
  stats().texture_eviction_over_budget = textures.stats().eviction_over_budget;
  stats().texture_eviction_entries_scanned = textures.stats().eviction_entries_scanned;
  stats().texture_eviction_eligible = textures.stats().eviction_eligible;
  stats().texture_eviction_sorted = textures.stats().eviction_sorted;
  stats().texture_budget_evicted_bytes = textures.stats().budget_evicted_bytes;
  stats().texture_age_evictions = textures.stats().age_evictions;
  stats().texture_age_evicted_bytes = textures.stats().age_evicted_bytes;
  stats().texture_eviction_scan_ns = textures.stats().eviction_scan_ns;
  stats().texture_eviction_candidate_ns = textures.stats().eviction_candidate_ns;
  stats().texture_eviction_sort_ns = textures.stats().eviction_sort_ns;
  stats().texture_eviction_release_ns = textures.stats().eviction_release_ns;
  stats().texture_eviction_age_scan_ns = textures.stats().eviction_age_scan_ns;
  stats().texture_pool_dedicated = textures.stats().pool_dedicated;
  stats().texture_pool_no_fit = textures.stats().pool_no_fit;
  stats().texture_pool_slabs_failed = textures.stats().pool_slabs_failed;
  for (size_t i = 0; i < 9; ++i) {
    stats().texture_change_paths[i] = textures.stats().change_paths[i];
  }
  for (size_t i = 0; i < 3; ++i) {
    stats().texture_flat_clear_count[i] = textures.stats().flat_clear_count[i];
    stats().texture_flat_clear_entries[i] = textures.stats().flat_clear_entries[i];
    stats().texture_flat_clear_capacity[i] = textures.stats().flat_clear_capacity[i];
    stats().texture_flat_clear_ns[i] = textures.stats().flat_clear_ns[i];
  }
  stats().vertex_change_paths[0] = vertex_change_stats.trusted_skips;
  stats().vertex_change_paths[1] = vertex_change_stats.sample_checks;
  stats().vertex_change_paths[2] = vertex_change_stats.sample_hits;
  stats().vertex_change_paths[3] = vertex_change_stats.full_checks;
  stats().vertex_change_paths[4] = vertex_change_stats.full_initial;
  stats().vertex_change_paths[5] = vertex_change_stats.full_unchanged;
  stats().vertex_change_paths[6] = vertex_change_stats.full_changed;
  stats().vertex_change_paths[7] = vertex_change_stats.sample_input_bytes;
  stats().vertex_change_paths[8] = vertex_change_stats.full_input_bytes;
#endif
  stats().texture_placeholders = textures.stats().placeholders;
  stats().texture_hash_ns = textures.stats().hash_ns;
  stats().texture_create_ns = textures.stats().create_ns;
  stats().texture_creates = textures.stats().creates_on_ring;
  stats().texture_source_snapshot_ns = textures.stats().source_snapshot_ns;
  stats().texture_source_snapshot_bytes = textures.stats().source_snapshot_bytes;
  stats().texture_source_snapshot_jobs = textures.stats().source_snapshot_jobs;
  stats().volume_uploads=textures.stats().volume_uploads;stats().volume_bytes=textures.stats().volume_bytes;
  stats().volume_conversion_ns=textures.stats().volume_conversion_ns;
  stats().volume_plan_ns=textures.stats().volume_plan_ns;stats().volume_retries=textures.stats().volume_retries;
  stats().textures_replaced = textures.stats().replaced;
  stats().frame_upload_wait_ns = frame_upload_wait_ns;
  stats().frame_fence_wait_ns = frame_fence_wait_ns;
  stats().frame_drain_wait_ns = frame_drain_wait_ns;
  stats().frame_texture_ns = frame_texture_ns;
  stats().texture_lookup_ns += frame_texture_ns;
  stats().frame_draws = frame_draw_count;
  stats().frame_stream_wait_ns = frame_stream_wait_ns;
  if(batch_recorder) {
    const auto record=batch_recorder->ReadStats();
    stats().record_queued=record.queued;stats().record_publications=record.publications;
    stats().record_wakes=record.wakes;stats().record_capacity_waits=record.capacity_waits;
    stats().record_drains=record.drains;stats().record_executed=record.executed;
    stats().record_execute_ns=record.execute_ns;stats().record_batches=record.executed_batches;
    stats().record_capacity_wait_ns=record.capacity_wait_ns;stats().record_drain_wait_ns=record.drain_wait_ns;
  }
  frame_stream_wait_ns = 0;
  frame_upload_wait_ns = frame_fence_wait_ns = frame_drain_wait_ns = frame_texture_ns = 0;
  last_frame_draw_count = frame_draw_count;
  frame_draw_count = 0;
  if (trace_frame) {
    TraceEndPass();
    for (const auto& [key, value] : trace_texture_formats) {
      REXLOG_INFO("[trace] textures: {:4} x {} (e.g. {})", value.first, key, value.second);
    }
    trace_texture_formats.clear();
    REXLOG_INFO("[trace] swap, end of frame {}", frame);
  }
  if (REXCVAR_GET(cod6_debug_mip_watch) && frame % 600 == 0) ReportMipWatch();
  ++frame;
  trace_frame = cod6::diagnostics::kEnabled && REXCVAR_GET(cod6_debug_trace_frame) && frame == REXCVAR_GET(cod6_debug_trace_frame);
  {
    static const auto trace_clock_start = std::chrono::steady_clock::now();
    static bool traced_after = false;
    const uint32_t after_s = cod6::diagnostics::kEnabled ? REXCVAR_GET(cod6_debug_trace_after_s) : 0;
    // After the delay, the first frame following one with 1000+ draws: in the map, not on a loading screen.
    if (after_s && !traced_after && last_frame_draw_count >= 1000 &&
        std::chrono::steady_clock::now() - trace_clock_start >= std::chrono::seconds(after_s)) {
      traced_after = true;
      trace_frame = true;
    }
  }
  if (trace_frame) REXLOG_INFO("[trace] frame {} begins", frame);
#if COD6_DIAGNOSTICS
  profile_frame = profile_pool && frame % 16 == 0;
#endif
  PollCompleted();
#if COD6_DIAGNOSTICS
  if (profile_frame) ++profiled_frames;
#endif
  textures.BeginFrame(frame, completed_frame);
  FreeRetiredSetPools();
  for (auto it = retired_render_targets.begin(); it != retired_render_targets.end();) {
    if (it->frame <= completed_frame) {
      if (it->sample_view) dfn().vkDestroyImageView(vk, it->sample_view, nullptr);
      dfn().vkDestroyImageView(vk, it->view, nullptr);
      dfn().vkDestroyImage(vk, it->image, nullptr);
      dfn().vkFreeMemory(vk, it->memory, nullptr);
      it = retired_render_targets.erase(it);
    } else {
      ++it;
    }
  }
}

// --- Driver pipeline cache ---------------------------------------------------------------

namespace {
constexpr uint64_t kDriverCacheMagic = 0x3148434E54414E52ULL;  // "RNATNCH1"

std::vector<uint8_t> ReadDriverCache(const std::filesystem::path& path) {
  std::vector<uint8_t> bytes;
  if (FILE* file = rex::filesystem::OpenFile(path, "rb")) {
    uint64_t header[3] = {};
    if (fread(header, sizeof(header), 1, file) == 1 && header[0] == kDriverCacheMagic &&
        header[1] >= 32 && header[1] <= 128u * 1024 * 1024) {
      bytes.resize(size_t(header[1]));
      if (fread(bytes.data(), bytes.size(), 1, file) != 1 ||
          XXH3_64bits(bytes.data(), bytes.size()) != header[2]) {
        bytes.clear();
      }
    }
    fclose(file);
  }
  return bytes;
}
// SPIR-V cache file: a header, then records appended as shaders are translated. A record that does not check out
// ends the file: the next record written goes over it.
constexpr uint64_t kSpirvCacheMagic = 0x3156505354414E52ULL;  // "RNATSPV1"
// Bump when the translator or the modifications the renderer uses change, so old SPIR-V is not reused.
#ifndef COD6_PACKED_VERTEX_SCALES
#define COD6_PACKED_VERTEX_SCALES 0
#endif
constexpr uint32_t kSpirvCacheVersion =
    1 + COD6_PACKED_VERTEX_SCALES + 2 * COD6_INSTANCE_CONSTANTS;
constexpr uint32_t kSpirvRecordMagic = 0x43455253;  // "SREC"
constexpr size_t kSpirvCacheMaxBytes = 96u * 1024 * 1024;

struct SpirvCacheHeader {
  uint64_t magic;
  uint32_t version;
  uint32_t vendor_id;
  uint32_t device_id;
  uint32_t driver_version;
};

struct SpirvRecordHeader {
  uint32_t magic;
  uint32_t type;
  uint64_t ucode_hash;
  uint64_t modification;
  uint32_t ucode_dwords;
  uint32_t texture_count;
  uint32_t sampler_count;
  uint32_t binary_size;
  uint64_t checksum;  // of the record header up to here, the bindings and the binary
};
static_assert(sizeof(SpirvShader::TextureBinding) == 4 && sizeof(SpirvShader::SamplerBinding) == 4);

uint64_t SpirvCacheKey(uint32_t type, uint64_t ucode_hash, uint64_t modification) {
  const uint64_t key[] = {type, ucode_hash, modification};
  return XXH3_64bits(key, sizeof(key));
}

uint64_t SpirvRecordChecksum(const SpirvRecordHeader& header, const void* payload, size_t size) {
  return XXH3_64bits_withSeed(payload, size,
                              XXH3_64bits(&header, offsetof(SpirvRecordHeader, checksum)));
}
}  // namespace

bool Renderer::Impl::TakeCachedTranslation(VulkanShader::VulkanTranslation* translation) {
  /* COD6_V226C_VACIA: la cache de SPIR-V en disco del proyecto COD5 necesita
   * SpirvShader::SetBindingsFromCache y Translation::SetTranslatedBinary, que
   * son anadidos de su fork del SDK y no existen en el nuestro. Sin ellos no se
   * puede reinstalar una traduccion ya hecha, asi que esta funcion siempre dice
   * "no hay nada en cache" y cada shader se traduce de la forma normal.
   * Funcionalmente identico; solo cuesta mas la primera vez por shader. */
  (void)translation;
  return false;
}

void Renderer::Impl::StoreTranslation(VulkanShader::VulkanTranslation* translation) {
  std::lock_guard<std::mutex> lock(spirv_cache_mutex);
  if (!spirv_cache_file || !translation->is_valid()) {
    return;
  }
  const auto& shader = static_cast<const SpirvShader&>(translation->shader());
  const auto& textures = shader.GetTextureBindingsAfterTranslation();
  const auto& samplers = shader.GetSamplerBindingsAfterTranslation();
  const auto& binary = translation->translated_binary();
  std::vector<uint8_t> payload((textures.size() + samplers.size()) * 4 + binary.size());
  if (!textures.empty()) std::memcpy(payload.data(), textures.data(), textures.size() * 4);
  if (!samplers.empty()) {
    std::memcpy(payload.data() + textures.size() * 4, samplers.data(), samplers.size() * 4);
  }
  if (!binary.empty()) {
    std::memcpy(payload.data() + (textures.size() + samplers.size()) * 4, binary.data(), binary.size());
  }
  SpirvRecordHeader header = {};
  header.magic = kSpirvRecordMagic;
  header.type = uint32_t(shader.type());
  header.ucode_hash = shader.ucode_data_hash();
  header.modification = translation->modification();
  header.ucode_dwords = uint32_t(shader.ucode_dword_count());
  header.texture_count = uint32_t(textures.size());
  header.sampler_count = uint32_t(samplers.size());
  header.binary_size = uint32_t(binary.size());
  header.checksum = SpirvRecordChecksum(header, payload.data(), payload.size());
  bool written = fwrite(&header, sizeof(header), 1, spirv_cache_file) == 1 &&
                 fwrite(payload.data(), payload.size(), 1, spirv_cache_file) == 1 &&
                 fflush(spirv_cache_file) == 0;
  if (!written) {
    // Stop writing; the partial record fails its checksum next time.
    fclose(spirv_cache_file);
    spirv_cache_file = nullptr;
    REXLOG_WARN("[native] SPIR-V cache: write failed, not caching more shaders this session");
  }
}

void Renderer::InitializeSpirvCache(const std::filesystem::path& directory, uint32_t title_id) {
  Impl& im = *impl_;
  const auto& properties = im.device->properties();
  REXLOG_INFO("[native] Shader variants: packed_vertex_scales={} instance_constants={} "
              "(SPIR-V cache v{})",
              bool(COD6_PACKED_VERTEX_SCALES), bool(COD6_INSTANCE_CONSTANTS),
              kSpirvCacheVersion);
  // Keep the established v1 path for existing installations, but never let an
  // opt-in translator experiment truncate it after rejecting the v1 header.
  // Experimental translator modes get their own cache namespace so A/B
  // switching preserves the warm control cache in both directions.
  const std::string cache_variant =
      (COD6_PACKED_VERTEX_SCALES || COD6_INSTANCE_CONSTANTS)
          ? fmt::format("-spv-v{}", kSpirvCacheVersion)
          : std::string();
  auto path = directory / fmt::format("native-{:08X}-{:08X}-{:08X}-{:08X}{}.spirv", title_id,
                                      properties.vendorID, properties.deviceID,
                                      properties.driverVersion, cache_variant);
  const SpirvCacheHeader expected = {kSpirvCacheMagic, kSpirvCacheVersion, properties.vendorID,
                                     properties.deviceID, properties.driverVersion};
  // Records with a checksum that fails are where the valid data ends.
  long valid_end = 0;
  size_t loaded_bytes = 0;
  if (FILE* file = rex::filesystem::OpenFile(path, "r+b")) {
    SpirvCacheHeader header = {};
    if (fread(&header, sizeof(header), 1, file) == 1 &&
        std::memcmp(&header, &expected, sizeof(header)) == 0) {
      valid_end = long(sizeof(header));
      std::vector<uint8_t> payload;
      SpirvRecordHeader record;
      while (loaded_bytes < kSpirvCacheMaxBytes && fread(&record, sizeof(record), 1, file) == 1) {
        if (record.magic != kSpirvRecordMagic || record.binary_size == 0 ||
            record.binary_size > 4u * 1024 * 1024 || record.texture_count > 64 ||
            record.sampler_count > 64) {
          break;
        }
        size_t binding_bytes = size_t(record.texture_count + record.sampler_count) * 4;
        payload.resize(binding_bytes + record.binary_size);
        if (fread(payload.data(), payload.size(), 1, file) != 1 ||
            SpirvRecordChecksum(record, payload.data(), payload.size()) != record.checksum) {
          break;
        }
        Impl::CachedSpirv& entry =
            im.spirv_cache[SpirvCacheKey(record.type, record.ucode_hash, record.modification)];
        entry.type = record.type;
        entry.ucode_hash = record.ucode_hash;
        entry.modification = record.modification;
        entry.ucode_dwords = record.ucode_dwords;
        entry.textures.resize(record.texture_count);
        entry.samplers.resize(record.sampler_count);
        if (record.texture_count) {
          std::memcpy(entry.textures.data(), payload.data(), size_t(record.texture_count) * 4);
        }
        if (record.sampler_count) {
          std::memcpy(entry.samplers.data(), payload.data() + size_t(record.texture_count) * 4,
                      size_t(record.sampler_count) * 4);
        }
        entry.binary.assign(payload.begin() + binding_bytes, payload.end());
        loaded_bytes += payload.size();
        valid_end = ftell(file);
      }
    }
    if (valid_end > 0 && fseek(file, valid_end, SEEK_SET) == 0) {
      im.spirv_cache_file = file;
    } else {
      fclose(file);
    }
  }
  if (!im.spirv_cache_file) {
    // Missing, from another driver or version, or unreadable: start over.
    im.spirv_cache.clear();
    loaded_bytes = 0;
    if (FILE* file = rex::filesystem::OpenFile(path, "wb")) {
      if (fwrite(&expected, sizeof(expected), 1, file) == 1 && fflush(file) == 0) {
        im.spirv_cache_file = file;
      } else {
        fclose(file);
      }
    }
  }
  REXLOG_INFO("[native] SPIR-V cache: {} translations ({} KB) loaded from {}{}", im.spirv_cache.size(),
              loaded_bytes >> 10, path.string(), im.spirv_cache_file ? "" : " (not writable)");
}

void Renderer::Impl::ListPipeline(uint64_t hash, const PipelineKey& key) {
  if (listed_pipelines.insert(hash).second) {
    pipelines_to_list.push_back(key);
  }
}

namespace {
constexpr uint64_t kPipelineListMagic = 0x314C505354414E52ULL;  // "RNATSPL1"
// Bump when what a PipelineKey means changes.
constexpr uint32_t kPipelineListVersion = 1;
struct PipelineListHeader {
  uint64_t magic;
  uint32_t version;
  uint32_t key_size;
  uint32_t vendor_id;
  uint32_t device_id;
  uint32_t driver_version;
  uint32_t reserved;
};
}  // namespace

void Renderer::PrewarmPipelines(const std::filesystem::path& directory, uint32_t title_id) {
  Impl& im = *impl_;
  const auto& properties = im.device->properties();
  auto path = directory / fmt::format("native-{:08X}-{:08X}-{:08X}-{:08X}.pipelines", title_id,
                                      properties.vendorID, properties.deviceID,
                                      properties.driverVersion);
  const PipelineListHeader expected = {kPipelineListMagic, kPipelineListVersion,
                                       uint32_t(sizeof(PipelineKey)), properties.vendorID,
                                       properties.deviceID, properties.driverVersion, 0};
  std::vector<PipelineKey> keys;
  long valid_end = 0;
  if (FILE* file = rex::filesystem::OpenFile(path, "r+b")) {
    PipelineListHeader header = {};
    if (fread(&header, sizeof(header), 1, file) == 1 &&
        std::memcmp(&header, &expected, sizeof(header)) == 0) {
      valid_end = long(sizeof(header));
      PipelineKey key;
      uint64_t checksum;
      while (keys.size() < 8192 && fread(&key, sizeof(key), 1, file) == 1 &&
             fread(&checksum, sizeof(checksum), 1, file) == 1 &&
             XXH3_64bits(&key, sizeof(key)) == checksum) {
        keys.push_back(key);
        valid_end = ftell(file);
      }
    }
    if (valid_end > 0 && fseek(file, valid_end, SEEK_SET) == 0) {
      im.pipeline_list_file = file;
    } else {
      fclose(file);
    }
  }
  if (!im.pipeline_list_file) {
    keys.clear();
    if (FILE* file = rex::filesystem::OpenFile(path, "wb")) {
      if (fwrite(&expected, sizeof(expected), 1, file) == 1 && fflush(file) == 0) {
        im.pipeline_list_file = file;
      } else {
        fclose(file);
      }
    }
  }
  if (!im.pipeline_thread.joinable() || !im.driver_cache) {
    for (const PipelineKey& key : keys) {
      im.listed_pipelines.insert(XXH3_64bits(&key, sizeof(key)));
    }
    return;
  }
  // One module per cached translation used, shared by the pipelines that use it.
  std::unordered_map<uint64_t, VkShaderModule> modules;
  auto module_for = [&](xenos::ShaderType type, uint64_t hash, uint64_t modification) -> VkShaderModule {
    uint64_t cache_key = SpirvCacheKey(uint32_t(type), hash, modification);
    auto made = modules.find(cache_key);
    if (made != modules.end()) return made->second;
    VkShaderModule module = VK_NULL_HANDLE;
    auto entry = im.spirv_cache.find(cache_key);
    if (entry != im.spirv_cache.end() && entry->second.ucode_hash == hash &&
        entry->second.modification == modification && entry->second.type == uint32_t(type)) {
      VkShaderModuleCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      info.codeSize = entry->second.binary.size();
      info.pCode = reinterpret_cast<const uint32_t*>(entry->second.binary.data());
      if (im.dfn().vkCreateShaderModule(im.vk, &info, nullptr, &module) != VK_SUCCESS) {
        module = VK_NULL_HANDLE;
      } else {
        im.prewarm_modules.push_back(module);
      }
    }
    modules.emplace(cache_key, module);
    return module;
  };
  std::deque<Impl::PipelineJob> jobs;
  uint32_t missing = 0;
  for (const PipelineKey& key : keys) {
    uint64_t hash = XXH3_64bits(&key, sizeof(key));
    im.listed_pipelines.insert(hash);
    if (im.pipelines.count(hash) || im.pending_pipelines.count(hash)) continue;
    Impl::PipelineJob job;
    job.hash = hash;
    job.key = key;
    job.prewarm = true;
    job.vertex = module_for(xenos::ShaderType::kVertex, key.vs_hash, key.vs_modification);
    if (key.ps_hash) {
      job.pixel = module_for(xenos::ShaderType::kPixel, key.ps_hash, key.ps_modification);
    }
    const PipelineLayout* layout =
        im.GetPipelineLayout(uint32_t(key.layout >> 48), uint32_t(key.layout >> 32) & 0xFFFF,
                             uint32_t(key.layout >> 16) & 0xFFFF, uint32_t(key.layout) & 0xFFFF);
    job.layout = layout ? layout->layout : VK_NULL_HANDLE;
    if (key.geometry_shader) {
      auto it = im.geometry_shaders.find(key.geometry_shader);
      if (it == im.geometry_shaders.end()) {
        GeometryShaderKey gs_key;
        gs_key.key = key.geometry_shader;
        it = im.geometry_shaders.emplace(key.geometry_shader, CreateGeometryShader(im.device, gs_key)).first;
      }
      job.geometry = it->second;
    }
    if (!job.vertex || (key.ps_hash && !job.pixel) || !job.layout ||
        (key.geometry_shader && !job.geometry)) {
      ++missing;
      continue;
    }
    im.pending_pipelines.insert(hash);
    jobs.push_back(job);
  }
  REXLOG_INFO("[native] Pipeline list: {} known, {} to compile at boot, {} without their shaders cached; {}",
              keys.size(), jobs.size(), missing, path.string());
  if (jobs.empty()) {
    for (VkShaderModule module : im.prewarm_modules) {
      im.dfn().vkDestroyShaderModule(im.vk, module, nullptr);
    }
    im.prewarm_modules.clear();
    return;
  }
  {
    std::lock_guard<std::mutex> lock(im.pipeline_mutex);
    im.prewarm_start = std::chrono::steady_clock::now();
    im.prewarm_remaining = uint32_t(jobs.size());
    for (auto& job : jobs) im.prewarm_jobs.push_back(job);
  }
  im.pipeline_cv.notify_one();
}

void Renderer::InitializeDriverCache(const std::filesystem::path& cache_root,
                                     uint32_t title_id) {
  Impl& im = *impl_;
  if (!im.device) {
    return;
  }
  std::error_code error;
  auto directory = cache_root / "shaders" / "driver";
  std::filesystem::create_directories(directory, error);
  if (error) {
    return;
  }
  InitializeSpirvCache(directory, title_id);
  const auto& properties = im.device->properties();
  im.driver_cache_path =
      directory / fmt::format("native-{:08X}-{:08X}-{:08X}-{:08X}.cache", title_id,
                              properties.vendorID, properties.deviceID, properties.driverVersion);
  // The newest complete file: the rename that replaces it can fail on Horizon
  // (it does not overwrite), leaving the data in .tmp.
  std::vector<uint8_t> data = ReadDriverCache(im.driver_cache_path);
  auto temporary = im.driver_cache_path;
  temporary += ".tmp";
  std::vector<uint8_t> newer = ReadDriverCache(temporary);
  if (newer.size() > data.size()) {
    data = std::move(newer);
  }
  VkPipelineCacheCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  info.initialDataSize = data.size();
  info.pInitialData = data.empty() ? nullptr : data.data();
  VkPipelineCache cache = VK_NULL_HANDLE;
  if (im.dfn().vkCreatePipelineCache(im.vk, &info, nullptr, &cache) != VK_SUCCESS) {
    info.initialDataSize = 0;
    info.pInitialData = nullptr;
    if (im.dfn().vkCreatePipelineCache(im.vk, &info, nullptr, &cache) != VK_SUCCESS) {
      return;
    }
    data.clear();
  }
  // Called before the guest runs, so the ring thread is not using the old one.
  if (im.driver_cache) {
    im.dfn().vkDestroyPipelineCache(im.vk, im.driver_cache, nullptr);
  }
  im.driver_cache = cache;
  im.driver_cache_saved_size = data.size();
  REXLOG_INFO("[native] Driver pipeline cache: {} KB loaded from {}", data.size() >> 10,
              im.driver_cache_path.string());
  PrewarmPipelines(directory, title_id);
}

void Renderer::Impl::SaveDriverCache() {
  pipelines_at_last_save = stats().pipelines_created;
  auto keys = std::make_shared<std::vector<PipelineKey>>(std::move(pipelines_to_list));
  pipelines_to_list.clear();
  size_t size = 0;
  std::shared_ptr<std::vector<uint8_t>> data;
  if (dfn().vkGetPipelineCacheData(vk, driver_cache, &size, nullptr) == VK_SUCCESS && size >= 32 &&
      size != driver_cache_saved_size) {
    data = std::make_shared<std::vector<uint8_t>>(size);
    if (dfn().vkGetPipelineCacheData(vk, driver_cache, &size, data->data()) == VK_SUCCESS) {
      data->resize(size);
      driver_cache_saved_size = size;
    } else {
      data.reset();
    }
  }
  if (!data && keys->empty()) {
    return;
  }
  driver_cache_writing = true;
  std::filesystem::path path = driver_cache_path;
  // Joined, never detached: detaching throws on Horizon, which ended the game.
  if (driver_cache_writer.joinable()) {
    driver_cache_writer.join();
  }
  driver_cache_writer = std::thread([this, data, keys, path]() {
    if (pipeline_list_file && !keys->empty()) {
      bool written = true;
      for (const PipelineKey& key : *keys) {
        const uint64_t checksum = XXH3_64bits(&key, sizeof(key));
        written = written && fwrite(&key, sizeof(key), 1, pipeline_list_file) == 1 &&
                  fwrite(&checksum, sizeof(checksum), 1, pipeline_list_file) == 1;
      }
      if (!written || fflush(pipeline_list_file) != 0) {
        fclose(pipeline_list_file);
        pipeline_list_file = nullptr;
      }
    }
    if (!data) {
      driver_cache_writing = false;
      return;
    }
    auto temporary = path;
    temporary += ".tmp";
    if (FILE* file = rex::filesystem::OpenFile(temporary, "wb")) {
      const uint64_t header[] = {kDriverCacheMagic, uint64_t(data->size()),
                                 XXH3_64bits(data->data(), data->size())};
      bool written = fwrite(header, sizeof(header), 1, file) == 1 &&
                     fwrite(data->data(), data->size(), 1, file) == 1;
      if (fclose(file) != 0) written = false;
      if (written) {
        std::error_code error;
        std::filesystem::remove(path, error);
        std::filesystem::rename(temporary, path, error);
      }
    }
    driver_cache_writing = false;
  });
}

// --- Renderer ---------------------------------------------------------------------------

Renderer::Renderer() : impl_(std::make_unique<Impl>()) { impl_->owner = this; }
Renderer::~Renderer() { Shutdown(); }

bool Renderer::Initialize(rex::ui::vulkan::VulkanProvider* provider, rex::memory::Memory* memory,
                          RegisterFile* regs) {
  impl_->provider = provider;
  impl_->memory = memory;
  impl_->regs = regs;
  if (!impl_->Initialize()) {
    impl_->Shutdown();
    return false;
  }
  return true;
}

#if COD6_DIAGNOSTICS
void Renderer::CaptureReportSnapshot(RendererReportSnapshot& snapshot) {
  if (!impl_) return;
  impl_->textures.TakeUploadReportSnapshot(snapshot.texture_uploads);
  impl_->CaptureProfileReport(snapshot.gpu_profile);
  snapshot.collect_diagnostics = impl_->diagnostics;
  if (!impl_->diagnostics) return;
  CaptureNvkDriverReport(snapshot.nvk_driver);
  std::copy(std::begin(state_change_counts_), std::end(state_change_counts_),
            std::begin(snapshot.state_change_counts));
  std::fill(std::begin(state_change_counts_), std::end(state_change_counts_), 0);
  for (size_t kind = 0; kind < impl_->shadow_diagnostics.size(); ++kind) {
    auto& source = impl_->shadow_diagnostics[kind];
    auto& dest = snapshot.shadows[kind];
    dest.size = source.size();
    dest.total = source.total();
    dest.omitted = source.omitted();
    for (size_t i = 0; i < dest.size; ++i) {
      const auto& in = source.entries()[i];
      std::copy(in.key.begin(), in.key.end(), std::begin(dest.entries[i].key));
      dest.entries[i].count = in.count;
    }
    source.Reset();
  }
}

void Renderer::FormatReportSnapshot(const RendererReportSnapshot& snapshot) {
  if (snapshot.texture_uploads.layout_diagnostics) {
    REXLOG_INFO("[texture-layout] schema=1 enabled={} queries={} hits={} builds={} cumulative=1 point=GetGuestTextureLayout",
                snapshot.texture_uploads.layout_extents, snapshot.texture_uploads.layout_queries,
                snapshot.texture_uploads.layout_hits, snapshot.texture_uploads.layout_builds);
  }
  std::string uploads = TextureCache::FormatUploadReport(snapshot.texture_uploads);
  if (!uploads.empty()) REXLOG_INFO("[native] most uploaded textures: {}", uploads);
  FormatGpuProfileReport(snapshot.gpu_profile);
  if (!snapshot.collect_diagnostics) return;
  FormatNvkDriverReport(snapshot.nvk_driver);

  static constexpr const char* names[] = {"depth-draw", "no-target-skip",
                                         "depth-copy-attempt", "depth-candidate-binding"};
  for (size_t kind = 0; kind < std::size(snapshot.shadows); ++kind) {
    const auto& group = snapshot.shadows[kind];
    std::array<RendererReportSnapshot::ShadowEntry, 16> entries;
    std::copy(group.entries, group.entries + group.size, entries.begin());
    std::sort(entries.begin(), entries.begin() + group.size,
              [](const auto& a, const auto& b) { return a.count > b.count; });
    REXLOG_INFO("[native-diag] shadow {}: {} events, {} states, {} events omitted by state limit",
                names[kind], group.total, group.size, group.omitted);
    for (size_t i = 0; i < std::min<size_t>(group.size, 3); ++i) {
      const auto& e = entries[i];
      const auto& k = e.key;
      REXLOG_INFO("[native-diag] shadow {} x{} VS {:016X} PS {:016X} surface {:08X} "
                  "depth {:08X} control {:08X} mask {:08X} a {:016X} b {:016X} c {:016X} mode {:08X}",
                  names[kind], e.count, k[0], k[1], k[2], k[3], k[4], k[5], k[6], k[7], k[8], k[9]);
    }
  }

  std::vector<std::pair<uint32_t, uint32_t>> top;
  for (uint32_t i = 0; i < 0x4000; ++i) {
    if (snapshot.state_change_counts[i]) top.emplace_back(snapshot.state_change_counts[i], i);
  }
  std::sort(top.rbegin(), top.rend());
  std::string list;
  for (size_t i = 0; i < std::min<size_t>(top.size(), 12); ++i)
    list += fmt::format(" {:04X}:{}", top[i].second, top[i].first);
  REXLOG_INFO("[native] state changes by register (report interval):{}", list);
}
#endif

void Renderer::Flush() {
  if (impl_) impl_->Submit();
}

void Renderer::KickRecorder() {
  if (impl_) impl_->KickRecorder();
}

void Renderer::AdoptRecorderProducer() {
  if(impl_ && impl_->batch_recorder)impl_->batch_recorder->AdoptProducer();
}

void Renderer::Shutdown() {
  if (impl_) {
    impl_->Shutdown();
  }
}

void Renderer::LoadShader(xenos::ShaderType type, const uint32_t* ucode, uint32_t dword_count,
                          uint32_t guest_address) {
  // COD5 correctness bring-up: verify all microcode on every load. The imported
  // once-per-frame/head-only shortcut assumes the title never replaces shader
  // bodies within a frame; that has not been established for our COD5 image.
  // Keep stage in the object-cache key as identical code has different meaning
  // when translated as a vertex or pixel shader.
  uint64_t location = 0;
  uint64_t sample = 0;
  if (guest_address) {
    location = (uint64_t(guest_address) << 20) | (uint64_t(dword_count) << 2) | uint64_t(type);
    auto known = impl_->shaders_by_location.find(location);
    sample = XXH3_64bits(ucode, dword_count * sizeof(uint32_t));
    if (known != impl_->shaders_by_location.end() && known->second.sample == sample) {
      if (type == xenos::ShaderType::kVertex) {
        impl_->active_vs = known->second.shader;
      } else {
        impl_->active_ps = known->second.shader;
      }
      return;
    }
  }
  uint64_t hash = guest_address ? sample : XXH3_64bits(ucode, dword_count * sizeof(uint32_t));
  auto& shaders = impl_->shaders;
  const auto object_key = std::make_pair(hash, uint32_t(type));
  auto it = shaders.find(object_key);
  VulkanShader* shader;
  if (it != shaders.end()) {
    shader = it->second.get();
  } else {
    auto created = std::make_unique<VulkanShader>(impl_->device, type, hash, ucode, dword_count);
    shader = created.get();
    shaders.emplace(object_key, std::move(created));
  }
  if (type == xenos::ShaderType::kVertex) {
    impl_->active_vs = shader;
  } else {
    impl_->active_ps = shader;
  }
  if (location) {
    impl_->shaders_by_location[location] = {shader, sample};
  }
}

namespace {
struct CpuTimer {
  uint64_t& total;
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  explicit CpuTimer(uint64_t& t) : total(t) {}
  ~CpuTimer() {
    total += uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count());
  }
};
}  // namespace

void Renderer::Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words) {
  CpuTimer timer(stats_.cpu_ns);
  impl_->Draw(immediate_indices, immediate_index_words);
}

void Renderer::Resolve() {
  CpuTimer timer(stats_.cpu_ns);
  impl_->Resolve();
  impl_->DebugDumpResolve();
}

void Renderer::GuestMemorySynchronized() {
  ++stats_.guest_cache_syncs;
  if(cod6::writes::Enabled()) {
    // An acknowledged coherency action may publish an unhooked writer. Never
    // treat missing page marks as proof that this publication changed nothing.
    cod6::writes::g_pages.InvalidateAll();
  }
  if (!impl_->vertex_sync_cache) return;
  // All acknowledged guest coherency actions are conservatively treated as
  // vertex/index publication points. Range units/action bits are not guessed.
  // Do not flush persistent copies, wait on the GPU, or change recorded draws.
  ++impl_->vertex_memory_epoch;
  impl_->fetch_resolved_frame.fill(UINT64_MAX);
}

void Renderer::Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer, uint32_t width,
                    uint32_t height) {
  CpuTimer timer(stats_.cpu_ns);
  impl_->Swap(presenter, frontbuffer, width, height);
}

void Renderer::SetGammaRamp256(const uint32_t* entries) {
  std::memcpy(impl_->gamma_ramp, entries, sizeof(impl_->gamma_ramp));
}

}  // namespace cod6::native


namespace cod6::native {
uint32_t rex_ps_hit_count() { return g_ps_hit_count; }
uint64_t rex_ps_hash(uint32_t i) { return i < g_ps_hit_count ? g_ps_hits[i].hash : 0; }
uint64_t rex_ps_draws(uint32_t i) { return i < g_ps_hit_count ? g_ps_hits[i].draws : 0; }
uint64_t rex_ps_blended(uint32_t i) { return i < g_ps_hit_count ? g_ps_hits[i].blended : 0; }
}  // namespace cod6::native
