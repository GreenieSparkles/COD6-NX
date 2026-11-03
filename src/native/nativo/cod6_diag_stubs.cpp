// COD6 - stubs para poder compilar CON diagnostico.
//
// HISTORIA
// En la ronda 2, para que el port de COD5 enlazara, se puso COD6_DIAGNOSTICS a 0.
// Eso quito del binario 19 simbolos que no existian, pero tambien se llevo por
// delante TODAS las herramientas de depuracion del renderer, porque estan
// detras de la misma bandera:
//
//   trace_frame = cod6::diagnostics::kEnabled && REXCVAR_GET(cod6_debug_trace_frame) && ...
//
// Es decir: cod6_debug_trace_frame, cod6_debug_trace_after_s, cod6_debug_dump_*,
// cod6_debug_mip_watch... no hacian nada. Por eso llevamos tantas rondas a
// ciegas.
//
// Aqui se definen los simbolos que faltaban para poder compilar con
// COD6_DIAGNOSTICS = 1. Los contadores son de ganchos de COD5 que no portamos
// (estan atados a direcciones de invitado de COD5); aqui devuelven ceros, que es
// justo lo que vale un contador que nadie incrementa.
//
// Lo que SI se recupera con esto: el trace de un fotograma completo
// (pases, resolves y dibujos descartados), el volcado de imagenes y el
// seguimiento de mips. Eso es lo que hace falta para ver que dibuja mal.



/* COD6_STUB_SHADOW (2026-11-03)
 *
 * These registrations used to live here as stubs with a false default, and they ran
 * FIRST - so the real definitions in the *_hooks.cpp files lost the duplicate
 * registration, printed 'second registration ignored', and every one of these features
 * was dead from the start, whatever cod6.toml said:
 *
 *   cod6_cpu_skinning
 *   cod6_cpu_skinning_verify_only
 *   cod6_lightgrid_pack
 *   cod6_lightgrid_pack_verify_only
 *   cod6_native_zero_fill
 *   cod6_native_zero_fill_verify_only
 *   cod6_worker_stalled_wait
 *
 * Measured consequence: [opt-evidence] id=worker_stalled enabled=false even with the
 * cvar available, i.e. the job threads spun instead of sleeping - the same optimisation
 * BO2 ships as bo2zm_wait_sites.
 */


#include <cstdint>

#include <string>

#include <rex/cvar.h>

// --- Stubs de los ganchos de COD5 que NO se compilan en este port ------------
// Medido con el enlazador: las implementaciones de los *_hooks.cpp no forman parte
// del enlace (undefined reference a FLAGS_*_storage_()), asi que aqui esta la UNICA
// definicion de estos cvar. Son falsos porque la capacidad no esta portada.
REXCVAR_DEFINE_BOOL(cod6_cpu_skinning, false, "COD6",
                    "Stub: gancho de skinning de COD5, no portado");
REXCVAR_DEFINE_BOOL(cod6_cpu_skinning_verify_only, false, "COD6", "Stub de diagnostico");
REXCVAR_DEFINE_BOOL(cod6_lightgrid_pack, false, "COD6",
                    "Stub: empaquetado de lightgrid de COD5, no portado");
REXCVAR_DEFINE_BOOL(cod6_lightgrid_pack_verify_only, false, "COD6", "Stub de diagnostico");
REXCVAR_DEFINE_BOOL(cod6_native_zero_fill, false, "COD6",
                    "Stub: relleno de ceros de COD5, no portado");
REXCVAR_DEFINE_BOOL(cod6_native_zero_fill_verify_only, false, "COD6", "Stub de diagnostico");
REXCVAR_DEFINE_BOOL(cod6_worker_stalled_wait, false, "COD6",
                    "Stub: espera de worker de COD5, no portado");


// --- Stubs de los ganchos de COD5 que NO se compilan en este port ------------
// Al principio crei que estas definiciones competian con las de los *_hooks.cpp y
// que ganaba el stub. No es asi: medido con el enlazador, esas implementaciones no
// forman parte del enlace (undefined reference a FLAGS_*_storage_()), de modo que
// aqui esta la UNICA definicion de estos cvar. Son falsos porque la capacidad no
// esta portada, no porque alguien los pise.

// --- Los cvar que solo declara cod6_optimization_evidence.h ------------------

// Sus definiciones vivian en los ganchos de COD5 que no se compilan.

/* Los stubs de cod6_native_vertex_by_writes y _verify_only vivian aqui, pero
 * cod6_page_writes.cpp define los de verdad desde v287: en el enlace de PC eso
 * es "multiple definition" y en Switch era el aviso de registro duplicado del
 * arranque - y ganaba el stub, por eso el valor del toml no se aplicaba. */
REXCVAR_DEFINE_UINT32(cod6_model_lod_rigid, 0, "COD6", "Stub: LOD de modelos rigidos");
REXCVAR_DEFINE_UINT32(cod6_model_lod_skinned, 0, "COD6", "Stub: LOD de modelos con skinning");

// Lo usa native_renderer.cpp para el camino de red; aqui no lo usamos.
REXCVAR_DEFINE_STRING(log_network, "", "Log", "Stub: redireccion de log por red");

// --- Los lectores de contadores de los ganchos que no portamos ---------------
// Devuelven la estructura a cero: nadie las incrementa, asi que cero es la
// verdad, no una mentira.

#include "cod6_fill_hooks.h"
#include "cod6_lightgrid_hooks.h"
#include "cod6_render_hooks.h"
#include "cod6_skinning_hooks.h"
#include "cod6_worker_hooks.h"
#include "cod6_handoff_diagnostics.h"

namespace cod6::fill {
Counters ReadCounters() { return {}; }
}  // namespace cod6::fill

namespace cod6::lightgrid {
Counters ReadCounters() { return {}; }
}  // namespace cod6::lightgrid

namespace cod6::skinning {
Counters ReadCounters() { return {}; }
}  // namespace cod6::skinning

namespace cod6::workers {
Counters ReadCounters() { return {}; }
}  // namespace cod6::workers

namespace cod6::game {
DrawSurfCounters ReadDrawSurfCounters() { return {}; }
LodCounters ReadLodCounters() { return {}; }
}  // namespace cod6::game

namespace cod6::handoff {
void Report(uint64_t sample, uint64_t frames) {
  (void)sample;
  (void)frames;
}
}  // namespace cod6::handoff
