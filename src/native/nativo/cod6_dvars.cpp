// COD6 - inyeccion de dvars del propio juego (equivalente a codmw_dvars de COD4).
//
// POR QUE EXISTE
// El registro de la tanda 241 confirma que en la maquina siguen saliendo el
// humo, el polvo y las calcomanias de los impactos. Apagarlas desde el
// renderizador NO es posible de forma segura: se intento dos veces y las dos
// salio la pantalla en negro, porque los efectos, el HUD y la composicion final
// son todos dibujos con mezcla y el renderizador no puede distinguirlos.
//
// El juego si sabe cual es cual. COD4 lo hace con codmw_dvars:
//     codmw_dvars = "r_glow 1; fx_enable 1; cg_brass 1; ..."
// Esto es lo mismo para COD6.
//
// COMO SE ENCUENTRAN LOS DVAR SIN SIMBOLOS
// El .xex esta comprimido (XEX2 + LZX): en el fichero no aparece ni una cadena.
// Pero en tiempo de ejecucion el SDK lo descomprime en la memoria del invitado,
// asi que los nombres SI estan ahi. Se hace en tres pasos, que es la tecnica
// clasica de los motores id Tech:
//
//   1. buscar el texto del nombre ("r_glow") en la memoria del invitado
//   2. buscar un puntero que apunte a esa direccion: el campo `name` de dvar_t
//   3. interpretar la estructura que empieza ahi:
//        +0  const char* name
//        +4  uint32 flags
//        +8  int8    type   (0 bool, 1 float, 2 int, 3 enum, 4 string, 5 color)
//        ... union value
//
// Todo lo que no cuadre se descarta: se comprueba que el puntero de nombre
// vuelve a apuntar al texto, que el tipo esta en rango y que el valor actual es
// razonable. Nunca se escribe a ciegas.
//
// SE EJECUTA TARDE A PROPOSITO
// Los dvar los registra el juego durante su arranque, no el SDK. Por eso el
// barrido no se hace al crear el sistema grafico sino tras N presentaciones,
// cuando el juego ya los ha creado.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <thread>
#include <atomic>
#include <vector>

#include <rex/cvar.h>
#include <rex/memory.h>
#include <rex/logging.h>

REXCVAR_DEFINE_STRING(cod6_dvars, "", "COD6",
                      "Dvars del propio juego a forzar al arrancar, separados por ';' "
                      "(formato: nombre valor; nombre valor). Vacio = no tocar nada");

// Cuantas presentaciones esperar antes de buscar. El juego registra sus dvar
// durante el arranque; a las 60 presentaciones ya estan todos.
REXCVAR_DEFINE_UINT32(cod6_dvars_at_swap, 60, "COD6",
                      "Presentaciones que esperar antes de aplicar cod6_dvars");

// false = solo registra lo que encuentra, sin escribir. Sirve para comprobar
// que la estructura se localiza bien antes de cambiar nada.
REXCVAR_DEFINE_BOOL(cod6_dvars_apply, true, "COD6",
                    "Aplicar de verdad los valores; false = solo buscarlos y registrarlos");

namespace cod6::dvars {
// Lista de ajustes, con el mismo formato que COD4:
//     "r_glow 0; fx_enable 0; cg_brass 0"


namespace {

// Limites del espacio del invitado que se barre. El .xex se carga a partir de
// 0x82000000; los dvar viven en el heap, por encima de la imagen.
constexpr uint32_t kScanStart = 0x82000000u;
constexpr uint32_t kScanEnd = 0x94000000u;  // 288 MB, cubre imagen y heap

struct Entrada {
  std::string name;
  std::string value;
};

std::vector<Entrada> Parsear(std::string_view texto) {
  std::vector<Entrada> salida;
  size_t i = 0;
  while (i < texto.size()) {
    while (i < texto.size() && (texto[i] == ';' || texto[i] == ' ')) ++i;
    size_t ini = i;
    while (i < texto.size() && texto[i] != ' ' && texto[i] != ';') ++i;
    if (ini == i) break;
    std::string nombre(texto.substr(ini, i - ini));
    while (i < texto.size() && texto[i] == ' ') ++i;
    size_t ini_val = i;
    while (i < texto.size() && texto[i] != ';') ++i;
    std::string valor(texto.substr(ini_val, i - ini_val));
    while (!valor.empty() && valor.back() == ' ') valor.pop_back();
    salida.push_back({std::move(nombre), std::move(valor)});
  }
  return salida;
}

/* COD6_V244_PAGINAS
 *
 * La v243 barria 288 MB a pelo con memchr y el juego murio con
 *
 *   Unhandled guest access violation: read of guest 0x84610000
 *
 * Esa direccion esta dentro del rango barrido pero NO esta mapeada: el espacio
 * del invitado es una reserva plana, no memoria comprometida. Leerla revienta.
 *
 * Ahora se comprueba pagina a pagina con TranslatePhysical antes de leer, y si
 * una pagina no esta mapeada se salta entera. Cuesta ~4600 comprobaciones por
 * barrido, que no es nada al lado de leer los datos. */
constexpr uint32_t kPage = 0x10000u;  // 64 KB

bool PaginaMapeada(rex::memory::Memory* memory, uint32_t dir) {
  if (!memory) return false;
  return memory->TranslatePhysical<void*>(size_t(dir)) != nullptr;
}

// Busca la primera aparicion del texto en la memoria del invitado.

uint32_t BuscarTexto(rex::memory::Memory* memory, const uint8_t* base,
                     const std::string& texto) {
  const size_t n = texto.size();
  if (!n) return 0;
  const uint8_t* p = base + kScanStart;
  const uint8_t* fin = base + kScanEnd - n;
  const uint8_t primero = uint8_t(texto[0]);
  while (p < fin) {
    const uint32_t dir_actual = uint32_t(p - base);
    const uint32_t fin_pagina = (dir_actual | (kPage - 1)) + 1;
    if (!PaginaMapeada(memory, dir_actual)) {
      p = base + fin_pagina;
      continue;
    }
    const uint8_t* limite = (fin_pagina < uint32_t(fin - base)) ? base + fin_pagina : fin;
    p = static_cast<const uint8_t*>(std::memchr(p, primero, size_t(limite - p)));
    if (!p) return 0;
    if (std::memcmp(p, texto.data(), n) == 0) {
      // Tiene que terminar en cero: es una cadena, no parte de otra palabra.
      if (p[n] == 0) return uint32_t(p - base);
    }
    ++p;
  }
  return 0;
}

// Busca un uint32 igual a `valor` en la memoria del invitado.
uint32_t BuscarPuntero(rex::memory::Memory* memory, const uint8_t* base, uint32_t valor) {
  const uint8_t* p = base + kScanStart;
  const uint8_t* fin = base + kScanEnd - 4;
  while (p < fin) {
    const uint32_t dir_actual = uint32_t(p - base);
    const uint32_t fin_pagina = (dir_actual | (kPage - 1)) + 1;
    if (!PaginaMapeada(memory, dir_actual)) {
      p = base + fin_pagina;
      continue;
    }
    const uint8_t* limite = (fin_pagina < uint32_t(fin - base)) ? base + fin_pagina : fin;
    p = static_cast<const uint8_t*>(std::memchr(p, uint8_t(valor & 0xFF), size_t(limite - p)));
    if (!p) return 0;
    if ((uintptr_t(p) & 3) == 0) {
      uint32_t leido;
      std::memcpy(&leido, p, 4);
      if (leido == valor) return uint32_t(p - base);
    }
    ++p;
  }
  return 0;
}

}  // namespace

namespace {

// COD6_V243_ASYNC
//
// La primera version hacia el barrido en el hilo de presentacion y el juego se
// quedaba clavado en la animacion de arranque: cada nombre cuesta dos barridos
// de ~288 MB (uno para el texto, otro para el puntero), y con cinco nombres son
// casi 3 GB de lecturas seguidas en el hilo que presenta.
//
// Ahora el barrido va en un hilo aparte: la presentacion sigue su curso, los
// dvar se escriben cuando aparecen y como mucho se pierde el fotograma en el que
// cambian. Los valores son enteros alineados a 4 bytes, asi que una escritura de
// ese tamano no se puede ver a medias.
std::atomic<bool> g_lanzado{false};

}  // namespace

void ApplyBlocking(rex::memory::Memory* memory, uint8_t* base) {
  const std::string lista = REXCVAR_GET(cod6_dvars);
  if (lista.empty()) return;
  const bool aplicar = REXCVAR_GET(cod6_dvars_apply);
  const auto entradas = Parsear(lista);
  REXLOG_INFO("[dvars] {} entradas en cod6_dvars (aplicar={})", entradas.size(), aplicar);

  uint32_t encontrados = 0, escritos = 0;
  for (const auto& e : entradas) {
    const uint32_t nombre_dir = BuscarTexto(memory, base, e.name);
    if (!nombre_dir) {
      REXLOG_WARN("[dvars] {}: nombre no encontrado en memoria", e.name);
      continue;
    }
    const uint32_t dvar_dir = BuscarPuntero(memory, base, nombre_dir);
    if (!dvar_dir) {
      REXLOG_WARN("[dvars] {}: texto en {:08X} pero ningun puntero apunta a el",
                  e.name, nombre_dir);
      continue;
    }
    if (!PaginaMapeada(memory, dvar_dir) || !PaginaMapeada(memory, dvar_dir + 16)) {
      REXLOG_WARN("[dvars] {}: la direccion {:08X} no esta mapeada", e.name, dvar_dir);
      continue;
    }
    // dvar_t: +0 name*, +4 flags, +8 type(int8), +12 valor
    uint32_t nombre_leido = 0, flags = 0;
    int8_t tipo = -1;
    uint32_t valor_actual = 0;
    std::memcpy(&nombre_leido, base + dvar_dir + 0, 4);
    std::memcpy(&flags, base + dvar_dir + 4, 4);
    std::memcpy(&tipo, base + dvar_dir + 8, 1);
    std::memcpy(&valor_actual, base + dvar_dir + 12, 4);
    if (nombre_leido != nombre_dir || tipo < 0 || tipo > 5) {
      REXLOG_WARN("[dvars] {}: estructura no valida en {:08X} (name={:08X} type={})",
                  e.name, dvar_dir, nombre_leido, int(tipo));
      continue;
    }
    ++encontrados;
    REXLOG_INFO("[dvars] {}: dvar en {:08X} tipo={} flags={:08X} valor actual={} (nuevo {})",
                e.name, dvar_dir, int(tipo), flags, valor_actual, e.value);
    if (!aplicar) continue;

    const int32_t deseado = int32_t(std::strtol(e.value.c_str(), nullptr, 10));
    switch (tipo) {
      case 0: {  // bool
        uint8_t v = deseado ? 1 : 0;
        std::memcpy(base + dvar_dir + 12, &v, 1);
        break;
      }
      case 1: {  // float
        float v = std::strtof(e.value.c_str(), nullptr);
        std::memcpy(base + dvar_dir + 12, &v, 4);
        break;
      }
      case 2:  // int
      case 3:  // enum
        std::memcpy(base + dvar_dir + 12, &deseado, 4);
        break;
      default:
        REXLOG_WARN("[dvars] {}: tipo {} no soportado, se deja como esta", e.name, int(tipo));
        continue;
    }
    ++escritos;
  }
  REXLOG_INFO("[dvars] resultado: {} localizados, {} escritos", encontrados, escritos);
}

// Lanza el barrido en un hilo aparte. Solo la primera llamada hace algo.
void Apply(rex::memory::Memory* memory, uint8_t* base) {
  if (!base || !memory) return;
  if (g_lanzado.exchange(true, std::memory_order_acq_rel)) return;
  if (REXCVAR_GET(cod6_dvars).empty()) return;
  std::thread([memory, base]() { ApplyBlocking(memory, base); }).detach();
}

}  // namespace cod6::dvars
