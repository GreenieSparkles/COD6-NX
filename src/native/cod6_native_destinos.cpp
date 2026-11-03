// COD6 - fase B: implementacion de la cache de destinos + auto-prueba.

#include "cod6_native_destinos.h"

#include <cstdio>

namespace cod6::native {

uint32_t DestinoCache::Registrar(const DestinoKey& clave, uint64_t swap) {
  for (uint32_t i = 0; i < destinos_.size(); ++i) {
    if (destinos_[i].clave == clave) {
      ++destinos_[i].usos;
      destinos_[i].last_seen_swap = swap;
      return i;
    }
  }
  if (destinos_.size() >= kMaximo) {
    return kMaximo;  // sin sitio: el llamante debe caer al camino normal
  }
  DestinoInfo info;
  info.clave = clave;
  info.usos = 1;
  info.first_seen_swap = swap;
  info.last_seen_swap = swap;
  destinos_.push_back(info);
  return uint32_t(destinos_.size() - 1);
}

void DestinoCache::AnotarResolve(uint32_t indice) {
  if (indice < destinos_.size()) {
    ++destinos_[indice].resolves;
  }
  ++total_resolves_;
}

std::string AutoPruebaDestinos() {
  DestinoCache cache;
  DestinoKey a{0x1000, 37, 1280, 720};
  DestinoKey b{0x2000, 37, 640, 360};
  const uint32_t ia = cache.Registrar(a, 1);
  const uint32_t ib = cache.Registrar(b, 1);
  const uint32_t ia2 = cache.Registrar(a, 2);
  cache.AnotarResolve(ia);
  const bool ok = (ia == 0 && ib == 1 && ia2 == 0 && cache.tamano() == 2 &&
                   cache.destinos()[0].usos == 2 && cache.destinos()[1].usos == 1 &&
                   cache.total_resolves() == 1);
  char texto[160];
  std::snprintf(texto, sizeof(texto),
                "%s (destinos=%u usos_a=%llu resolves=%llu)", ok ? "OK" : "FALLA",
                (unsigned)cache.tamano(),
                (unsigned long long)(cache.tamano() ? cache.destinos()[0].usos : 0),
                (unsigned long long)cache.total_resolves());
  return std::string(texto);
}

}  // namespace cod6::native
