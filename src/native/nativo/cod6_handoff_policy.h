// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace cod6::handoff {
struct CodeCheck {uint32_t address, bytes; uint64_t fnv;};
inline constexpr CodeCheck kChecks[] = {
    {0x8242CDC8,216,UINT64_C(0xE874D618D1D97965)},
    {0x82426578,184,UINT64_C(0x730BD07948E8D3A5)},
    {0x82241880,60,UINT64_C(0xE06C1B9BEF92225B)},
};
inline bool MatchesCode(const uint8_t* base) {
  for(const auto& check:kChecks) {
    uint64_t hash=UINT64_C(14695981039346656037);
    for(uint32_t i=0;i<check.bytes;++i)hash=(hash^base[check.address+i])*UINT64_C(1099511628211);
    if(hash!=check.fnv)return false;
  }
  return true;
}
inline bool MainCall(uint32_t lr,uint32_t predicate,uint32_t repeat) {
  return lr==0x8241925C && predicate==0x82240C00 && repeat==1;
}
inline bool BackendCall(uint32_t lr) {
  return lr==0x82426708 || lr==0x82426750 || lr==0x824267AC;
}
struct Partition {
  uint64_t begin=0,notify_begin=0,notify_end=0;
  uint32_t notifications=0;
  void NotifyBegin(uint64_t now) {++notifications;notify_begin=now;}
  void NotifyEnd(uint64_t now) {notify_end=now;}
  bool Valid(uint64_t end) const {
    return notifications==1 && begin<=notify_begin && notify_begin<=notify_end && notify_end<=end;
  }
};
}
