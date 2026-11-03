// COD6 - procesador del backend nativo: recorrido del tramo consumido por fotograma.

#include "cod6_native_cp.h"

#include <rex/kernel.h>
#include <rex/logging.h>
#include <rex/switch_log.h>
#include <rex/system/kernel_state.h>

#include "cod6_native_system.h"

namespace cod6::native {

NativeCommandProcessor::NativeCommandProcessor(
    rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
    rex::system::KernelState* kernel_state)
    : Cod6CommandProcessor(graphics_system, kernel_state) {
  REX_SWITCH_LOG("[cod6] NATIVO/CP: procesador con lectror de anillo propio instalado\n");
}

NativeCommandProcessor::~NativeCommandProcessor() = default;

void NativeCommandProcessor::WorkerThreadMain() {
  /* COD6_BUCLE115: punto de entrada del bucle controlado por nosotros. */
  REX_SWITCH_LOG("[cod6] NATIVO/BUCLE: WorkerThreadMain propio activo (v115, envuelve al SDK)\n");
  rex::graphics::vulkan::VulkanCommandProcessor::WorkerThreadMain();
  REX_SWITCH_LOG("[cod6] NATIVO/BUCLE: bucle terminado\n");
}

void NativeCommandProcessor::IssueSwap(uint32_t frontbuffer_ptr, uint32_t frontbuffer_width,
                                       uint32_t frontbuffer_height) {
  /*
   * Tramo a recorrer: desde donde quedo el ultimo intercambio hasta el puntero de
   * lectura actual, o sea justo lo que la emulacion acaba de consumir.
   */
  /* COD6_ANILLO111: la geometria la tiene el propio procesador; el juego nunca
   * llamo a InitializeRingBuffer, asi que la version anterior leia ceros. */
  const uint32_t base = primary_buffer_ptr_;
  const uint32_t bytes = primary_buffer_size_;
  const uint32_t read_bytes = read_ptr_index_ * uint32_t(sizeof(uint32_t));

  if (base != 0 && bytes >= 8 && memory_ != nullptr && read_bytes != last_read_) {
    // Solo se recorre el avance normal (sin dar la vuelta) en esta fase: es el caso
    // comun y no necesita correccion de mascara.
    if (read_bytes > last_read_ && (read_bytes - last_read_) <= bytes) {
      walked_bytes_ += read_bytes - last_read_;
      reader_.Walk(memory_, base, bytes, last_read_, read_bytes);
    }
    last_read_ = read_bytes;
    ++swaps_;
    if ((swaps_ % 200) == 0) {
      REX_SWITCH_LOG("[cod6] NATIVO/LECTOR: swaps=%llu recorrido=%llu B | %s\n",
                     (unsigned long long)swaps_, (unsigned long long)walked_bytes_,
                     reader_.Summary().c_str());
    }
  }

  Cod6CommandProcessor::IssueSwap(frontbuffer_ptr, frontbuffer_width, frontbuffer_height);
}

}  // namespace cod6::native
