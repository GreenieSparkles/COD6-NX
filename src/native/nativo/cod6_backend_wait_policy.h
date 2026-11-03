// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "cod6_main_wait_policy.h"

namespace cod6::backend_wait {
// Fixed-Korean-COD5 windows that identify the Backend's pending-difference
// wait. These are explicit audited windows, not recovered function sizes.
inline constexpr cod6::handoff::CodeCheck kChecks[] = {
    {0x82425708, 404, UINT64_C(0xE4A7226922D2E8B7)},
    {0x824256A8, 92, UINT64_C(0x7E6F0610E9917D25)},
    {0x8235ECE8, 36, UINT64_C(0x43DBC341125D4297)},
};

inline bool MatchesCode(const uint8_t* base) {
  if (!cod6::main_wait::MatchesCode(base)) return false;
  for (const auto& check : kChecks) {
    uint64_t hash = UINT64_C(14695981039346656037);
    for (uint32_t i = 0; i < check.bytes; ++i) {
      hash = (hash ^ base[check.address + i]) * UINT64_C(1099511628211);
    }
    if (hash != check.fnv) return false;
  }
  return true;
}

// 82425744 calls 8242CDC8(predicate=824256A8, repeat=1); LR is the next
// instruction. No other helper caller is admitted by this candidate.
inline bool Call(uint32_t lr, uint32_t predicate, uint32_t repeat) {
  return lr == 0x82425748 && predicate == 0x824256A8 && repeat == 1;
}
}  // namespace cod6::backend_wait
