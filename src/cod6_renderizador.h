// cod6 - renderer selection: the native path (work in progress).
//
// What this is
//   The reference port (nfsmw-nx, which is also what makes MW1/COD4 run on the
//   Switch: see its codmw.toml, nfsmw_renderizador = "nativo") does not use the
//   Xenos emulation to draw. It installs its own graphics system and draws the
//   game's command ring with Vulkan.
//
//   This project takes the same road in small steps. The seam is ReXApp's
//   RuntimeConfig::graphics: when it is empty the SDK loads the plugin named by
//   gpu_plugin (rexgpu-xenos, the emulation). When we fill it ourselves, our
//   class is used instead and we decide what the command processor does.
//
//   Step 1 (this file, today): a graphics system and a command processor that
//   are still the emulation, but ours. Nothing is drawn differently yet - the
//   point is that every draw and every copy already goes through a class we can
//   change, and the log says which renderer is active. The next steps replace
//   IssueDraw / IssueCopy with a leaner implementation, which is where the 0.75
//   ms per draw and the 98% of time inside the GPU emulation have to come down.
//
//   It is off by default: cod6.toml has to ask for it with
//       cod6_renderizador = "nativo"
//   so a build with an unfinished renderer never takes the running one's place.

#pragma once

#include <memory>
#include <string>

#include <rex/cvar.h>
#include <rex/graphics/command_processor.h>
#include <rex/graphics/vulkan/command_processor.h>
#include <rex/graphics/vulkan/graphics_system.h>
#include <rex/system/kernel_state.h>

namespace cod6 {

// The command processor we own. For now it is the emulation (it inherits
// VulkanCommandProcessor and overrides nothing), which is exactly what makes it
// useful: the next step overrides IssueDraw and IssueCopy here.
class Cod6CommandProcessor : public rex::graphics::vulkan::VulkanCommandProcessor {
 public:
  Cod6CommandProcessor(rex::graphics::vulkan::VulkanGraphicsSystem* graphics_system,
                       rex::system::KernelState* kernel_state);
  ~Cod6CommandProcessor() override;

    /*
     * N1: los bloques de constantes (ALU/FETCH/BOOL/LOOP) toman una via rapida.
     * Ver cod6_renderizador.cpp: son el 40-50 % del tiempo dentro de la
     * emulacion y no tienen efectos laterales, asi que no hace falta pasar
     * por el switch de 20.000 registros de WriteRegister.
     */
    void WriteRegistersFromMem(uint32_t start_index, uint32_t* base,
                               uint32_t num_registers) override;

    /* COD6_SELFTEST111: primera comprobacion del lector de paquetes, ya con log
     * abierto (en el constructor todavia no existe y las lineas se pierden). */
    void IssueSwap(uint32_t frontbuffer_ptr, uint32_t frontbuffer_width,
                   uint32_t frontbuffer_height) override;

    /* COD6_BUCLE117: el bucle de anillo y el recorrido propio, en la clase que corre. */
    void WorkerThreadMain() override;

  // Draws, copies and presentation tuning land here in the next steps.
};

// The graphics system we own: the Vulkan one, with our command processor.
class Cod6GraphicsSystem : public rex::graphics::vulkan::VulkanGraphicsSystem {
 public:
  Cod6GraphicsSystem();
  ~Cod6GraphicsSystem() override;

  std::string name() const override;

  /* COD6_ANILLO106: apuntar la geometria del anillo que declara el juego. Solo
   * registra numeros en el log (no lee memoria), asi que no cambia nada. */
  void InitializeRingBuffer(uint32_t ptr, uint32_t size_log2) override;
  void EnableReadPointerWriteBack(uint32_t ptr, uint32_t block_size_log2) override;

 protected:
  std::unique_ptr<rex::graphics::CommandProcessor> CreateCommandProcessor() override;
};

// True when cod6.toml asked for the native renderer.
bool Cod6QuiereRenderizadorNativo();

// Builds it (the caller stores it in RuntimeConfig::graphics).
std::unique_ptr<rex::system::IGraphicsSystem> Cod6CrearGraphicsSystem();

}  // namespace cod6
