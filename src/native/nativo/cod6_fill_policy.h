// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "cod6_page_writes.h"
#include <bit>
#include <cstring>

namespace cod6::fill {
struct Spec {uint32_t address,bytes;uint64_t fingerprint;};
inline constexpr Spec kFill{0x8237AB98,428,UINT64_C(0x4647A99BCA1FCA2E)};
inline constexpr Spec kBlockFill{0x8237AD48,248,UINT64_C(0x8CC9944D776DCEBB)};
inline constexpr uint32_t kVectorStride=0x824B4738;
inline bool Matches(const uint8_t* base,Spec spec) {
  uint64_t h=UINT64_C(14695981039346656037);
  for(uint32_t i=0;i<spec.bytes;++i)h=(h^base[spec.address+i])*UINT64_C(1099511628211);
  return h==spec.fingerprint;
}
template<class Context>
bool CanZero(const Context& ctx,const uint8_t* base) {
  const uint32_t bytes=ctx.r5.u32;
  if(ctx.r5.u64!=ctx.r5.u32 ||
     (ctx.r3.u32&127) || (ctx.r4.u32&255) || bytes<128 || bytes>(16u<<20) ||
     !cod6::writes::GuestPhysicalRange(ctx.r3.u32,ctx.r5.u32).valid)return false;
  uint32_t stride;std::memcpy(&stride,base+kVectorStride,4);
  if constexpr(std::endian::native==std::endian::little)stride=std::byteswap(stride);
  return stride==16;
}

// Exact effects of the audited zero-byte/aligned physical path. Retain scratch
// GPR/vector/CR/XER/LR effects too, not just the ABI's nonvolatile registers.
// The selected path has at least one 128-byte block; its final decrement sets
// CA and CR0. CR6 compares the remainder BEFORE the optional 16-byte loop.
template<class Context>
void ZeroTail(Context& ctx,const uint8_t* shift_table_zero) {
  const uint64_t destination=ctx.r3.u64;
  const uint32_t bytes=ctx.r5.u32;
  const uint32_t tail=bytes&127;
  ctx.r12.u64=ctx.lr;ctx.lr=kFill.address+8;
  ctx.r6.u64=16;ctx.r7.u64=bytes;
  ctx.r8.s64=ctx.r4.s32>>4;
  ctx.r9.u64=tail<16?bytes>>7:0;
  ctx.r10.u64=tail&15;
  ctx.r11.u64=destination+bytes-(tail&15);
  ctx.xer.ca=1;
  ctx.cr0.template compare<int32_t>(0,0,ctx.xer);
  ctx.cr6.template compare<uint32_t>(tail,16,ctx.xer);
  std::memset(ctx.v0.u8,0,16);
  std::memcpy(ctx.v12.u8,shift_table_zero,16);
  std::memcpy(ctx.v13.u8,shift_table_zero,16);
}
inline bool IsZero(const uint8_t* data,uint32_t bytes) {
  uint64_t bits=0;
  while(bytes>=8) {uint64_t word;std::memcpy(&word,data,8);bits|=word;data+=8;bytes-=8;}
  while(bytes--)bits|=*data++;
  return bits==0;
}
} // namespace cod6::fill
