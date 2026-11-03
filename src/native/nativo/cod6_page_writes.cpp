// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include "cod6_page_writes.h"
#include <rex/cvar.h>
#include <rex/ppc/func.h>
#include <rex/ppc/context.h>

/* COD6_WRITES_CVAR_FREE
 *
 * Estos dos llevaban .lifecycle(kInitOnly), y con eso el valor del toml no se
 * aplicaba: en cada arranque, ya desde el primer fotograma, se leia el valor por
 * defecto y el registro decia
 *   [writes] vertex hashes false, verify-only true
 * con cod6_native_vertex_by_writes = true escrito en el archivo. Se descarto
 * primero la hipotesis del momento de la llamada: en v300 Configure() paso al
 * primer BeginFrame, con el toml ya aplicado, y siguio leyendo false.
 *
 * En cambio cod6_native_force_mip0, que no lleva ese ciclo de vida, si toma el
 * valor del archivo. Sin el, el valor se puede fijar desde el toml como el resto.
 */
/* COD6_WRITES_DEFAULT_ON
 *
 * El valor del toml nunca llegaba a este cvar. La cadena es esta, en cvar.cpp:
 *
 *   if (GetFlagInfo(full_key) == nullptr) {
 *     GetPendingValuesStorage()[full_key].config = value_str;   // diferido
 *     ...
 *   }
 *
 * Al leer el toml, este cvar todavia no estaba registrado (se registra en este
 * archivo, que es un modulo cargado despues), asi que el valor quedo pendiente y
 * no se aplico. Por eso en cada arranque salia
 *   [writes] vertex hashes false, verify-only true
 * con cod6_native_vertex_by_writes = true en el archivo, y luego
 *   [texmon] marcas: aciertos 0 fallos 0
 * ni una comprobacion por marcas en toda la sesion.
 *
 * Quitarle kInitOnly (v301) no basto: el valor no llegaba ni a intentarse.
 *
 * BO2 no depende de la configuracion para esto: sus marcas de escritura por
 * pagina estan activas por defecto (su menu solo ofrece "TEXTURE CHECKS" para
 * forzar la relectura completa, como herramienta de diagnostico). Aqui igual:
 * activado por defecto, y el toml o la linea de comandos pueden apagarlo.
 */
REXCVAR_DEFINE_BOOL(cod6_native_vertex_by_writes, true, "COD5",
                    "Observed-write page stamps instead of hashing unchanged data (on by default, as in BO2)");
REXCVAR_DEFINE_BOOL(cod6_native_vertex_by_writes_verify_only, true, "COD5",
                    "Hash every observed-write candidate hit; detect missing writers before permitting reuse");

void cod6::writes::Configure() {
  g_verify_only=REXCVAR_GET(cod6_native_vertex_by_writes_verify_only);
  g_enabled.store(REXCVAR_GET(cod6_native_vertex_by_writes),std::memory_order_relaxed);
  REXLOG_INFO("[writes] vertex hashes {}, verify-only {}; 4KB pages, tracked writes bracketed; "
              "first 256 eligible hits and every 64th checked; writer coverage incomplete",Enabled(),g_verify_only);
}

// Audited retail copy routine. Do not replace it with host memcpy here: its
// volatile registers, alignment paths, guest stack and cache-line stores matter.
// Bracket its destination with a conservative cache-line margin on both sides.
// Other inline stores/fills and guest-stack effects remain untracked.
#if defined(__SWITCH__)
// The __imp_ alias of the original only exists in the Switch link environment;
// on PC the symbol is undefined, and this write-tracking hook is only meaningful
// on the console anyway.
extern "C" REX_FUNC(__imp__sub_8237AE40);
extern "C" REX_FUNC(sub_8237AE40) {
  if(!cod6::writes::Enabled()) {__imp__sub_8237AE40(ctx,base);return;}
  static const bool supported=[&] {
    uint64_t hash=UINT64_C(14695981039346656037);
    for(uint32_t i=0;i<736;++i)hash=(hash^base[0x8237AE40u+i])*UINT64_C(1099511628211);
    const bool supported=hash==UINT64_C(0x32950FE82AAAE662);
    if(!supported)REXLOG_ERROR("[writes] copy writer fingerprint mismatch; original retained, write-cache reuse prohibited for physical writes");
    return supported;
  }();
  cod6::writes::GuestWriteScope write(ctx.r3.u32,ctx.r5.u32,supported,128);
  __imp__sub_8237AE40(ctx,base);
}
#endif  // __SWITCH__
