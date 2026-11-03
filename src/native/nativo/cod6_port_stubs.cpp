// COD6 - stubs de lo que el port de COD5 espera y nuestro SDK no tiene.
//
// Solo dos simbolos; los demas se resolvieron compilando sin diagnostico
// (COD6_DIAGNOSTICS = 0), que ademas es lo que pide la documentacion del
// proyecto para el camino caliente.

#include <cstdint>

#include <cstdio>
#include "cod6_main_wait.h"

namespace cod6::main_wait {

// COD5 enganchaba aqui el hilo principal del juego para avisarle cuando habia
// progreso en el anillo, y asi no gastaba CPU esperando.
//
// Ese enganche depende de direcciones del invitado de COD5 (sub_82xxxxxx) y no
// existe en COD6. Sin el, el juego sigue esperando como lo haria en una consola
// real: es correcto, solo que la espera se resuelve sondeando en vez de por
// aviso. Cuando portemos los ganchos de COD6, esta funcion pasara a despertar
// al hilo de verdad.
void Notify(uint8_t* base, Reason reason) {
  (void)base;
  (void)reason;
}

}  // namespace cod6::main_wait

// COD5 anadio esta funcion a su fork del SDK: deja de vigilar escrituras en las
// paginas que el juego marca de solo lectura, para que leerlas no provoque una
// excepcion simulada (200.000 por segundo en el menu, que los dejaba a 8 fps).
//
// Nuestro SDK no la tiene con ese nombre, pero ya hace lo mismo por otra via: el
// cvar cod6_sin_proteccion (por defecto true) toma el camino seguro de
// RexGmProtect y no registra ninguna pagina vigilada. Es el arreglo PROT218 de
// la version 218, que bajo los fallos de 1.488.973 a 222 y quito el parpadeo
// magenta.
//
// Asi que aqui solo se deja constancia; no hay nada que activar.
extern "C" void RexGmSetReadOnlyMapped(bool enabled) {
  static bool avisado = false;
  if (!avisado && enabled) {
    avisado = true;
    // Se usa printf y no el log del SDK a proposito: esto puede llamarse antes
    // de que el registro de logs este listo.
    std::fprintf(stderr,
                 "[cod6] RexGmSetReadOnlyMapped(true): equivalente ya activo "
                 "mediante cod6_sin_proteccion (PROT218)\n");
    std::fflush(stderr);
  }
}
