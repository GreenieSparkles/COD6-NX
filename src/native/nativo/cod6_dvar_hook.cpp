// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * COD6_DVAR_HOOK
 *
 * COD4 apaga efectos con codmw_dvars y lo hace BIEN: engancha Dvar_Register* y
 * pone su valor EN EL MOMENTO DEL REGISTRO. No escanea memoria. Sus propios
 * strings lo dicen: "Se ponen AL REGISTRARSE".
 *
 * Nuestro cod6_dvars escaneaba 288 MB por nombre y colgaba el juego. Esto es el
 * camino de COD4, y ahora FUNCIONA.
 *
 * Como se encontraron las funciones. En el codigo generado se busco que sub_ era
 * llamada con mas direcciones de cadena distintas: sub_822E1660 salio con 728
 * direcciones distintas en 743 llamadas. Enganchandolas y registrando el nombre
 * (r3) se confirmo el reparto, con 3809 dvars vistos:
 *
 *   sub_822E15D0  Dvar_RegisterBool   r4 = 0/1
 *       "r_dof_enable" r4=1   "sm_enable" r4=1   "r_glow" r4=1   "r_fog" r4=1
 *   sub_822E1618  Dvar_RegisterInt    r4 = entero
 *       "com_maxfps" r4=60   "com_maxFrameTime" r4=100
 *   sub_822E1660  Dvar_RegisterFloat  r4 = bits IEEE
 *       "r_dof_nearBlur" r4=1065353216 (1.0f)
 *
 * REQUISITO: las llamadas del codigo generado tienen que ir por el alias debil
 * sub_XXXX, no por __imp__sub_XXXX. Sin eso el enganche no se ejecuta nunca.
 * Ese cambio se hizo aparte con sed sobre generated/default
 * (743 + 319 + 183 + 977 = 2222 puntos de llamada).
 */
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/memory/utils.h>
#include <rex/ppc/function.h>

/* Lista de ajustes: 'nombre valor; nombre valor; ...' igual que COD4. */
REXCVAR_DEFINE_STRING(cod6_dvar_set, "", "COD6",
                      "Dvars a forzar al registrarse: 'r_dof_enable 0; sm_enable 0; ...'");

REXCVAR_DEFINE_BOOL(cod6_dvar_diag, false, "COD6",
                    "Registrar cada dvar que se registra (nombre y valor). Muy verboso.");

namespace {

struct Ajuste {
  std::string nombre;
  double valor;
};

const std::vector<Ajuste>& Ajustes() {
  static std::vector<Ajuste> cache;
  static std::string ultima;
  const std::string& lista = REXCVAR_GET(cod6_dvar_set);
  if (lista == ultima) return cache;
  ultima = lista;
  cache.clear();
  size_t pos = 0;
  while (pos < lista.size()) {
    while (pos < lista.size() && (lista[pos] == ' ' || lista[pos] == ';' || lista[pos] == ',')) ++pos;
    size_t ini = pos;
    while (pos < lista.size() && lista[pos] != ' ' && lista[pos] != ';' && lista[pos] != ',') ++pos;
    if (ini == pos) break;
    std::string nombre = lista.substr(ini, pos - ini);
    while (pos < lista.size() && (lista[pos] == ' ' || lista[pos] == '=')) ++pos;
    size_t vini = pos;
    while (pos < lista.size() && lista[pos] != ';' && lista[pos] != ',') ++pos;
    std::string valor = lista.substr(vini, pos - vini);
    if (!nombre.empty() && !valor.empty()) {
      cache.push_back({nombre, std::strtod(valor.c_str(), nullptr)});
    }
  }
  return cache;
}

const char* CadenaEn(rex::memory::Memory* mem, uint32_t dir) {
  if (!mem || !dir) return nullptr;
  const char* p = mem->TranslateVirtual<const char*>(dir);
  if (!p) return nullptr;
  for (int i = 0; i < 64; ++i) {
    const char c = p[i];
    if (c == 0) return i ? p : nullptr;
    if (c < 32 || c > 126) return nullptr;
  }
  return nullptr;
}

/* Devuelve true y escribe el valor forzado si el nombre esta en la lista. */
bool BuscarForzado(const char* nombre, double* valor_out) {
  if (!nombre) return false;
  for (const Ajuste& a : Ajustes()) {
    if (a.nombre == nombre) { *valor_out = a.valor; return true; }
  }
  return false;
}

std::atomic<uint32_t> g_vistos{0}, g_forzados{0};
}  // namespace

#define COD6_DVAR_BOOL(addr)                                                      \
  extern "C" REX_FUNC(__imp__sub_##addr);                                         \
  extern "C" REX_FUNC(sub_##addr) {                                               \
    auto* mem = REX_KERNEL_STATE()->memory();                                     \
    const char* nombre = CadenaEn(mem, ctx.r3.u32);                               \
    double valor = 0.0;                                                           \
    if (BuscarForzado(nombre, &valor)) {                                          \
      if (ctx.r4.u32 != (valor != 0.0 ? 1u : 0u)) {                               \
        REXLOG_INFO("[dvar-set] {}: {} -> {}", nombre, ctx.r4.u32, (valor != 0.0 ? 1 : 0)); \
      }                                                                           \
      ctx.r4.u32 = (valor != 0.0) ? 1u : 0u;                                      \
      g_forzados.fetch_add(1, std::memory_order_relaxed);                         \
    } else if (REXCVAR_GET(cod6_dvar_diag)) {                                     \
      REXLOG_INFO("[dvar] {} = {}", nombre ? nombre : "?", ctx.r4.u32);            \
    }                                                                             \
    g_vistos.fetch_add(1, std::memory_order_relaxed);                             \
    __imp__sub_##addr(ctx, base);                                                 \
  }

#define COD6_DVAR_INT(addr)                                                       \
  extern "C" REX_FUNC(__imp__sub_##addr);                                         \
  extern "C" REX_FUNC(sub_##addr) {                                               \
    auto* mem = REX_KERNEL_STATE()->memory();                                     \
    const char* nombre = CadenaEn(mem, ctx.r3.u32);                               \
    double valor = 0.0;                                                           \
    if (BuscarForzado(nombre, &valor)) {                                          \
      ctx.r4.s64 = int32_t(valor);                                                \
      g_forzados.fetch_add(1, std::memory_order_relaxed);                         \
    }                                                                             \
    g_vistos.fetch_add(1, std::memory_order_relaxed);                             \
    __imp__sub_##addr(ctx, base);                                                 \
  }

#define COD6_DVAR_FLOAT(addr)                                                     \
  extern "C" REX_FUNC(__imp__sub_##addr);                                         \
  extern "C" REX_FUNC(sub_##addr) {                                               \
    auto* mem = REX_KERNEL_STATE()->memory();                                     \
    const char* nombre = CadenaEn(mem, ctx.r3.u32);                               \
    double valor = 0.0;                                                           \
    if (BuscarForzado(nombre, &valor)) {                                          \
      const float f = float(valor);                                               \
      uint32_t bits = 0;                                                          \
      std::memcpy(&bits, &f, sizeof(bits));                                       \
      ctx.r4.u32 = bits;                                                          \
      g_forzados.fetch_add(1, std::memory_order_relaxed);                         \
    }                                                                             \
    g_vistos.fetch_add(1, std::memory_order_relaxed);                             \
    __imp__sub_##addr(ctx, base);                                                 \
  }

COD6_DVAR_BOOL(822E15D0)
COD6_DVAR_INT(822E1618)
COD6_DVAR_FLOAT(822E1660)
