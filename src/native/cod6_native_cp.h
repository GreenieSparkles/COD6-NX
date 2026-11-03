// COD6 - procesador de comandos del backend nativo (fase 1, paso 2).
//
// Hereda del procesador que ya usa el proyecto (que a su vez es el de la emulacion)
// y solo ANADE una cosa: en cada intercambio de fotograma recorre con nuestro
// RingReader el tramo del anillo que la emulacion acaba de consumir y compara los
// totales con los suyos.
//
// Por que asi y no en medio del bucle de paquetes:
//   - el tramo ya consumido esta mapeado (la emulacion acaba de leerlo), asi que no
//     se provoca la tormenta de fallos de paginas que tumbo las versiones anteriores;
//   - si el recorrido se equivoca, no puede atascar el hilo de GPU: ya termino su
//     trabajo y el peor caso es una cuenta distinta en el log.
//
// Solo se instala en modo "nativo"; el camino por defecto no lo usa nunca.

#pragma once

#include <cstdint>

#include "../cod6_renderizador.h"
#include "cod6_native_reader.h"

namespace cod6::native {

class NativeCommandProcessor final : public cod6::Cod6CommandProcessor {
 public:
  NativeCommandProcessor(rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
                         rex::system::KernelState* kernel_state);
  ~NativeCommandProcessor() override;

  void IssueSwap(uint32_t frontbuffer_ptr, uint32_t frontbuffer_width,
                 uint32_t frontbuffer_height) override;

  /* COD6_BUCLE115: el bucle de anillo pasa a ser nuestro. La version 115 solo lo
   * envuelve para anunciarse; la 116 lo sustituye por el recorrido propio. */
  void WorkerThreadMain() override;

 private:
  RingReader reader_;
  uint32_t last_read_ = 0;
  uint64_t swaps_ = 0;
  uint64_t walked_bytes_ = 0;
};

}  // namespace cod6::native
