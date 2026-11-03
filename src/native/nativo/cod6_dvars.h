// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>

#include <rex/cvar.h>
#include <rex/memory.h>

REXCVAR_DECLARE(std::string, cod6_dvars);
REXCVAR_DECLARE(uint32_t, cod6_dvars_at_swap);
REXCVAR_DECLARE(bool, cod6_dvars_apply);

namespace cod6::dvars {
// Lanza el barrido en un hilo aparte; no bloquea la presentacion.
void Apply(rex::memory::Memory* memory, uint8_t* base);
// El barrido en si. No llamar desde el hilo de presentacion.
void ApplyBlocking(rex::memory::Memory* memory, uint8_t* base);
}  // namespace cod6::dvars
