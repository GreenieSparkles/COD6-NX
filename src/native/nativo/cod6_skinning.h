// SPDX-License-Identifier: GPL-3.0-or-later
// Fixed-COD5 local-register CPU reference for the later GPU skinning path.
// Arithmetic/packing follow the generated PPC routines, including their
// non-fused multiply/add order and first-bone-only normal/tangent transform.
#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <rex/ppc/context.h>
#include <rex/ppc/intrinsics.h>

namespace cod6::skinning {
inline simde__m128 LoadVector(const uint8_t* p) {
  return simde_mm_castsi128_ps(simde_mm_shuffle_epi8(
      simde_mm_load_si128(reinterpret_cast<const simde__m128i*>(p)),
      simde_mm_load_si128(reinterpret_cast<const simde__m128i*>(VectorMaskL))));
}
inline void StoreVector(uint8_t* p, simde__m128 value) {
  simde_mm_store_si128(reinterpret_cast<simde__m128i*>(p), simde_mm_shuffle_epi8(
      simde_mm_castps_si128(value),
      simde_mm_load_si128(reinterpret_cast<const simde__m128i*>(VectorMaskL))));
}
template<unsigned GuestLane> inline simde__m128 Splat(simde__m128 value) {
  constexpr int lane = 3 - GuestLane;
  return simde_mm_shuffle_ps(value, value, (lane<<6)|(lane<<4)|(lane<<2)|lane);
}
inline simde__m128 MulAdd(simde__m128 a, simde__m128 b, simde__m128 c) {
  // The target and oracle are compiled with -ffp-contract=off.
  return simde_mm_add_ps(simde_mm_mul_ps(a,b),c);
}
inline uint16_t Load16(const uint8_t* p) {
  uint16_t value; std::memcpy(&value,p,2); return std::byteswap(value);
}
inline uint32_t Bits(simde__m128 value) {
  return static_cast<uint32_t>(simde_mm_cvtsi128_si32(simde_mm_castps_si128(value)));
}
inline int32_t Signed10(uint32_t value) {
  const int32_t v = static_cast<int32_t>(value & 1023);
  return v >= 512 ? v - 1024 : v;
}
inline simde__m128 Unpack(uint32_t word) {
  const int32_t x = Signed10(word), y = Signed10(word>>10), z = Signed10(word>>20);
  return simde_mm_castsi128_ps(simde_mm_set_epi32(
      x == -512 ? 0x7FC00000 : 0x40400000+x,
      0x40400000+y,0x40400000+z,(word>>30)|0x3F800000));
}
inline uint32_t Pack(simde__m128 value) {
  alignas(16) float lanes[4]; simde_mm_store_ps(lanes,value);
  uint32_t packed = 0;
  for (unsigned i=0;i<4;++i) {
    const float lo=std::bit_cast<float>(i==3 ? 0x40400000u : 0x403FFE01u);
    const float hi=std::bit_cast<float>(i==3 ? 0x40400003u : 0x404001FFu);
    float v=lanes[3-i];
    v = v < lo ? lo : (v > hi ? hi : v);
    packed |= (std::bit_cast<uint32_t>(v)&(i==3 ? 3u : 1023u)) << (i*10);
  }
  return packed;
}
struct Matrix {
  simde__m128 x,y,z,t;
  explicit Matrix(const uint8_t* p)
      :x(LoadVector(p)),y(LoadVector(p+16)),z(LoadVector(p+32)),t(LoadVector(p+48)) {}
  simde__m128 Position(simde__m128 p) const {
    return MulAdd(Splat<0>(p),x,MulAdd(Splat<1>(p),y,MulAdd(z,Splat<2>(p),t)));
  }
  simde__m128 Direction(simde__m128 p) const {
    return MulAdd(Splat<0>(p),x,MulAdd(Splat<1>(p),y,simde_mm_mul_ps(Splat<2>(p),z)));
  }
};
struct Constants {
  simde__m128 unpack_scale,unpack_bias,pack_scale,pack_bias,one;
  simde__m128i position_permutation,weight_permutation,weight_fill;
  Constants(const uint8_t* base,bool weighted)
      :unpack_scale(LoadVector(base+0x82000DC0)),
       unpack_bias(LoadVector(base+0x82000DD0)),
       pack_scale(LoadVector(base+0x82000DB0)),
       pack_bias(LoadVector(base+0x820A69C0)),
       one(weighted?LoadVector(base+0x82000B40):simde_mm_setzero_ps()),
       position_permutation(simde_mm_castps_si128(LoadVector(base+0x824BFC60))),
       weight_permutation(weighted?simde_mm_castps_si128(LoadVector(base+0x84E9C140)):simde_mm_setzero_si128()),
       weight_fill(weighted?simde_mm_castps_si128(LoadVector(base+0x82000B20)):simde_mm_setzero_si128()) {}
  simde__m128 Weight(const uint8_t* base,uint32_t address) const {
    const auto left=simde_mm_shuffle_epi8(
        simde_mm_load_si128(reinterpret_cast<const simde__m128i*>(base+(address&~15u))),
        simde_mm_load_si128(reinterpret_cast<const simde__m128i*>(VectorMaskL+(address&15)*16)));
    const auto word=rex::ppc::simde_mm_perm_epi8_(left,weight_fill,weight_permutation);
    return simde_mm_mul_ps(rex::ppc::simde_mm_cvtepu32_ps_(word),
                          simde_mm_castsi128_ps(simde_mm_set1_epi32(0x37800000)));
  }
};

// Inputs/outputs and all matrix addresses must be 16-byte aligned, as the PPC
// lvx/stvx operations require. Guest pointer and overlap checks belong to the
// caller. Constants are read from the live guest; runtime-initialized masks are
// not replaced with assumptions from the static executable image.
template<unsigned Bones>
void RunTo(uint8_t* base,uint32_t input,uint32_t blend,uint32_t count,
           uint32_t matrices,uint8_t* output) {
  static_assert(Bones>=1 && Bones<=4);
  const Constants c(base,Bones>1);
  for (uint32_t n=0;n<count;++n,input+=32,output+=32,blend+=4*Bones-2) {
    const auto position=LoadVector(base+input);
    const auto tail=LoadVector(base+input+16);
    const Matrix first(base+matrices+Load16(base+blend));
    auto p=first.Position(position);
    if constexpr (Bones>1) {
      auto sum=simde_mm_setzero_ps(), blended=simde_mm_setzero_ps();
      for (unsigned i=1;i<Bones;++i) {
        const auto weight=c.Weight(base,blend+4*i);
        const Matrix m(base+matrices+Load16(base+blend+4*i-2));
        const auto transformed=m.Position(position);
        if (i==1) {
          sum=weight; blended=simde_mm_mul_ps(transformed,weight);
        } else {
          sum=simde_mm_add_ps(sum,weight);
          blended=MulAdd(weight,transformed,blended);
        }
      }
      p=MulAdd(simde_mm_sub_ps(c.one,sum),p,blended);
    }
    const auto normal=first.Direction(MulAdd(Unpack(Bits(tail)),c.unpack_scale,c.unpack_bias));
    const auto tangent=first.Direction(MulAdd(Unpack(Bits(Splat<2>(tail))),c.unpack_scale,c.unpack_bias));
    // Keep the original UV/other tail words, replace just the packed directions.
    auto packed=simde_mm_castps_si128(tail);
    packed=simde_mm_insert_epi32(packed,Pack(MulAdd(normal,c.pack_scale,c.pack_bias)),0);
    packed=simde_mm_insert_epi32(packed,Pack(MulAdd(tangent,c.pack_scale,c.pack_bias)),1);
    const auto merged=rex::ppc::simde_mm_perm_epi8_(simde_mm_castps_si128(p),
        simde_mm_castps_si128(Splat<3>(position)),c.position_permutation);
    StoreVector(output,simde_mm_castsi128_ps(merged));
    StoreVector(output+16,simde_mm_castsi128_ps(packed));
  }
}
template<unsigned Bones>
void Run(uint8_t* base,uint32_t input,uint32_t blend,uint32_t count,
         uint32_t matrices,uint32_t output) {
  RunTo<Bones>(base,input,blend,count,matrices,base+output);
}
} // namespace cod6::skinning
