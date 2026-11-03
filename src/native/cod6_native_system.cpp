// COD6 - backend nativo: implementacion del esqueleto (fase 1).
//
// Lo unico que hace de momento: apuntar la geometria del anillo que el juego
// declara (base, tamano, direccion del puntero de lectura/escritura) y sacarla por
// el log. Nada de leer memoria todavia y nada de dibujar: el camino de comandos
// sigue siendo el de la emulacion, asi que el comportamiento no cambia.

#include "cod6_native_system.h"

#include "cod6_native_renderer.h"
#include "cod6_native_selftest.h"
#include "cod6_native_cp.h"

#include <rex/switch_log.h>

namespace cod6 {

std::atomic<uint32_t> Cod6NativeGraphicsSystem::anillo_base_{0};
std::atomic<uint32_t> Cod6NativeGraphicsSystem::anillo_bytes_{0};
std::atomic<uint32_t> Cod6NativeGraphicsSystem::puntero_escritura_{0};
std::atomic<uint32_t> Cod6NativeGraphicsSystem::rptr_direccion_{0};

Cod6NativeGraphicsSystem::Cod6NativeGraphicsSystem() {
  REX_SWITCH_LOG("[cod6] NATIVO: backend propio seleccionado (cod6_renderizador = nativo)\n");
  REX_SWITCH_LOG(
      "[cod6] NATIVO: fase 1 - se apunta la geometria del anillo que declara el juego. "
      "El camino de comandos sigue siendo el de la emulacion.\n");
  REX_SWITCH_LOG("[cod6] NATIVO: renderer propio %s\n", NativeRenderer::Estado().c_str());
  /* Fase 1: comprobar que el recorrido de paquetes es correcto. */
  {
    const native::SelfTestResult prueba = native::RunSelfTest();
    REX_SWITCH_LOG("[cod6] NATIVO/LECTOR: self-test %s\n",
                   native::FormatSelfTest(prueba).c_str());
  }
}

Cod6NativeGraphicsSystem::~Cod6NativeGraphicsSystem() = default;

std::string Cod6NativeGraphicsSystem::name() const {
  return "cod6-native-backend";
}

std::unique_ptr<rex::graphics::CommandProcessor> Cod6NativeGraphicsSystem::CreateCommandProcessor() {
  /* Fase A paso 2: el procesador del backend recorre el anillo con nuestro lector.
   * El dibujo sigue siendo el de la emulacion, asi que la imagen no cambia. */
  return std::make_unique<cod6::native::NativeCommandProcessor>(this, kernel_state_);
}

void Cod6NativeGraphicsSystem::InitializeRingBuffer(uint32_t ptr, uint32_t size_log2) {
  const uint32_t bytes = uint32_t(1) << (size_log2 + 3);
  anillo_base_.store(ptr, std::memory_order_relaxed);
  anillo_bytes_.store(bytes, std::memory_order_relaxed);
  REX_SWITCH_LOG("[cod6] NATIVO: anillo base=0x%08X bytes=%u (log2=%u) dwords=%u\n",
                 (unsigned)ptr, (unsigned)bytes, (unsigned)size_log2, (unsigned)(bytes / 4));
  // La emulacion tambien lo necesita: se le pasa igual.
  Cod6GraphicsSystem::InitializeRingBuffer(ptr, size_log2);
}

void Cod6NativeGraphicsSystem::EnableReadPointerWriteBack(uint32_t ptr, uint32_t block_size_log2) {
  rptr_direccion_.store(ptr, std::memory_order_relaxed);
  REX_SWITCH_LOG("[cod6] NATIVO: puntero de lectura se escribe en 0x%08X (bloque log2=%u)\n",
                 (unsigned)ptr, (unsigned)block_size_log2);
  Cod6GraphicsSystem::EnableReadPointerWriteBack(ptr, block_size_log2);
}

uint32_t Cod6NativeGraphicsSystem::AnilloBase() {
  return anillo_base_.load(std::memory_order_relaxed);
}
uint32_t Cod6NativeGraphicsSystem::AnilloBytes() {
  return anillo_bytes_.load(std::memory_order_relaxed);
}
uint32_t Cod6NativeGraphicsSystem::PunteroEscritura() {
  return puntero_escritura_.load(std::memory_order_relaxed);
}

}  // namespace cod6
