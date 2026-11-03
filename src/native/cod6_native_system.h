// COD6 - backend nativo: esqueleto del sistema grafico propio.
//
// REGLA DE ORO: este backend NO toca el camino de emulacion. Se elige con
//   cod6_renderizador = "nativo"
// y, mientras este en obras, se comporta exactamente igual que el actual.
//
// Fase 1 (esta version): enterarse de la geometria del anillo directamente del
// juego (base, tamano, puntero de escritura), que es el primer dato que necesita
// un lector propio. Solo se registra en el log: no se lee memoria ni se dibuja.

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "../cod6_renderizador.h"

namespace cod6 {

class Cod6NativeGraphicsSystem final : public Cod6GraphicsSystem {
 public:
  Cod6NativeGraphicsSystem();
  ~Cod6NativeGraphicsSystem() override;

  std::string name() const override;
  std::unique_ptr<rex::graphics::CommandProcessor> CreateCommandProcessor() override;

  // El juego avisa aqui de donde esta el anillo (xboxkrnl Vd*).
  void InitializeRingBuffer(uint32_t ptr, uint32_t size_log2) override;
  void EnableReadPointerWriteBack(uint32_t ptr, uint32_t block_size_log2) override;

  // Datos del anillo tal como los dio el juego (0 = aun no lo ha dicho).
  static uint32_t AnilloBase();
  static uint32_t AnilloBytes();
  static uint32_t PunteroEscritura();

 private:
  static std::atomic<uint32_t> anillo_base_;
  static std::atomic<uint32_t> anillo_bytes_;
  static std::atomic<uint32_t> puntero_escritura_;
  static std::atomic<uint32_t> rptr_direccion_;
};

}  // namespace cod6
