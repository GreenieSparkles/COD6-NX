// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "cod6_diagnostics.h"
#include <cstdint>

namespace cod6::handoff {
#if COD6_DIAGNOSTICS
void Configure();
void Report(uint64_t sample, uint64_t frames);
#else
inline void Configure() {}
inline void Report(uint64_t, uint64_t) {}
#endif
}
