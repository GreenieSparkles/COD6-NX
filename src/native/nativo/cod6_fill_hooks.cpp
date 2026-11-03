// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include <rex/hook.h>  // COD5_DEAD_HOOKS: REX_FUNC lives here
#include "cod6_fill_policy.h"
#include "cod6_fill_hooks.h"
#include <rex/cvar.h>
#include <type_traits>

REXCVAR_DEFINE_BOOL(cod6_native_zero_fill, false, "COD5",
                    "Fixed-XEX aligned physical zero fill with exact context comparison and original fallback")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_native_zero_fill_verify_only, true, "COD5",
                    "Retain original zero-fill output and compare its full context on every eligible call")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
namespace {
bool zero_requested=false,zero_verify_only=true;
std::atomic<bool> zero_healthy{true};
std::atomic<uint64_t> observed_fill{0},observed_block{0},zero_calls{0},zero_bytes{0},zero_compared{0},zero_fast{0},zero_fallbacks{0},zero_mismatches{0},zero_validated{0};
}
void cod6::fill::Configure() {
  zero_requested=REXCVAR_GET(cod6_native_zero_fill);
  zero_verify_only=REXCVAR_GET(cod6_native_zero_fill_verify_only);
  REXLOG_INFO("[fill] native aligned physical zero {}, verify-only {}; first 256 eligible calls and every 128th compared",
              zero_requested,zero_verify_only);
}
cod6::fill::Counters cod6::fill::ReadCounters() {
  return {observed_fill.load(),observed_block.load(),zero_calls.load(),zero_bytes.load(),zero_compared.load(),
          zero_fast.load(),zero_fallbacks.load(),zero_mismatches.load()};
}

extern "C" REX_FUNC(__imp__sub_8237AB98);
extern "C" REX_FUNC(sub_8237AB98) {
  if(!zero_requested && !cod6::writes::Enabled()) {__imp__sub_8237AB98(ctx,base);return;}
  static const bool supported=[&] {
    const bool ok=cod6::fill::Matches(base,cod6::fill::kFill);
    if(!ok)REXLOG_ERROR("[fill] full fill code fingerprint mismatch; original retained");
    return ok;
  }();
  cod6::writes::GuestWriteScope write(ctx.r3.u32,ctx.r5.u32,supported,128);
  if(cod6::writes::Enabled())observed_fill.fetch_add(1,std::memory_order_relaxed);
  if(!zero_requested) {__imp__sub_8237AB98(ctx,base);return;}
  if(!supported || !zero_healthy.load(std::memory_order_relaxed) || !cod6::fill::CanZero(ctx,base)) {
    zero_fallbacks.fetch_add(1,std::memory_order_relaxed);__imp__sub_8237AB98(ctx,base);return;
  }
  const uint32_t destination=ctx.r3.u32,bytes=ctx.r5.u32;
  const uint64_t ticket=zero_calls.fetch_add(1,std::memory_order_relaxed);
  zero_bytes.fetch_add(bytes,std::memory_order_relaxed);
  if(zero_verify_only || zero_validated.load(std::memory_order_relaxed)<256 || !(ticket%128)) {
    static_assert(std::is_trivially_copyable_v<PPCContext>);
    PPCContext expected;
    // Copy padding too: full-context comparison must not report unspecified
    // memberwise-copy padding as a register mismatch.
    std::memcpy(static_cast<void*>(&expected),&ctx,sizeof(ctx));
    cod6::fill::ZeroTail(expected,reinterpret_cast<const uint8_t*>(VectorShiftTableL));
    __imp__sub_8237AB98(ctx,base);
    const bool match=!std::memcmp(&ctx,&expected,sizeof(ctx)) && cod6::fill::IsZero(base+destination,bytes);
    zero_compared.fetch_add(1,std::memory_order_relaxed);
    if(match)zero_validated.fetch_add(1,std::memory_order_relaxed);
    else {
      zero_mismatches.fetch_add(1,std::memory_order_relaxed);zero_healthy.store(false,std::memory_order_relaxed);
      REXLOG_ERROR("[fill] zero output/context mismatch; original retained and native zero fill disabled");
    }
    return;
  }
  std::memset(base+destination,0,bytes);
  cod6::fill::ZeroTail(ctx,reinterpret_cast<const uint8_t*>(VectorShiftTableL));
  zero_fast.fetch_add(1,std::memory_order_relaxed);
}

extern "C" REX_FUNC(__imp__sub_8237AD48);
extern "C" REX_FUNC(sub_8237AD48) {
  if(!cod6::writes::Enabled()) {__imp__sub_8237AD48(ctx,base);return;}
  static const bool supported=[&] {
    const bool ok=cod6::fill::Matches(base,cod6::fill::kBlockFill);
    if(!ok)REXLOG_ERROR("[fill] block fill fingerprint mismatch; original retained");
    return ok;
  }();
  cod6::writes::GuestWriteScope write(ctx.r3.u32,ctx.r5.u32,supported,128);
  observed_block.fetch_add(1,std::memory_order_relaxed);
  __imp__sub_8237AD48(ctx,base);
}
