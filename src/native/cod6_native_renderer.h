// COD6 - renderer nativo: esqueleto con las MISMAS entradas que el renderer de COD5.
//
// Esto es deliberadamente una copia de la forma, no del contenido: cuando se porte
// native_renderer.cpp de COD5, cada funcion de aqui se rellenara con su equivalente.
// Las firmas siguen las suyas (LoadShader / Draw / Resolve / Swap / GuestMemorySynchronized)
// para que el port sea mecanico y revisable.
//
// Ninguna de estas funciones se llama todavia: el esqueleto solo compila y responde
// al estado por el log. Ver src/native/README.md.

#pragma once

#include <cstdint>
#include <string>

namespace rex::memory {
class Memory;
}
namespace rex::ui {
class Presenter;
}

namespace cod6 {

class NativeRenderer {
 public:
  NativeRenderer();
  ~NativeRenderer();

  // Ciclo de vida.
  bool Initialize(rex::memory::Memory* memory);
  void Shutdown();

  // Entradas del bucle (mismas que COD5).
  void LoadShader(uint32_t type, const uint32_t* ucode, uint32_t dword_count);
  void Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words);
  void Resolve();
  void Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer, uint32_t width, uint32_t height);
  void GuestMemorySynchronized();

  // Estado legible para el log / la documentacion.
  static std::string Estado();

 private:
  rex::memory::Memory* memory_ = nullptr;
  bool initialised_ = false;
  uint64_t draws_ = 0;
  uint64_t resolves_ = 0;
  uint64_t swaps_ = 0;
  uint64_t shaders_ = 0;
};

}  // namespace cod6
