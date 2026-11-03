// cod6 - the native renderer seam, plus step N1: a fast lane for the constant
// registers. See cod6_renderizador.h for why the seam exists and what the next
// steps are.

#include "cod6_renderizador.h"

#include "cod6_nativo.h"

#include "native/cod6_native_system.h"
#include "native/cod6_native_selftest.h"
#include "native/cod6_native_destinos.h"
#include "native/cod6_native_reader.h"
/* COD6_V227_NATIVO2: sistema grafico nativo completo, portado de COD5. */
#include "native/nativo/cod6_native_system_nativo.h"

#include <atomic>
#include <string>

#include <rex/switch_log.h>

namespace cod6 {

// "xenos" (default) keeps the SDK's emulation plugin. "nativo" installs the
// classes below instead. Init-only: the renderer cannot change once running.
REXCVAR_DEFINE_STRING(cod6_renderizador, "xenos", "COD6",
                      "Renderizador: \"xenos\" = emulacion de la GPU (como hasta ahora), \"nativo\" = el "
                      "sistema grafico de este proyecto (en obras; ver cod6_renderizador.h)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

bool Cod6QuiereRenderizadorNativo() {
  /* COD6_V227_NATIVO2: "nativo" es el esqueleto propio de siempre; "nativo2" es
   * el sistema grafico nativo completo portado de COD5 (Reader propio + renderer
   * de 18.400 lineas). Se dejan los dos valores para poder comparar. */
  const std::string& v = REXCVAR_GET(cod6_renderizador);
  return v == "nativo" || v == "nativo2";
}

namespace {

/*
 * N1 - via rapida de las constantes.
 *
 * La tanda 89 midio, dentro de la emulacion y en un lote de 120 buffers
 * primarios (unos 5,7 s de tiempo real):
 *
 *     reg   (WriteRegister)           48-74 ms     ~1 %
 *     coh   (MakeCoherent)            18-23 ms     ~0,3 %
 *     const (SET_CONSTANT* + rangos)  1727-3028 ms ~40-50 %
 *
 * Los bloques de constantes son almacenamiento plano: ALU (0x4000-0x47FF),
 * FETCH (0x4800-0x48FF) y BOOL/LOOP (0x4900-0x490F). Ninguno de ellos es uno de
 * los registros especiales del switch de WriteRegister (estado de D3D, control
 * de la GPU, gamma ramp), asi que por registro la emulacion solo hace una
 * comprobacion de limites, un almacenamiento volatile y un switch de 20.000
 * casos que siempre cae en vacio.
 *
 * Aqui se sustituye ese camino solo para esos bloques: una comprobacion por
 * paquete en vez de una por registro, y almacenamientos directos despues. Todo
 * lo que caiga fuera de los bloques, y cualquier paquete que cruce el final de
 * un bloque, sigue por el codigo original, de modo que los registros con efectos
 * laterales no pueden cambiar de comportamiento.
 */
constexpr uint32_t kPlanoIni = 0x4000;  // constantes ALU
constexpr uint32_t kPlanoFin = 0x4910;  // fin de BOOL/LOOP (exclusivo)

std::atomic<uint64_t> g_registros_rapidos{0};
std::atomic<uint64_t> g_registros_lentos{0};
std::atomic<uint64_t> g_lotes_rapidos{0};

}  // namespace

Cod6CommandProcessor::Cod6CommandProcessor(
    rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
    rex::system::KernelState* kernel_state)
    : rex::graphics::vulkan::VulkanCommandProcessor(graphics_system, kernel_state) {
  REX_SWITCH_LOG("[cod6] RENDERIZADOR nativo: procesador de comandos propio creado\n");
}

Cod6CommandProcessor::~Cod6CommandProcessor() = default;

void Cod6CommandProcessor::WriteRegistersFromMem(uint32_t start_index, uint32_t* base,
                                                 uint32_t num_registers) {
  if (!num_registers) {
    return;
  }

  const uint32_t fin = start_index + num_registers;
  const bool todo_plano = start_index >= kPlanoIni && fin <= kPlanoFin;

  if (!todo_plano) {
    g_registros_lentos.fetch_add(num_registers, std::memory_order_relaxed);
    rex::graphics::vulkan::VulkanCommandProcessor::WriteRegistersFromMem(start_index, base,
                                                                        num_registers);
    return;
  }

  /*
   * register_file_ es protected en CommandProcessor, asi que se puede usar
   * desde aqui. El almacenamiento volatile es el mismo que hace WriteRegister
   * ("volatile for the WAIT_REG_MEM loop"), para que el compilador no lo elimine
   * ni lo reordene respecto al hilo de la GPU que lo observa.
   */
  auto& valores = register_file_->values;
  for (uint32_t i = 0; i < num_registers; ++i) {
    const uint32_t dato = rex::memory::load_and_swap<uint32_t>(base + i);
    const_cast<volatile uint32_t&>(valores[start_index + i]) = dato;
  }

  g_registros_rapidos.fetch_add(num_registers, std::memory_order_relaxed);
  const uint64_t lotes = g_lotes_rapidos.fetch_add(1, std::memory_order_relaxed) + 1;
  if ((lotes % 2048) == 0) {
    REX_SWITCH_LOG("[cod6] via rapida constantes: lotes=%llu registros=%llu (lentos=%llu)\n",
                   (unsigned long long)lotes,
                   (unsigned long long)g_registros_rapidos.load(std::memory_order_relaxed),
                   (unsigned long long)g_registros_lentos.load(std::memory_order_relaxed));
  }
}

Cod6GraphicsSystem::Cod6GraphicsSystem() {
  REX_SWITCH_LOG("[cod6] RENDERIZADOR nativo: sistema grafico propio creado\n");

}

Cod6GraphicsSystem::~Cod6GraphicsSystem() = default;

std::string Cod6GraphicsSystem::name() const {
  return "cod6-nativo";
}

std::unique_ptr<rex::graphics::CommandProcessor> Cod6GraphicsSystem::CreateCommandProcessor() {
  /*
   * NATIVO_N1: el camino propio empieza aqui. Cod6NativoCommandProcessor hereda
   * de VulkanCommandProcessor y, de momento, solo se diferencia en que cuenta
   * (y opcionalmente omite) los dibujos; es el sitio donde iran el lector del
   * anillo y los dibujos propios, sin la vigilancia de paginas que en Switch
   * convierte cada lectura de textura en una excepcion.
   */
  return std::make_unique<Cod6NativoCommandProcessor>(this, kernel_state_);
}

void Cod6CommandProcessor::IssueSwap(uint32_t frontbuffer_ptr, uint32_t frontbuffer_width,
                                     uint32_t frontbuffer_height) {
  /* COD6_SELFTEST111: una sola vez, ya con el log funcionando. */
  static bool probado = false;
  if (!probado) {
    probado = true;
    const cod6::native::SelfTestResult prueba = cod6::native::RunSelfTest();
    REX_SWITCH_LOG("[cod6] LECTOR111: self-test %s\n",
                   cod6::native::FormatSelfTest(prueba).c_str());
    REX_SWITCH_LOG("[cod6] LECTOR111: anillo del procesador base=0x%08X bytes=%u\n",
                   (unsigned)primary_buffer_ptr_, (unsigned)primary_buffer_size_);
  }
  /* COD6_FUERA118: el recorrido del anillo va en CADA swap, no solo en el primero. */
  {
    static cod6::native::RingReader lector118;
    static uint32_t ultima118 = 0;
    const uint32_t base = primary_buffer_ptr_;
    const uint32_t bytes = primary_buffer_size_;
    const uint32_t lectura = read_ptr_index_ * uint32_t(sizeof(uint32_t));
    /* COD6_VUELTA121: recorrer tambien cuando el puntero da la vuelta al anillo. */
    if (base != 0 && bytes >= 8 && memory_ != nullptr && lectura != ultima118) {
      if (lectura > ultima118) {
        lector118.Walk(memory_, base, bytes, ultima118, lectura);
      } else {
        // del puntero viejo al final del anillo, y del principio al nuevo
        lector118.Walk(memory_, base, bytes, ultima118, bytes);
        lector118.Walk(memory_, base, bytes, 0, lectura);
      }
      ultima118 = lectura;
      static cod6::native::Counters previo120;
      static uint64_t vueltas120 = 0;
      ++vueltas120;
      /* COD6_PORFOTOGRAMA120: diferencia contra el informe anterior = lo de esta ventana. */
      if ((vueltas120 % 200) == 0) {
        const cod6::native::Counters& ahora = lector118.counters();
        REX_SWITCH_LOG(
            "[cod6] LECTOR120: vuelta=%llu lectura=%u | ventana: paquetes=%llu dwords=%llu "
            "dibujos=%llu shaders=%llu registros=%llu esperas=%llu otros=%llu\n",
            (unsigned long long)vueltas120, (unsigned)lectura,
            (unsigned long long)(ahora.packets - previo120.packets),
            (unsigned long long)(ahora.dwords - previo120.dwords),
            (unsigned long long)(ahora.draws - previo120.draws),
            (unsigned long long)(ahora.shader_loads - previo120.shader_loads),
            (unsigned long long)(ahora.register_writes - previo120.register_writes),
            (unsigned long long)(ahora.waits - previo120.waits),
            (unsigned long long)(ahora.unknown - previo120.unknown));
        REX_SWITCH_LOG(
            "[cod6] REPARTO123: idx3=%llu idx6=%llu idx<=32=%llu idx<=128=%llu grandes=%llu\n",
            (unsigned long long)(ahora.dibujos_3 - previo120.dibujos_3),
            (unsigned long long)(ahora.dibujos_6 - previo120.dibujos_6),
            (unsigned long long)(ahora.dibujos_32 - previo120.dibujos_32),
            (unsigned long long)(ahora.dibujos_128 - previo120.dibujos_128),
            (unsigned long long)(ahora.dibujos_grandes - previo120.dibujos_grandes));
        previo120 = ahora;
      }
    }
  }

  rex::graphics::vulkan::VulkanCommandProcessor::IssueSwap(frontbuffer_ptr, frontbuffer_width,
                                                           frontbuffer_height);
}

void Cod6CommandProcessor::WorkerThreadMain() {
  /* COD6_BUCLE117 */
  REX_SWITCH_LOG("[cod6] BUCLE117: bucle de anillo propio activo\n");
  rex::graphics::vulkan::VulkanCommandProcessor::WorkerThreadMain();
  REX_SWITCH_LOG("[cod6] BUCLE117: bucle terminado\n");
}

void Cod6GraphicsSystem::InitializeRingBuffer(uint32_t ptr, uint32_t size_log2) {
  /* COD6_ANILLO106 */
  const uint32_t bytes = uint32_t(1) << (size_log2 + 3);
  REX_SWITCH_LOG("[cod6] ANILLO: base=0x%08X bytes=%u (log2=%u) dwords=%u\n", (unsigned)ptr,
                 (unsigned)bytes, (unsigned)size_log2, (unsigned)(bytes / 4));
  rex::graphics::vulkan::VulkanGraphicsSystem::InitializeRingBuffer(ptr, size_log2);
}

void Cod6GraphicsSystem::EnableReadPointerWriteBack(uint32_t ptr, uint32_t block_size_log2) {
  /* COD6_ANILLO106 */
  REX_SWITCH_LOG("[cod6] ANILLO: puntero de lectura -> 0x%08X (bloque log2=%u)\n", (unsigned)ptr,
                 (unsigned)block_size_log2);
  rex::graphics::vulkan::VulkanGraphicsSystem::EnableReadPointerWriteBack(ptr, block_size_log2);
}

std::unique_ptr<rex::system::IGraphicsSystem> Cod6CrearGraphicsSystem() {
  /* COD6_BACKEND104: interruptor de backend. "xenos" (por defecto) es el camino de
   * siempre; "nativo" construye el sistema del backend propio, que de momento se
   * comporta igual y solo anuncia su fase por el log. */
  if (Cod6QuiereRenderizadorNativo()) {
    if (REXCVAR_GET(cod6_renderizador) == "nativo2") {
      /* COD6_V227_NATIVO2: el sistema completo. Sustituye por entero al del SDK,
       * como en COD5: el juego sigue hablando con su Direct3D y su anillo PM4
       * como si fuera una GPU Xbox 360, pero quien lo consume y dibuja es el
       * renderer nativo. */
      REX_SWITCH_LOG("[cod6] BACKEND nativo2: sistema grafico completo (puerto COD5)\n");
      return cod6::native::CreateGraphicsSystem();
    }
    return std::make_unique<Cod6NativeGraphicsSystem>();
  }
  return std::make_unique<Cod6GraphicsSystem>();
}

}  // namespace cod6
