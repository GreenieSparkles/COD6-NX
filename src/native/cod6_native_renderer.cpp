// COD6 - renderer nativo: implementacion del esqueleto (sin dibujar todavia).

#include "cod6_native_renderer.h"

#include <rex/switch_log.h>

namespace cod6 {

NativeRenderer::NativeRenderer() = default;
NativeRenderer::~NativeRenderer() = default;

bool NativeRenderer::Initialize(rex::memory::Memory* memory) {
  memory_ = memory;
  initialised_ = true;
  REX_SWITCH_LOG("[cod6] NATIVO/RENDERER: Initialize (esqueleto, aun no dibuja)\n");
  return true;
}

void NativeRenderer::Shutdown() {
  initialised_ = false;
  memory_ = nullptr;
}

void NativeRenderer::LoadShader(uint32_t type, const uint32_t* ucode, uint32_t dword_count) {
  (void)type;
  (void)ucode;
  (void)dword_count;
  ++shaders_;
}

void NativeRenderer::Draw(const uint32_t* immediate_indices, uint32_t immediate_index_words) {
  (void)immediate_indices;
  (void)immediate_index_words;
  ++draws_;
}

void NativeRenderer::Resolve() { ++resolves_; }

void NativeRenderer::Swap(rex::ui::Presenter* presenter, uint32_t frontbuffer, uint32_t width,
                          uint32_t height) {
  (void)presenter;
  (void)frontbuffer;
  (void)width;
  (void)height;
  ++swaps_;
}

void NativeRenderer::GuestMemorySynchronized() {}

std::string NativeRenderer::Estado() {
  return "esqueleto listo (0 lineas portadas de COD5 todavia)";
}

}  // namespace cod6
