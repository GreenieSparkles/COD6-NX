// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace cod6::game {
inline constexpr uint32_t kLodSetupAddress=0x82405078;
inline constexpr uint32_t kLodSetupBytes=488;
inline constexpr uint64_t kLodSetupFingerprint=UINT64_C(0x399112B4349D7BED);
inline bool MatchesLodSetup(const uint8_t* code) {
  uint64_t hash=UINT64_C(14695981039346656037);
  for(uint32_t i=0;i<kLodSetupBytes;++i) hash=(hash^code[i])*UINT64_C(1099511628211);
  return hash==kLodSetupFingerprint;
}

struct LodAdjustment { uint32_t changed=0, invalid=0; };

// Reproduce the original final multiply with the requested scale, using the
// original function's cached FOV/zoom-dependent base. 0 preserves that channel.
// The game's own scale registration admits [1,4], with larger = less detail.
// Do not change bias, origin, cached bases, culling planes or saved dvar values.
template<class Load,class Store>
LodAdjustment ApplyLodScale(uint32_t output,uint32_t rigid,uint32_t skinned,
                            Load load,Store store) {
  LodAdjustment result;
  if(output<0x10000 || uint64_t(output)+0x38>UINT64_C(0x100000000)) {
    if(rigid || skinned) result.invalid=3;
    return result;
  }
  const uint32_t requested[2]={rigid,skinned};
  for(uint32_t channel=0;channel<2;++channel) {
    if(!requested[channel]) continue;
    const uint32_t bit=1u<<channel;
    if(requested[channel]>4) { result.invalid|=bit;continue; }
    const uint32_t scale_offset=channel?0x18:0x0C;
    const uint32_t base_offset=channel?0x20:0x14;
    const float base=std::bit_cast<float>(load(output+base_offset));
    if(!std::isfinite(base) || base<0) { result.invalid|=bit;continue; }
    // The generated fmuls promotes the operands to double, rounds to float.
    const double product=double(base)*double(requested[channel]);
    if(product>double(std::numeric_limits<float>::max())) { result.invalid|=bit;continue; }
    const float value=float(product);
    const uint32_t bits=std::bit_cast<uint32_t>(value);
    if(load(output+scale_offset)!=bits) {
      store(output+scale_offset,bits);
      result.changed|=bit;
    }
  }
  return result;
}
} // namespace cod6::game
