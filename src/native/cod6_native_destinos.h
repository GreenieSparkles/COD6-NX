// COD6 - fase B: gestion de destinos de render propios.
//
// Por que existe: la via rapida de la v137 dejo la pantalla en negro porque se
// saltaba la preparacion de la emulacion, que incluye ENTRAR en el render target.
// Para poder emitir dibujos sin ella hace falta tener nuestros propios destinos y
// nuestra propia entrada en el pass (BeginPass).
//
// Este fichero solo define la clave de destino y su cache. Todavia NO se usa para
// dibujar: primero se valida que la clave identifica bien los destinos reales.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cod6::native {

// Identidad de un destino de render: direccion de EDRAM + formato + tamano.
struct DestinoKey {
  uint32_t edram_base = 0;
  uint32_t formato = 0;
  uint32_t ancho = 0;
  uint32_t alto = 0;

  bool operator==(const DestinoKey& otro) const {
    return edram_base == otro.edram_base && formato == otro.formato && ancho == otro.ancho &&
           alto == otro.alto;
  }
};

struct DestinoInfo {
  DestinoKey clave;
  uint64_t usos = 0;
  uint64_t resolves = 0;
  uint64_t first_seen_swap = 0;
  uint64_t last_seen_swap = 0;
};

// Cache de destinos vistos. Solo cuenta y agrupa: no reserva nada de GPU.
class DestinoCache {
 public:
  // Devuelve el indice del destino (creandolo si es nuevo, hasta kMaximo).
  uint32_t Registrar(const DestinoKey& clave, uint64_t swap);
  void AnotarResolve(uint32_t indice);

  size_t tamano() const { return destinos_.size(); }
  const std::vector<DestinoInfo>& destinos() const { return destinos_; }
  uint64_t total_resolves() const { return total_resolves_; }

  static constexpr uint32_t kMaximo = 32;

 private:
  std::vector<DestinoInfo> destinos_;
  uint64_t total_resolves_ = 0;
};

// Auto-prueba: comprueba el comportamiento de la cache (sin GPU, sin invitado).
std::string AutoPruebaDestinos();

}  // namespace cod6::native
