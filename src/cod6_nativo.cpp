// cod6 - native renderer, N1: our own command processor (see cod6_nativo.h).

#include "cod6_nativo.h"

#include "native/cod6_native_reader.h"

#include <rex/graphics/vulkan/deferred_command_buffer.h>
#include <rex/graphics/vulkan/nativo_hooks.h>

#include <rex/cvar.h>
#include <rex/switch_log.h>

namespace cod6 {

/*
 * N1 switch.
 *
 * false (default): our processor behaves exactly like the emulation. Useful to
 *                  confirm the seam runs without changing anything.
 * true:            draws are counted and skipped. The picture disappears, which
 *                  is expected - the point is to prove that the draw path is
 *                  ours and that the game keeps running (ring advances, no
 *                  hang, audio and input alive).
 */
REXCVAR_DEFINE_BOOL(cod6_nativo_sin_dibujos, false, "COD6",
                    "N1: el procesador propio cuenta y omite los dibujos (para probar el camino)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

/*
 * N2 switch: build the draw inventory.
 *
 * Every draw that reaches our processor is recorded with the data needed to
 * reproduce it (shaders, index count, primitive type). Once per second a
 * summary goes to the log: how many draws, how many distinct shader pairs, the
 * index count distribution. That list is what the next step (own pipelines and
 * vkCmdDraw) will consume.
 *
 * It does not change the picture: the emulation still draws.
 */
REXCVAR_DEFINE_BOOL(cod6_nativo_inventario, true, "COD6",
                    "N2: registrar el inventario de dibujos (que shaders, cuantos indices, que primitiva)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

Cod6NativoCommandProcessor::Cod6NativoCommandProcessor(
    rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
    rex::system::KernelState* kernel_state)
    : rex::graphics::vulkan::VulkanCommandProcessor(graphics_system, kernel_state) {
  sin_dibujos_ = REXCVAR_GET(cod6_nativo_sin_dibujos);
  inventario_ = REXCVAR_GET(cod6_nativo_inventario);
  REX_SWITCH_LOG("[cod6] NATIVO N1: procesador propio activo (sin_dibujos=%d)\n",
                 sin_dibujos_ ? 1 : 0);
  /*
   * Deja constancia al arrancar de si el envio propio esta activo. La tanda 121
   * no lo dejo claro: el log no tenia ninguna linea "Config:" y por tanto no se
   * sabia si el cod6.toml con cod6_nativo_envio_propio = true llego a la consola.
   * Con esta linea, un solo arranque lo responde.
   */
  REX_SWITCH_LOG("[cod6] NATIVO ENVIO activo=%d (cod6_nativo_envio_propio)\n",
                 rex::graphics::vulkan::NativeDrawSubmissionEnabled() ? 1 : 0);
}

Cod6NativoCommandProcessor::~Cod6NativoCommandProcessor() {
  REX_SWITCH_LOG("[cod6] NATIVO N1: dibujos vistos=%llu copias=%llu\n",
                 (unsigned long long)dibujos_contados_.load(),
                 (unsigned long long)copias_contadas_.load());
}

bool Cod6NativoCommandProcessor::IssueDraw(rex::graphics::xenos::PrimitiveType prim_type, uint32_t index_count,
                                           rex::graphics::CommandProcessor::IndexBufferInfo* index_buffer_info,
                                           bool major_mode_explicit) {
  const uint64_t n = dibujos_contados_.fetch_add(1, std::memory_order_relaxed) + 1;

  /*
   * N3-1 - LEER LOS VERTICES COMO LO HACE EL PORT DE MW1.
   *
   * Tres intentos anteriores fallaron por la misma razon, ahora identificada en
   * el codigo del port de referencia (nfsmw_nativo_dibujos.cpp):
   *
   *     const uint8_t* datos = memoria_->TranslatePhysical(base & 0x1FFFFFFF);
   *     const uint8_t* vertices = memoria_->TranslatePhysical(direccion * 4);
   *
   * Es decir:
   *   1. la direccion del fetch constante esta en DWORDs -> hay que multiplicar
   *      por 4 para tener bytes (direccion * 4);
   *   2. y luego hay que enmascarar con 0x1FFFFFFF para quedarse con la parte
   *      fisica, porque las direcciones del juego viven en la ventana
   *      0x80000000 + fisica (0x80400000 & 0x1FFFFFFF = 0x00400000);
   *   3. se traduce con TranslatePhysical, no con TranslateVirtual.
   *
   * Lo que hice mal antes: v18/v19 usaron TranslatePhysical SIN la mascara (leia
   * ceros) y v20 uso TranslateVirtual CON la direccion sin enmascarar (excepcion
   * no manejada y pantalla negra).
   *
   * Se leen los indices y los primeros 8 dwords del buffer de vertices. Si los
   * vertices salen distintos de cero, ya se puede pasar a dibujar.
   */
  if (inventario_ && index_count <= 8 && index_buffer_info != nullptr) {
    static uint32_t muestras = 0;
    if (muestras < 6) {
      ++muestras;
      const auto vfetch = register_file_->GetVertexFetch(0);
      uint32_t idx[6] = {0, 0, 0, 0, 0, 0};
      uint32_t vb[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      bool ok_i = false, ok_v = false;

      if (memory_ != nullptr) {
        /*
         * NATIVO SLOTS: el slot 0 de fetch lee ceros, asi que los vertices de
         * estos quads no estan ahi. Se recorren los 16 slots y se informa de
         * cual tiene datos distintos de cero: ese es el buffer real.
         */
        for (uint32_t slot = 0; slot < 16; ++slot) {
          const auto vf = register_file_->GetVertexFetch(slot);
          if (vf.size == 0) continue;
          const uint32_t fis = (vf.address << 2) & 0x1FFFFFFFu;
          const uint8_t* q = memory_->TranslatePhysical(fis);
          if (q == nullptr) continue;
          uint32_t d0 = uint32_t(q[0]) | (uint32_t(q[1]) << 8) | (uint32_t(q[2]) << 16) |
                        (uint32_t(q[3]) << 24);
          uint32_t d1 = uint32_t(q[4]) | (uint32_t(q[5]) << 8) | (uint32_t(q[6]) << 16) |
                        (uint32_t(q[7]) << 24);
          if (d0 != 0 || d1 != 0) {
            REX_SWITCH_LOG(
                "[cod6] NATIVO SLOTS: slot=%u addr=0x%08X->0x%08X size=%u d0=%08X d1=%08X\n",
                (unsigned)slot, (unsigned)(vf.address << 2), (unsigned)fis, (unsigned)vf.size,
                d0, d1);
          }
        }

        const uint32_t ib_fis = index_buffer_info->guest_base & 0x1FFFFFFFu;
        const uint8_t* p = memory_->TranslatePhysical(ib_fis);
        if (p != nullptr) {
          for (uint32_t k = 0; k < 6; ++k) {
            idx[k] = uint32_t(p[k * 2]) | (uint32_t(p[k * 2 + 1]) << 8);
          }
          ok_i = true;
        }
        const uint32_t vb_fis = (vfetch.address << 2) & 0x1FFFFFFFu;
        const uint8_t* q = memory_->TranslatePhysical(vb_fis);
        if (q != nullptr) {
          for (uint32_t k = 0; k < 8; ++k) {
            vb[k] = uint32_t(q[k * 4]) | (uint32_t(q[k * 4 + 1]) << 8) |
                    (uint32_t(q[k * 4 + 2]) << 16) | (uint32_t(q[k * 4 + 3]) << 24);
          }
          ok_v = true;
        }
        REX_SWITCH_LOG(
            "[cod6] NATIVO QUAD3: n=%u ib=0x%08X->0x%08X vb=0x%08X->0x%08X | "
            "idx=[%u %u %u %u %u %u] vb=[%08X %08X %08X %08X %08X %08X %08X %08X] "
            "ok_i=%d ok_v=%d\n",
            (unsigned)index_count, (unsigned)index_buffer_info->guest_base, (unsigned)ib_fis,
            (unsigned)(vfetch.address << 2), (unsigned)vb_fis,
            (unsigned)idx[0], (unsigned)idx[1], (unsigned)idx[2], (unsigned)idx[3],
            (unsigned)idx[4], (unsigned)idx[5],
            vb[0], vb[1], vb[2], vb[3], vb[4], vb[5], vb[6], vb[7],
            ok_i ? 1 : 0, ok_v ? 1 : 0);
      }
    }
  }


  /*
   * N3a - LEER LA GEOMETRIA DE LOS QUADS DE UI.
   *
   * El inventario de la tanda 109 dijo que en el menu el 92,1 % de los dibujos
   * tienen <= 8 indices, y que solo se usan 5 sombreadores de vertices. Eso es un
   * objetivo pequeno y cerrado: si somos capaces de leer bien esos quads, dibujarlos
   * nosotros es lo siguiente.
   *
   * Este paso NO dibuja: comprueba que podemos sacar la geometria del juego.
   * Para cada quad se apunta de donde vienen los indices y los primeros vertices
   * (direccion y formato del buffer de vertices), y cada 600 dibujos se imprime una
   * muestra. Si las coordenadas salen en el rango de la pantalla (0..1280, 0..720),
   * la via de datos es correcta y se puede pasar a emitir vkCmdDraw.
   */
  if (index_count <= 8 && index_buffer_info != nullptr) {
    static uint32_t muestras = 0;
    if (muestras < 6) {
      ++muestras;
      /* Primer buffer de vertices declarado por el juego (fetch constante 0). */
      const auto vfetch = register_file_->GetVertexFetch(0);
      /*
       * LECTURA DE VERTICES RETIRADA (tanda 114).
       *
       * La version 20 leia la memoria del invitado con TranslateVirtual(address<<2)
       * y luego con punteros crudos. Resultado:
       *
       *     Unhandled guest access violation: read of guest 0x80400002
       *
       * o sea que TranslateVirtual devolvio una direccion no valida y el proceso
       * murio al leerla: pantalla negra sin menu. No se vuelve a tocar memoria del
       * invitado desde el camino del dibujo; si hace falta leer vertices, se hara
       * con shared_memory_->RequestRange + el buffer ya mapeado, que es la via que
       * usa el propio SDK.
       *
       * Se deja solo el registro de donde estan los datos, que es informacion
       * util y no arriesga nada.
       */
      REX_SWITCH_LOG("[cod6] NATIVO QUAD: n=%u fmt=%u idx@0x%08X dwords_vb=0x%08X size=%u\n",
          (unsigned)index_count, (unsigned)index_buffer_info->format,
          (unsigned)index_buffer_info->guest_base, (unsigned)vfetch.address,
          (unsigned)vfetch.size);    }
  }

  /*
   * N2: apuntar el dibujo. Los hashes de shader se piden al procesador padre,
   * que ya los tiene resueltos para este dibujo.
   */
  if (inventario_) {
    DibujoRegistrado d;
    /* Los hashes de shader no son accesibles desde aqui (son locales de
       IssueDraw en la clase base); se dejan a 0 y el inventario se apoya en
       codigo de primitiva, numero de indices y el tipo de primitiva, que es lo
       que hace falta para decidir por donde empezar a dibujar. Los pares de
       shader ya se ven en el log de creacion de canalizaciones. */
    d.vs_hash = 0;
    d.ps_hash = 0;
    d.indices = index_count;
    d.prim_type = uint32_t(prim_type);
    d.opcode = uint32_t(prim_type);
    dibujos_frame_.push_back(d);
    ++dibujos_totales_inv_;

    if ((n % 600) == 0) {
      /* Resumen del tramo: cuantos dibujos, cuantos pares de shader distintos,
         y el reparto de tamanos (asi se ve si son quads de UI o geometria). */
      uint32_t pequenos = 0, medianos = 0, grandes = 0;
      for (const DibujoRegistrado& r : dibujos_frame_) {
        if (r.indices <= 8) ++pequenos;
        else if (r.indices <= 256) ++medianos;
        else ++grandes;
      }
      REX_SWITCH_LOG("[cod6] NATIVO INVENTARIO: dibujos=%zu  <=8=%u  <=256=%u  >256=%u\n",
                     dibujos_frame_.size(), pequenos, medianos, grandes);
      dibujos_frame_.clear();
    }
  }
  if ((n % 4096) == 0) {
    REX_SWITCH_LOG("[cod6] NATIVO N1: dibujos=%llu (indices=%u)\n", (unsigned long long)n,
                   (unsigned)index_count);
  }
  if (sin_dibujos_) {
    /* Ring keeps advancing, the game does not notice. */
    return true;
  }
  return rex::graphics::vulkan::VulkanCommandProcessor::IssueDraw(prim_type, index_count,
                                                                 index_buffer_info,
                                                                 major_mode_explicit);
}

bool Cod6NativoCommandProcessor::IssueCopy() {
  copias_contadas_.fetch_add(1, std::memory_order_relaxed);
  if (sin_dibujos_) {
    return true;
  }
  return rex::graphics::vulkan::VulkanCommandProcessor::IssueCopy();
}

}  // namespace cod6

namespace rex::graphics::vulkan {

/*
 * NATIVO_HOOK - implementacion del lado del proyecto.
 *
 * El SDK llama aqui desde IssueDraw para cada dibujo pequeno (cuadros de UI),
 * con los datos que ya tiene calculados: cuantos indices, cuantos vertices, el
 * tipo de buffer de indices y su handle si esta convertido al host.
 *
 * Hoy solo cuenta y publica un resumen: sirve para saber cuanto del tiempo de
 * fotograma son estos dibujos, que es lo que el camino nativo va a sustituir.
 * No toca el dibujo actual.
 */
namespace {
std::atomic<uint64_t> g_dibujos_pequenos{0};
std::atomic<uint64_t> g_vertices_pequenos{0};
std::atomic<uint64_t> g_con_handle{0};
}  // namespace

void NativeReportDraw(const NativeDrawInfo& info) {
  const uint64_t n = g_dibujos_pequenos.fetch_add(1, std::memory_order_relaxed) + 1;
  g_vertices_pequenos.fetch_add(info.host_vertex_count, std::memory_order_relaxed);
  if (info.index_buffer_handle != 0) {
    g_con_handle.fetch_add(1, std::memory_order_relaxed);
  }
  if ((n % 2000) == 0) {
    REX_SWITCH_LOG(
        "[cod6] NATIVO DIBUJO: pequenos=%llu vertices=%llu con_handle=%llu (ultimo: n=%u "
        "v=%u tipo=%u prim=%u handle=%llu)\n",
        (unsigned long long)n, (unsigned long long)g_vertices_pequenos.load(),
        (unsigned long long)g_con_handle.load(), (unsigned)info.index_count,
        (unsigned)info.host_vertex_count, (unsigned)info.buffer_type,
        (unsigned)info.primitive, (unsigned long long)info.index_buffer_handle);
  }
}

}  // namespace rex::graphics::vulkan

namespace rex::graphics::vulkan {

/* N3-3: emitir nosotros el vkCmdDrawIndexed de los dibujos pequenos. */
/*
 * kAnyChange en vez de kInitOnly: la tanda 121 llevaba cod6_nativo_envio_propio
 * = true en cod6.toml y el envio propio no se activo ni una vez. Si el valor del
 * archivo se aplica despues de la inicializacion, una cvar kInitOnly se queda con
 * el valor por defecto (false) para siempre. Con kHotReload se lee siempre el
 * valor vigente, que es lo que hace falta para un interruptor de pruebas.
 */
REXCVAR_DEFINE_BOOL(cod6_nativo_envio_propio, false, "COD6",
                    "N3-3: emitir desde el camino nativo el CmdVkDraw de los dibujos pequenos")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

/* COD6_ENVIO_FORZADO (temporal): la tarjeta no puede cambiar el toml por
 * netloader, asi que para esta prueba el envio propio va siempre activo.
 * Se revierte despues del experimento. */
bool NativeDrawSubmissionEnabled() {
#if defined(COD6_ENVIO_FORZADO)
  return true;
#else
  return REXCVAR_GET(cod6_nativo_envio_propio);
#endif
}

void NativeSubmitDrawIndexed(void* cb, uint32_t index_count) {
  static std::atomic<uint64_t> enviados{0};
  const uint64_t n = enviados.fetch_add(1, std::memory_order_relaxed) + 1;
  if ((n % 2000) == 0) {
    REX_SWITCH_LOG("[cod6] NATIVO ENVIO: dibujos=%llu (ultimo n=%u)\n", (unsigned long long)n,
                   (unsigned)index_count);
  }
  if (cb == nullptr) return;
  static_cast<DeferredCommandBuffer*>(cb)->CmdVkDrawIndexed(index_count, 1, 0, 0, 0);
}

}  // namespace rex::graphics::vulkan

namespace rex::graphics::vulkan {

void NativeSubmitDraw(void* cb, uint32_t vertex_count) {
  static std::atomic<uint64_t> enviados{0};
  const uint64_t n = enviados.fetch_add(1, std::memory_order_relaxed) + 1;
  if ((n % 2000) == 0) {
    REX_SWITCH_LOG("[cod6] NATIVO ENVIO: dibujos=%llu (sin indice, v=%u)\n",
                   (unsigned long long)n, (unsigned)vertex_count);
  }
  if (cb == nullptr) return;
  auto* dcb = static_cast<DeferredCommandBuffer*>(cb);

  /* COD6_ENVIO_FORZADO: primera vez, dejar constancia del estado real. */
  {
    static bool avisado = false;
    if (!avisado) {
      avisado = true;
      REX_SWITCH_LOG("[cod6] NATIVO SUBMIT #1: pipeline_propia=0x%llX verts=%u\n",
                     (unsigned long long)NativeDebugPipeline(), (unsigned)vertex_count);
    }
  }

  /*
   * N3-4c: si nuestra canalizacion magenta existe, se enlaza antes de dibujar.
   * Es el primer dibujo emitido con una pipeline PROPIA (shaders propios): si el
   * bloque magenta aparece en pantalla, el camino nativo dibuja de verdad.
   *
   * Los estados dinamicos (viewport, scissor) los deja puestos el SDK antes de
   * llegar aqui, asi que no hace falta fijarlos otra vez.
   */
  const VkPipeline propia = NativeDebugPipeline();
  if (propia != VK_NULL_HANDLE) {
    /*
     * N3-4f: la tanda 132 seguia sin mostrar nada. El motivo mas probable es que
     * el SDK fija viewport y scissor para CADA dibujo (estados dinamicos), y el
     * scissor de un cuadro de UI pequeno recorta nuestro triangulo de pantalla
     * completa hasta dejarlo en nada. Antes de dibujar se fijan viewport y
     * scissor completos; el SDK los volvera a fijar en su siguiente dibujo.
     */
    VkViewport vp_full{};
    vp_full.x = 0.0f;
    vp_full.y = 0.0f;
    vp_full.width = 1280.0f;
    vp_full.height = 720.0f;
    vp_full.minDepth = 0.0f;
    vp_full.maxDepth = 1.0f;
    dcb->CmdVkSetViewport(0, 1, &vp_full);
    VkRect2D sc_full{};
    sc_full.offset.x = 0;
    sc_full.offset.y = 0;
    sc_full.extent.width = 1280;
    sc_full.extent.height = 720;
    dcb->CmdVkSetScissor(0, 1, &sc_full);
    dcb->CmdVkBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, propia);
  }
  /*
   * N3-4e: si se enlaza nuestra canalizacion, se dibujan 3 vertices (el VS
   * genera un triangulo de pantalla completa). Si no, se mantiene el
   * comportamiento anterior para no alterar el cuadro.
   */
  dcb->CmdVkDraw(propia != VK_NULL_HANDLE ? 3u : vertex_count, 1, 0, 0);
}

}  // namespace rex::graphics::vulkan


// COD6_BUCLE116 ---------------------------------------------------------------
namespace {
cod6::native::RingReader g_lector116;
uint64_t g_bucles116 = 0;
uint64_t g_swaps116 = 0;
uint32_t g_ultima_lectura116 = 0;
}  // namespace

void cod6::Cod6NativoCommandProcessor::WorkerThreadMain() {
  ++g_bucles116;
  REX_SWITCH_LOG("[cod6] BUCLE116: bucle de anillo propio activo (llamada %llu)\n",
                 (unsigned long long)g_bucles116);
  rex::graphics::vulkan::VulkanCommandProcessor::WorkerThreadMain();
  REX_SWITCH_LOG("[cod6] BUCLE116: bucle terminado\n");
}

void cod6::Cod6NativoCommandProcessor::IssueSwap(uint32_t frontbuffer_ptr,
                                                 uint32_t frontbuffer_width,
                                                 uint32_t frontbuffer_height) {
  const uint32_t base = primary_buffer_ptr_;
  const uint32_t bytes = primary_buffer_size_;
  const uint32_t lectura = read_ptr_index_ * uint32_t(sizeof(uint32_t));
  if (base != 0 && bytes >= 8 && memory_ != nullptr && lectura != g_ultima_lectura116 &&
      lectura > g_ultima_lectura116 && (lectura - g_ultima_lectura116) <= bytes) {
    g_lector116.Walk(memory_, base, bytes, g_ultima_lectura116, lectura);
    g_ultima_lectura116 = lectura;
    if ((++g_swaps116 % 200) == 0) {
      REX_SWITCH_LOG("[cod6] LECTOR116: swaps=%llu | %s\n", (unsigned long long)g_swaps116,
                     g_lector116.Summary().c_str());
    }
  }
  rex::graphics::vulkan::VulkanCommandProcessor::IssueSwap(frontbuffer_ptr, frontbuffer_width,
                                                           frontbuffer_height);
}


