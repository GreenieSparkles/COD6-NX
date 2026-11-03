// COD6 - lector de anillo PM4 (fase 1). Implementacion.

#include "cod6_native_reader.h"

#include <cstdio>
#include <memory>

#include <rex/memory.h>

namespace cod6::native {

uint32_t Reader::Peek() const {
  if (base == nullptr) {
    return 0;
  }
  /* COD6_ORDEN119: el 202 decodifico TODO como "otros", sintoma de byte order
   * equivocado. Se prueba a leer sin invertir. */
  return rex::memory::load<uint32_t>(base + size_t(pos) * 4);
}

uint32_t Reader::Read() {
  const uint32_t value = Peek();
  Advance(1);
  return value;
}

bool RingReader::Walk(rex::memory::Memory* memory, uint32_t base, uint32_t bytes, uint32_t read_bytes,
                      uint32_t write_bytes) {
  // Guardas: sin memoria, sin anillo, o un rango imposible -> no se recorre nada.
  if (memory == nullptr || base == 0 || bytes < 8) {
    return false;
  }
  if ((bytes & (bytes - 1)) != 0) {  // el anillo siempre es potencia de dos
    return false;
  }
  const uint32_t words = bytes / 4;
  if (read_bytes >= bytes || write_bytes > bytes) {
    return false;
  }

  const uint8_t* memory_base = memory->TranslatePhysical(base);
  if (memory_base == nullptr) {
    return false;
  }

  Reader r;
  r.base = memory_base;
  r.mask = words - 1;
  r.pos = (read_bytes / 4) & r.mask;
  r.end = (write_bytes / 4) & r.mask;

  // Tope duro: un recorrido nunca puede quedarse colgado.
  constexpr uint32_t kMaxPackets = 200000;
  uint32_t packets = 0;

  while (r.Pending() >= 1 && packets < kMaxPackets) {
    const uint32_t header = r.Read();
    const uint32_t type = header >> 30;
    ++packets;
    ++counters_.packets;

    if (type == 0) {
      // Tipo 0: cabecera + N registros (N = bits 16..29 + 1).
      const uint32_t registers = ((header >> 16) & 0x3FFF) + 1;
      counters_.register_writes += registers;
      counters_.dwords += 1 + registers;
      if (r.Pending() < registers) {
        break;
      }
      r.Advance(registers);
    } else if (type == 1) {
      // Tipo 1: siempre 2 dwords en total.
      counters_.dwords += 2;
      if (r.Pending() < 1) {
        break;
      }
      r.Advance(1);
    } else if (type == 2) {
      // Tipo 2: NOP de una palabra.
      counters_.dwords += 1;
    } else {
      const uint32_t opcode = (header >> 8) & 0x7F;
      const uint32_t body = ((header >> 16) & 0x3FFF) + 1;
      counters_.dwords += 1 + body;
      if (r.Pending() < body) {
        break;
      }
      switch (opcode) {
        case 0x22:  // PM4_DRAW_INDX
        case 0x36:  // PM4_DRAW_INDX_2
          ++counters_.draws;
          break;
        case 0x27:  // PM4_IM_LOAD
        case 0x2B:  // PM4_IM_LOAD_IMMEDIATE
          ++counters_.shader_loads;
          break;
        case 0x3C:  // PM4_WAIT_REG_MEM
          ++counters_.waits;
          break;
        case 0x58:  // PM4_SET_CONSTANT
          ++counters_.writes;
          break;
        case 0x3F: {  // PM4_INDIRECT_BUFFER: entrar en el buffer destino
          if (body >= 2) {
            const uint32_t direccion = r.Read();
            const uint32_t palabras = r.Read();
            if (palabras != 0) {
              WalkLinear(memory, direccion & 0x1FFFFFFFu, palabras * 4u, 2);
            }
            r.Advance(body - 2);
            continue;
          }
          break;
        }
        case 0x63:  // PM4_INTERRUPT
        case 0x64:  // PM4_XE_SWAP
          break;
        default:
          ++counters_.unknown;
          break;
      }
      r.Advance(body);
    }
  }
  return true;
}

bool RingReader::WalkLinear(rex::memory::Memory* memory, uint32_t address, uint32_t bytes,
                           uint32_t depth) {
  if (memory == nullptr || depth == 0 || bytes < 4 || bytes > (1u << 20)) {
    return false;
  }
  const uint8_t* base = memory->TranslatePhysical(address);
  if (base == nullptr) {
    return false;
  }
  Reader r;
  r.base = base;
  r.mask = 0;
  r.pos = 0;
  r.end = bytes / 4;
  constexpr uint32_t kMax = 200000;
  uint32_t packets = 0;
  while (r.Pending() >= 1 && packets < kMax) {
    const uint32_t header = r.Read();
    const uint32_t type = header >> 30;
    ++packets;
    ++counters_.packets;
    if (type == 0) {
      const uint32_t registers = ((header >> 16) & 0x3FFF) + 1;
      counters_.register_writes += registers;
      counters_.dwords += 1 + registers;
      if (r.Pending() < registers) break;
      r.Advance(registers);
    } else if (type == 1) {
      counters_.dwords += 2;
      if (r.Pending() < 1) break;
      r.Advance(1);
    } else if (type == 2) {
      counters_.dwords += 1;
    } else {
      const uint32_t opcode = (header >> 8) & 0x7F;
      const uint32_t body = ((header >> 16) & 0x3FFF) + 1;
      counters_.dwords += 1 + body;
      if (r.Pending() < body) break;
      switch (opcode) {
        case 0x22:
        case 0x36: {
          ++counters_.draws;
          /* COD6_LISTA123: repartir por tamano para saber que tipo de dibujo es. */
          const uint32_t n = body;  // palabras del paquete ~ indices en el sitio
          if (n <= 3) {
            ++counters_.dibujos_3;
          } else if (n <= 6) {
            ++counters_.dibujos_6;
          } else if (n <= 32) {
            ++counters_.dibujos_32;
          } else if (n <= 128) {
            ++counters_.dibujos_128;
          } else {
            ++counters_.dibujos_grandes;
          }
          r.Advance(body);
          break;
        }
        case 0x27:
        case 0x2B:
          ++counters_.shader_loads;
          r.Advance(body);
          break;
        case 0x3C:
          ++counters_.waits;
          r.Advance(body);
          break;
        case 0x3F: {
          if (body >= 2) {
            const uint32_t direccion = r.Read();
            const uint32_t palabras = r.Read();
            if (palabras != 0) {
              WalkLinear(memory, direccion & 0x1FFFFFFFu, palabras * 4u, depth - 1);
            }
            r.Advance(body - 2);
          } else {
            r.Advance(body);
          }
          break;
        }
        default:
          ++counters_.unknown;
          r.Advance(body);
          break;
      }
    }
  }
  return true;
}

std::string RingReader::Summary() const {
  char buffer[256];
  std::snprintf(buffer, sizeof(buffer),
                "paquetes=%llu dwords=%llu dibujos=%llu shaders=%llu registros=%llu esperas=%llu "
                "otros=%llu",
                (unsigned long long)counters_.packets, (unsigned long long)counters_.dwords,
                (unsigned long long)counters_.draws, (unsigned long long)counters_.shader_loads,
                (unsigned long long)counters_.register_writes,
                (unsigned long long)counters_.waits, (unsigned long long)counters_.unknown);
  return std::string(buffer);
}

}  // namespace cod6::native
