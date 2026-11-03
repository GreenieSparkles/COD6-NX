// COD6 - self-test del lector de anillo: flujo sintetico y comprobacion.

#include "cod6_native_selftest.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include "cod6_native_reader.h"

namespace cod6::native {

namespace {

// Cabecera tipo 0 con N registros (N en bits 16..29, guardando N-1).
uint32_t PacketType0(uint32_t registers) {
  return (0u << 30) | ((registers - 1u) << 16);
}
// Cabecera tipo 1: dos dwords.
uint32_t PacketType1() { return (1u << 30); }
// Cabecera tipo 2: NOP de una palabra.
uint32_t PacketType2() { return (2u << 30); }
// Cabecera tipo 3 con opcode y cuerpo de N dwords.
uint32_t PacketType3(uint32_t opcode, uint32_t body) {
  return (3u << 30) | (opcode << 8) | ((body - 1u) << 16);
}

}  // namespace

SelfTestResult RunSelfTest() {
  /*
   * Flujo construido a mano (todo en dwords), con la cuenta esperada:
   *   tipo0 con 3 registros      -> 1 + 3 dwords, 3 escrituras de registro
   *   tipo2 (NOP)                -> 1 dword
   *   tipo3 DRAW_INDX_2 (cuerpo 2) -> 1 + 2 dwords, 1 dibujo
   *   tipo3 IM_LOAD (cuerpo 3)   -> 1 + 3 dwords, 1 carga de shader
   *   tipo3 WAIT_REG_MEM (2)     -> 1 + 2 dwords, 1 espera
   *   tipo1                      -> 2 dwords
   */
  std::vector<uint32_t> stream;
  stream.push_back(PacketType0(3));
  stream.push_back(0x11111111);
  stream.push_back(0x22222222);
  stream.push_back(0x33333333);
  stream.push_back(PacketType2());
  stream.push_back(PacketType3(0x36, 2));
  stream.push_back(0xAAAAAAAA);  // iniciador
  stream.push_back(0xBBBBBBBB);  // indice
  stream.push_back(PacketType3(0x27, 3));
  stream.push_back(0x00000001);
  stream.push_back(0x00000002);
  stream.push_back(0x00000003);
  stream.push_back(PacketType3(0x3C, 2));
  stream.push_back(0x00000000);
  stream.push_back(0x00000000);
  stream.push_back(PacketType1());
  stream.push_back(0x00000000);

  /* COD6_ORDEN120: el lector ya NO invierte bytes (v119), asi que el flujo de prueba
   * va tal cual. Antes cada cambio de orden rompia este self-test. */

  const unsigned expected_packets = 6;
  const unsigned expected_draws = 1;
  const unsigned expected_shaders = 1;
  const unsigned expected_registers = 3;

  SelfTestResult result;
  result.packets = expected_packets;
  result.draws = expected_draws;
  result.shader_loads = expected_shaders;
  result.register_writes = expected_registers;
  result.dwords = unsigned(stream.size());

  // Buffer lineal: la mascara 0 del Reader ya soporta este caso.
  Reader r;
  r.base = reinterpret_cast<const uint8_t*>(stream.data());
  r.mask = 0;
  r.pos = 0;
  r.end = uint32_t(stream.size());

  unsigned packets = 0, draws = 0, shaders = 0, registers = 0;
  while (r.Pending() >= 1 && packets < 1000) {
    const uint32_t header = r.Read();
    const uint32_t type = header >> 30;
    ++packets;
    if (type == 0) {
      const uint32_t count = ((header >> 16) & 0x3FFF) + 1;
      registers += count;
      if (r.Pending() < count) {
        break;
      }
      r.Advance(count);
    } else if (type == 1) {
      if (r.Pending() < 1) {
        break;
      }
      r.Advance(1);
    } else if (type == 2) {
      // una sola palabra
    } else {
      const uint32_t opcode = (header >> 8) & 0x7F;
      const uint32_t body = ((header >> 16) & 0x3FFF) + 1;
      if (r.Pending() < body) {
        break;
      }
      if (opcode == 0x22 || opcode == 0x36) {
        ++draws;
      } else if (opcode == 0x27 || opcode == 0x2B) {
        ++shaders;
      }
      r.Advance(body);
    }
  }

  /* COD6_VEREDICTO117: se guardan los numeros REALES medidos, no los esperados. */
  result.packets = packets;
  result.draws = draws;
  result.shader_loads = shaders;
  result.register_writes = registers;
  result.dwords = unsigned(stream.size());
  result.ok = (packets == expected_packets && draws == expected_draws &&
               shaders == expected_shaders && registers == expected_registers);
  return result;
}

std::string FormatSelfTest(const SelfTestResult& result) {
  char buffer[256];
  std::snprintf(buffer, sizeof(buffer),
                "%s (paquetes=%u esperado=6, dibujos=%u, shaders=%u, registros=%u, dwords=%u)",
                result.ok ? "OK" : "FALLA", result.packets, result.draws, result.shader_loads,
                result.register_writes, result.dwords);
  return std::string(buffer);
}

}  // namespace cod6::native
