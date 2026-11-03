// SPDX-License-Identifier: GPL-3.0-or-later
//
// Interruptor de diagnostico del port.
//
// En el proyecto COD5 estaba a 1 y eso metia contadores atomicos y funciones de
// informe en el camino caliente. La documentacion del propio proyecto
// (docs/08_nfsmw_原生渲染器设计.md, lineas 114-119) dice dos cosas:
//   - "Do not time everything. Timers are only read on a sample..."
//   - los atomicos en Cortex-A57 son bucles lentos que molestan a otros nucleos
//
// Para la primera puesta en marcha en COD6 lo dejamos a 0: los bloques de
// diagnostico se compilan fuera y las funciones de informe pasan a ser
// versiones inline vacias que ya declaran las cabeceras.
#pragma once
#ifndef COD6_DIAGNOSTICS
#define COD6_DIAGNOSTICS 1
#endif
namespace cod6::diagnostics {
inline constexpr bool kEnabled = COD6_DIAGNOSTICS != 0;
}
