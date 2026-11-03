// COD6 - self-test del lector de anillo (fase 1).
//
// Se ejecuta SOLO en modo "nativo", al arrancar, sobre un flujo de paquetes
// sintetico en memoria propia: no toca memoria del juego ni dibuja nada. Sirve para
// verificar hoy mismo que el recorrido de paquetes es correcto (cabeceras, tamaños
// por tipo de paquete y cuenta de dibujos/cargas de shader) antes de conectarlo al
// anillo real.

#pragma once

#include <string>

namespace cod6::native {

struct SelfTestResult {
  bool ok = false;
  unsigned packets = 0;
  unsigned draws = 0;
  unsigned shader_loads = 0;
  unsigned register_writes = 0;
  unsigned dwords = 0;
};

// Construye un flujo conocido, lo recorre con RingReader y compara con lo esperado.
SelfTestResult RunSelfTest();

std::string FormatSelfTest(const SelfTestResult& result);

}  // namespace cod6::native
