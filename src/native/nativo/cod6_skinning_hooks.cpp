// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include <rex/hook.h>  // COD5_DEAD_HOOKS: REX_FUNC lives here
#include "cod6_skinning.h"
#include "cod6_skinning_policy.h"
#include "cod6_skinning_hooks.h"
#include "cod6_page_writes.h"
#include <atomic>
#include <new>
#include <vector>
#include <rex/cvar.h>

REXCVAR_DEFINE_BOOL(cod6_cpu_skinning, false, "COD5",
                    "Use fixed-XEX local-register 1-4 bone skinning with generated-output comparison and fallback")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_cpu_skinning_verify_only, true, "COD5",
                    "Compare every native skinning output; retain generated output for hardware validation")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace {
bool requested=false,verify_only=true;
std::atomic<bool> healthy{true};
std::atomic<uint64_t> calls{0},vertices{0},compared{0},fast{0},fallbacks{0},mismatches{0};
std::array<std::atomic<uint64_t>,4> by_bones{},validated{},kernel_calls{};
using Original=void(*)(PPCContext&,uint8_t*);

template<unsigned Bones>
void Skin(PPCContext& ctx,uint8_t* base,Original original) {
  const bool writer_supported=!cod6::writes::Enabled() || [&] {
    static const bool supported=cod6::skinning::MatchesKernel(
        base+cod6::skinning::kKernels[Bones-1].address,cod6::skinning::kKernels[Bones-1]);
    return supported;
  }();
  // Existing scalar/native helpers and generated PPC use different pointer
  // construction sites. Until E-view handling is separately verified there,
  // conservatively cover both the raw-view and +4KB physical destinations.
  cod6::writes::GuestWriteScope write(ctx.r7.u32,uint64_t(ctx.r5.u32)*32,writer_supported,
                                     ctx.r7.u32>=0xE0000000u?4096:0);
  if(!requested || !healthy.load(std::memory_order_relaxed)) { original(ctx,base);return; }
  static const bool supported=[&] {
    const bool match=cod6::skinning::MatchesKernel(
        base+cod6::skinning::kKernels[Bones-1].address,cod6::skinning::kKernels[Bones-1]);
    if(!match) {
      healthy.store(false,std::memory_order_relaxed);
      REXLOG_ERROR("[skinning] {}-bone full code fingerprint mismatch; using original kernels",Bones);
    }
    return match;
  }();
  const uint32_t input=ctx.r3.u32,blend=ctx.r4.u32,count=ctx.r5.u32;
  const uint32_t matrices=ctx.r6.u32,output=ctx.r7.u32;
  if(!supported || !cod6::skinning::ValidRanges(Bones,input,blend,count,matrices,output)) {
    fallbacks.fetch_add(1,std::memory_order_relaxed);original(ctx,base);return;
  }
  // Reject misaligned matrix offsets before changing any output. PPC lvx rounds
  // each address; the optimized contract deliberately accepts only aligned jobs.
  for(uint32_t n=0;n<count;++n) for(unsigned i=0;i<Bones;++i) {
    const uint32_t offset=i?4*i-2:0;
    if(cod6::skinning::Load16(base+blend+n*(4*Bones-2)+offset)&15) {
      fallbacks.fetch_add(1,std::memory_order_relaxed);original(ctx,base);return;
    }
  }
  calls.fetch_add(1,std::memory_order_relaxed);
  vertices.fetch_add(count,std::memory_order_relaxed);
  by_bones[Bones-1].fetch_add(count,std::memory_order_relaxed);
  const uint64_t ticket=kernel_calls[Bones-1].fetch_add(1,std::memory_order_relaxed);
  const bool compare=verify_only || validated[Bones-1].load(std::memory_order_relaxed)<20000 || !(ticket%128);
  ctx.fpscr.enableFlushMode();
  if(compare) {
    // Per-thread, aligned output storage. Compute before the original so an
    // in-place original cannot destroy the input needed by the comparison.
    struct alignas(16) HalfVertex { uint32_t words[4]; };
    thread_local std::vector<HalfVertex> scratch;
    try { scratch.resize(size_t(count)*2); }
    catch(const std::bad_alloc&) {
      fallbacks.fetch_add(1,std::memory_order_relaxed);original(ctx,base);return;
    }
    auto* bytes=reinterpret_cast<uint8_t*>(scratch.data());
    cod6::skinning::RunTo<Bones>(base,input,blend,count,matrices,bytes);
    original(ctx,base);
    compared.fetch_add(count,std::memory_order_relaxed);
    if(std::memcmp(bytes,base+output,size_t(count)*32)) {
      mismatches.fetch_add(1,std::memory_order_relaxed);
      healthy.store(false,std::memory_order_relaxed);
      REXLOG_ERROR("[skinning] {}-bone output mismatch ({} vertices); retained original output and disabled native CPU skinning",Bones,count);
    } else validated[Bones-1].fetch_add(count,std::memory_order_relaxed);
    return;
  }
  cod6::skinning::Run<Bones>(base,input,blend,count,matrices,output);
  fast.fetch_add(count,std::memory_order_relaxed);
  // Preserve the original argument advancement and leaf LR/CR effects in
  // addition to the ABI's nonvolatile state; other volatile scratch is unused.
  ctx.r3.s64+=int64_t(count)*32;ctx.r4.s64+=int64_t(count)*(4*Bones-2);ctx.r7.s64+=int64_t(count)*32;
  ctx.r5.s64=(Bones==1 || Bones==3)?32:48;
  ctx.r12.u64=ctx.lr;ctx.lr=cod6::skinning::kKernels[Bones-1].address+8;
  ctx.xer.ca=1;ctx.cr0.compare<int32_t>(0,0,ctx.xer);ctx.cr6.compare<uint32_t>(count,0,ctx.xer);
}
}

void cod6::skinning::Configure() {
  requested=REXCVAR_GET(cod6_cpu_skinning);
  verify_only=REXCVAR_GET(cod6_cpu_skinning_verify_only);
  REXLOG_INFO("[skinning] native CPU {}, verify-only {}; first 20000 vertices per kernel and every 128th call compared",requested,verify_only);
}
cod6::skinning::Counters cod6::skinning::ReadCounters() {
  Counters c;
  c.calls=calls.load(std::memory_order_relaxed);c.vertices=vertices.load(std::memory_order_relaxed);
  c.compared_vertices=compared.load(std::memory_order_relaxed);c.fast_vertices=fast.load(std::memory_order_relaxed);
  c.fallbacks=fallbacks.load(std::memory_order_relaxed);c.mismatches=mismatches.load(std::memory_order_relaxed);
  for(unsigned i=0;i<4;++i)c.vertices_by_bones[i]=by_bones[i].load(std::memory_order_relaxed);
  return c;
}
#define COD6_SKIN_HOOK(address,bones) \
  extern "C" REX_FUNC(__imp__sub_##address); \
  extern "C" REX_FUNC(sub_##address) { Skin<bones>(ctx,base,__imp__sub_##address); }
COD6_SKIN_HOOK(82446A98,1)
COD6_SKIN_HOOK(82446C28,2)
COD6_SKIN_HOOK(82446E18,3)
COD6_SKIN_HOOK(82447048,4)
#undef COD6_SKIN_HOOK
