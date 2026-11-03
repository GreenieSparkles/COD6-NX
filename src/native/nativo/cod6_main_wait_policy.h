// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "cod6_worker_policy.h"
#include "cod6_handoff_policy.h"
namespace cod6::main_wait {
inline constexpr cod6::handoff::CodeCheck kChecks[]={
  {0x824191E8,240,UINT64_C(0x9826E0101FED781F)},
  {0x8242CDC8,216,UINT64_C(0xE874D618D1D97965)},
  {0x8242CCA8,288,UINT64_C(0x3B41A3202CD206C3)},
  {0x82240C00,52,UINT64_C(0x1F96D858AB4414E3)},
  {0x82241880,60,UINT64_C(0xE06C1B9BEF92225B)},
};
inline bool MatchesCode(const uint8_t* base) {
  if(!cod6::workers::MatchesWorkerCode(base))return false;
  for(const auto& check:kChecks) {
    uint64_t hash=UINT64_C(14695981039346656037);
    for(uint32_t i=0;i<check.bytes;++i)hash=(hash^base[check.address+i])*UINT64_C(1099511628211);
    if(hash!=check.fnv)return false;
  }
  return true;
}
}
