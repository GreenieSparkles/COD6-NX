// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>

namespace cod6::lightgrid {
inline constexpr uint32_t kAddress=0x82429DE0,kCodeBytes=2312;
inline constexpr uint64_t kFingerprint=UINT64_C(0x27F04846247629CF);
inline constexpr uint32_t kSourceBytes=168,kDestinationSpan=5408;
// Audited from the locked retail instructions, not from another game's ABI.
inline constexpr std::array<uint8_t,64> kSourceOffsets={
  0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,
  48,51,54,57,60,0,9,63,66,36,45,69,72,75,78,81,
  84,87,90,93,96,120,129,99,102,156,165,105,108,111,114,117,
  120,123,126,129,132,135,138,141,144,147,150,153,156,159,162,165};
inline constexpr std::array<uint16_t,8> kRuns={0,0x100,0x400,0x500,0x1000,0x1100,0x1400,0x1500};
inline constexpr uint32_t DestinationOffset(unsigned i) { return kRuns[i/8]+(i%8)*4; }
inline bool MatchesCode(const uint8_t* code) {
  uint64_t hash=UINT64_C(14695981039346656037);
  for(uint32_t i=0;i<kCodeBytes;++i) hash=(hash^code[i])*UINT64_C(1099511628211);
  return hash==kFingerprint;
}
inline bool ValidRanges(uint32_t source,uint32_t destination) {
  const uint64_t source_end=uint64_t(source)+kSourceBytes;
  const uint64_t destination_end=uint64_t(destination)+kDestinationSpan;
  if(source<0x10000 || destination<0x10000 || (destination&3) ||
     source_end>UINT64_C(0x100000000) || destination_end>UINT64_C(0x100000000)) return false;
  // The sampled verifier computes expected words before the original runs.
  // Reject overlapping bounding ranges (including holes) so it cannot read a
  // different source generation. The streaming kernel itself preserves aliases.
  if(uint64_t(source)<destination_end && uint64_t(destination)<source_end) return false;
  // Do not allow the candidate to overwrite its own audited instructions.
  return !(uint64_t(destination)<uint64_t(kAddress)+kCodeBytes && uint64_t(kAddress)<destination_end);
}
struct Tail {
  uint32_t r4=0,r6=0,r7=0,r8=0,r9=0,r10=0,r11=0;
};
// Each original store is preceded by three byte loads of this triplet, with no
// intervening store. Complete those loads before invoking store; do not memcpy
// all 168 input bytes or defer all stores. This also matches overlapping memory.
template<class Store>
Tail PackTo(const uint8_t* source,uint8_t alpha,Store store) {
  uint32_t red=0,green=0,blue=0;
  const uint32_t a=uint32_t(alpha)<<24;
  for(unsigned i=0;i<kSourceOffsets.size();++i) {
    const unsigned offset=kSourceOffsets[i];
    red=source[offset];green=source[offset+1];blue=source[offset+2];
    store(i,DestinationOffset(i),a|(red<<16)|(green<<8)|blue);
  }
  return {a|(red<<16)|(green<<8)|blue,a|(red<<16)|(green<<8),
          (uint32_t(alpha)<<16)|(red<<8)|green,(uint32_t(alpha)<<16)|(red<<8),
          blue,(uint32_t(alpha)<<8)|red,green};
}
inline void StoreWord(uint8_t* destination,uint32_t word) {
  if constexpr(std::endian::native==std::endian::little) word=std::byteswap(word);
  std::memcpy(destination,&word,4);
}
inline uint32_t LoadWord(const uint8_t* source) {
  uint32_t word;std::memcpy(&word,source,4);
  if constexpr(std::endian::native==std::endian::little) word=std::byteswap(word);
  return word;
}
template<class Context>
void ApplyTail(Context& ctx,const Tail& tail) {
  // Assignment to u64 deliberately clears the upper half, as each original
  // expression does. CR, XER, LR, MSR, FPSCR and every other register survive.
  ctx.r4.u64=tail.r4;ctx.r6.u64=tail.r6;ctx.r7.u64=tail.r7;
  ctx.r8.u64=tail.r8;ctx.r9.u64=tail.r9;ctx.r10.u64=tail.r10;ctx.r11.u64=tail.r11;
}
template<class Context>
bool MatchesTail(const Context& ctx,const Tail& tail) {
  return ctx.r4.u64==tail.r4 && ctx.r6.u64==tail.r6 && ctx.r7.u64==tail.r7 &&
         ctx.r8.u64==tail.r8 && ctx.r9.u64==tail.r9 && ctx.r10.u64==tail.r10 && ctx.r11.u64==tail.r11;
}
} // namespace cod6::lightgrid
