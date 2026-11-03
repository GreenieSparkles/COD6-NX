// cod6 - native renderer, N1: our own command processor, first real code.
//
// Where this goes
//   The emulation draws through generic Xenos state translation. Measured: the
//   static part of a draw is already cheap (0.09 ms), but everything that reads
//   guest memory - textures above all - costs an exception per 4 bytes, because
//   Switch cannot set page permissions (svcSetProcessMemoryPermission = 0xD401)
//   and the runtime has to unmap instead. A texture upload measured 8 seconds.
//
//   The native renderer does not need that watch at all: it reads the game's PM4
//   ring, decides itself when to re-read a texture, and uploads it. So the plan
//   is, in this order:
//
//     N1  this file: our processor owns the draw path (counts and skips for now,
//         selected with cod6_nativo_sin_dibujos = true). Proves the seam works
//         end to end with our code in the hot path.
//     N2  read the ring ourselves and build the command inventory: which opcodes,
//         with which arguments, in which order (the counts we have are only
//         per-opcode totals).
//     N3  draw with our own pipelines, without shared-memory watch.
//     N4  upload textures with our own reader.
//     N5  present.
//
// Every step is a switch in cod6.toml, and "xenos" always remains available.

#pragma once

#include <atomic>
#include <vector>
#include <cstdint>

#include <rex/graphics/xenos.h>
#include <rex/graphics/vulkan/command_processor.h>

namespace cod6 {

class Cod6NativoCommandProcessor : public rex::graphics::vulkan::VulkanCommandProcessor {
  /* COD6_BUCLE116: estos dos SI se ejecutan (comprobado en el log del 199). */
  void WorkerThreadMain() override;
  void IssueSwap(uint32_t frontbuffer_ptr, uint32_t frontbuffer_width,
                 uint32_t frontbuffer_height) override;

 public:
  Cod6NativoCommandProcessor(rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
                             rex::system::KernelState* kernel_state);
  ~Cod6NativoCommandProcessor() override;

  /*
   * N2 - el inventario de dibujos.
   *
   * Lo que pide native-renderer.md: leer lo que el juego escribe para la GPU y
   * quedarse con los dibujos, sin imitar nada. Aqui ya estamos en el camino del
   * dibujo (N1), asi que solo hace falta apuntar los datos que hacen falta para
   * reproducirlo: que shaders, cuantos indices, que primitiva, a que destino de
   * render y con que viewport.
   *
   * Todavia NO se dibuja nada propio: esto es la lista de la compra. Cuando el
   * inventario salga estable, el paso siguiente es crear las canalizaciones y
   * emitir los vkCmdDraw con estos mismos datos.
   */
  struct DibujoRegistrado {
    uint64_t vs_hash;
    uint64_t ps_hash;
    uint32_t indices;
    uint32_t prim_type;
    uint32_t opcode;   /* tipo de primitiva como lo ve el juego */
  };

  // N1: take the draw path. With the switch off, the emulation draws as always.
  bool IssueDraw(rex::graphics::xenos::PrimitiveType prim_type, uint32_t index_count,
                 rex::graphics::CommandProcessor::IndexBufferInfo* index_buffer_info, bool major_mode_explicit) override;
  bool IssueCopy() override;

 private:
  bool sin_dibujos_ = false;
  bool inventario_ = false;
  std::vector<DibujoRegistrado> dibujos_frame_;
  uint64_t dibujos_totales_inv_ = 0;
  std::atomic<uint64_t> dibujos_contados_{0};
  std::atomic<uint64_t> copias_contadas_{0};
};

}  // namespace cod6
