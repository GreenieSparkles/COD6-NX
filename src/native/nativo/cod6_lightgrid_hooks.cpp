// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include <rex/hook.h>  // COD5_DEAD_HOOKS: REX_FUNC lives here
#include "cod6_lightgrid_pack.h"
#include "cod6_lightgrid_hooks.h"
#include "cod6_page_writes.h"
#include <atomic>
#include <rex/cvar.h>

REXCVAR_DEFINE_BOOL(cod6_lightgrid_pack, false, "COD5",
                    "Fixed-XEX RGB light-grid brick packing with exact GPR effects and sampled comparison")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_lightgrid_pack_verify_only, true, "COD5",
                    "Compare all light-grid packing calls while retaining the original output")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_lightgrid_pack_diagnostics, false, "COD5",
                    "Count this one light-grid writer and its last arguments; does not establish complete writer coverage")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace {
bool requested=false,verify_only=true,diagnostics=false;
std::atomic<bool> healthy{true};
std::atomic<uint64_t> calls{0},compared{0},fast{0},fallbacks{0},mismatches{0};
std::atomic<uint64_t> validated{0};
std::atomic<uint64_t> last_arguments{0};
}

void cod6::lightgrid::Configure() {
  requested=REXCVAR_GET(cod6_lightgrid_pack);
  verify_only=REXCVAR_GET(cod6_lightgrid_pack_verify_only);
  diagnostics=REXCVAR_GET(cod6_lightgrid_pack_diagnostics);
  REXLOG_INFO("[lightgrid] native pack {}, verify-only {}, diagnostics {}; first 256 calls and every 128th compared; one writer only",
              requested,verify_only,diagnostics);
}
cod6::lightgrid::Counters cod6::lightgrid::ReadCounters() {
  const uint64_t arguments=last_arguments.load(std::memory_order_relaxed);
  return {calls.load(std::memory_order_relaxed),compared.load(std::memory_order_relaxed),
          fast.load(std::memory_order_relaxed),fallbacks.load(std::memory_order_relaxed),
          mismatches.load(std::memory_order_relaxed),uint32_t(arguments>>32),uint32_t(arguments)};
}

extern "C" REX_FUNC(__imp__sub_82429DE0);
extern "C" REX_FUNC(sub_82429DE0) {
  const bool writer_supported=!cod6::writes::Enabled() || [&] {
    static const bool supported=cod6::lightgrid::MatchesCode(base+cod6::lightgrid::kAddress);
    return supported;
  }();
  cod6::writes::GuestWriteScope write(ctx.r5.u32,cod6::lightgrid::kDestinationSpan,writer_supported,
                                     ctx.r5.u32>=0xE0000000u?4096:0);
  if(!requested && !diagnostics) { __imp__sub_82429DE0(ctx,base);return; }
  const uint64_t ticket=calls.fetch_add(1,std::memory_order_relaxed);
  const uint32_t source=ctx.r3.u32,destination=ctx.r5.u32;
  last_arguments.store((uint64_t(source)<<32)|destination,std::memory_order_relaxed);
  if(!requested) { __imp__sub_82429DE0(ctx,base);return; }
  static const bool supported=[&] {
    const bool match=cod6::lightgrid::MatchesCode(base+cod6::lightgrid::kAddress);
    if(!match) REXLOG_ERROR("[lightgrid] full writer code fingerprint mismatch; using original packing");
    return match;
  }();
  if(!supported || !healthy.load(std::memory_order_relaxed) ||
     !cod6::lightgrid::ValidRanges(source,destination)) {
    fallbacks.fetch_add(1,std::memory_order_relaxed);
    __imp__sub_82429DE0(ctx,base);return;
  }
  const uint8_t alpha=uint8_t(ctx.r4.u32);
  if(verify_only || validated.load(std::memory_order_relaxed)<256 || !(ticket%128)) {
    std::array<uint32_t,64> expected;
    const auto tail=cod6::lightgrid::PackTo(base+source,alpha,
        [&](unsigned i,uint32_t,uint32_t word) { expected[i]=word; });
    __imp__sub_82429DE0(ctx,base);
    bool match=cod6::lightgrid::MatchesTail(ctx,tail);
    for(unsigned i=0;i<expected.size();++i)
      match &= expected[i]==cod6::lightgrid::LoadWord(base+destination+cod6::lightgrid::DestinationOffset(i));
    compared.fetch_add(1,std::memory_order_relaxed);
    if(match) validated.fetch_add(1,std::memory_order_relaxed);
    else {
      mismatches.fetch_add(1,std::memory_order_relaxed);
      healthy.store(false,std::memory_order_relaxed);
      REXLOG_ERROR("[lightgrid] output/register mismatch; retained original packing and disabled candidate");
    }
    return;
  }
  const auto tail=cod6::lightgrid::PackTo(base+source,alpha,
      [&](unsigned,uint32_t offset,uint32_t word) { cod6::lightgrid::StoreWord(base+destination+offset,word); });
  cod6::lightgrid::ApplyTail(ctx,tail);
  fast.fetch_add(1,std::memory_order_relaxed);
}
