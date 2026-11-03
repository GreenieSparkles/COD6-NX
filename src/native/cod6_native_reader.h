// COD6 - lector de anillo PM4 (fase 1 del backend nativo).
//
// Portado de COD5-SP-NX:  app/src/native/cod5_native_system.cpp  (struct Reader)
//
// El original es deliberadamente pequeno: cuatro enteros y tres operaciones. Lo
// importante no es el codigo, es la idea: un anillo se recorre con mascara
// (pos + words) & mask, sin punteros ni casos especiales, y cada paquete se lee del
// mismo buffer que ve la GPU.
//
// Aqui solo esta el lector y los contadores: NO dibuja, NO toca el SDK y NO se
// instala todavia. El objetivo de esta fase es que compile, y que cuando se conecte
// cuente exactamente lo mismo que el SDK (MAPA98 ya da la referencia).

#pragma once

#include <cstdint>
#include <string>

namespace rex::memory {
class Memory;
}

namespace cod6::native {

// Copia fiel del Reader de COD5.
struct Reader {
  const uint8_t* base = nullptr;
  uint32_t mask = 0;  // palabras - 1 dentro del anillo; 0 en un buffer lineal
  uint32_t pos = 0;
  uint32_t end = 0;

  uint32_t Pending() const { return mask ? ((end - pos) & mask) : (end - pos); }
  uint32_t Peek() const;
  uint32_t Read();
  void Advance(uint32_t words) { pos = mask ? ((pos + words) & mask) : (pos + words); }
};

// Contadores de lo recorrido (equivalen a los del mapa MAPA98 del SDK).
struct Counters {
  uint64_t packets = 0;
  uint64_t dwords = 0;
  uint64_t draws = 0;
  uint64_t shader_loads = 0;
  uint64_t register_writes = 0;
  uint64_t writes = 0;
  uint64_t waits = 0;
  uint64_t unknown = 0;
  /* COD6_LISTA123: reparto de dibujos por tamano (indices): 3, 6, <=32, <=128, mas */
  uint64_t dibujos_3 = 0;
  uint64_t dibujos_6 = 0;
  uint64_t dibujos_32 = 0;
  uint64_t dibujos_128 = 0;
  uint64_t dibujos_grandes = 0;
};

class RingReader {
 public:
  // base: direccion fisica del anillo, bytes: tamano, read/write: punteros en bytes.
  // Devuelve false si el rango no es sano (no se recorre nada en ese caso).
  bool Walk(rex::memory::Memory* memory, uint32_t base, uint32_t bytes, uint32_t read_bytes,
            uint32_t write_bytes);

  /* COD6_INDIRECTO122: los dibujos van dentro de buffers indirectos; hay que entrar. */
  bool WalkLinear(rex::memory::Memory* memory, uint32_t address, uint32_t bytes,
                  uint32_t depth = 1);

  const Counters& counters() const { return counters_; }
  std::string Summary() const;

 private:
  Counters counters_;
};

}  // namespace cod6::native
