// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>

namespace cod6::skinning {
struct KernelSpec { uint32_t address,bytes; uint64_t fingerprint; };
inline constexpr std::array<KernelSpec,4> kKernels = {{
    {0x82446A98,396,UINT64_C(0xE19BDCDF33C2605A)},
    {0x82446C28,496,UINT64_C(0x3DF298D850839237)},
    {0x82446E18,556,UINT64_C(0x176ECFB3209B3037)},
    {0x82447048,616,UINT64_C(0x1998E17EF3ED74F0)}}};
inline bool MatchesKernel(const uint8_t* code,const KernelSpec& spec) {
  uint64_t hash=UINT64_C(14695981039346656037);
  for(uint32_t i=0;i<spec.bytes;++i) hash=(hash^code[i])*UINT64_C(1099511628211);
  return hash==spec.fingerprint;
}
inline bool Overlap(uint32_t a,uint64_t bytes_a,uint32_t b,uint64_t bytes_b) {
  return uint64_t(a)<uint64_t(b)+bytes_b && uint64_t(b)<uint64_t(a)+bytes_a;
}
inline bool ValidRanges(unsigned bones,uint32_t input,uint32_t blend,uint32_t count,
                        uint32_t matrices,uint32_t output) {
  if(bones<1 || bones>4 || !count || count>32767 ||
      ((input|output|matrices)&15) || (blend&1)) return false;
  const uint64_t vertices=uint64_t(count)*32, weights=uint64_t(count)*(4*bones-2);
  // Bone offsets are uint16 byte offsets, followed by four 16-byte vectors.
  const uint64_t matrix_bytes=65536+64;
  const auto valid=[](uint32_t start,uint64_t bytes) {
    return start>=0x10000 && uint64_t(start)+bytes<=UINT64_C(0x100000000);
  };
  if(!valid(input,vertices) || !valid(output,vertices) || !valid(blend,weights) ||
      !valid(matrices,matrix_bytes)) return false;
  if((input!=output && Overlap(input,vertices,output,vertices)) ||
      Overlap(output,vertices,blend,weights) || Overlap(output,vertices,matrices,matrix_bytes)) return false;
  for(uint32_t constant : {0x82000DB0u,0x82000DC0u,0x82000DD0u,0x820A69C0u,
                           0x82000B40u,0x824BFC60u,0x84E9C140u,0x82000B20u})
    if(Overlap(output,vertices,constant,16)) return false;
  return true;
}
} // namespace cod6::skinning
