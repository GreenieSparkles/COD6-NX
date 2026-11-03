// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include "cod6_handoff_diagnostics.h"
#include "cod6_main_wait.h"
#if COD6_DIAGNOSTICS
#include "cod6_handoff_policy.h"
#include <rex/cvar.h>
#include <rex/logging.h>
#include <array>
#include <atomic>
#include <chrono>

REXCVAR_DECLARE(bool, cod6_native_diagnostics);
namespace {
using namespace cod6::handoff;
static_assert(std::atomic<uint64_t>::is_always_lock_free);
bool enabled=false; // Configure before guest threads start; restart to change.
std::atomic<uint32_t> verified{0}; // 0=not observed, 1=matched, 2=mismatch
std::atomic<uint64_t> main_inflight{0},backend_inflight{0},unpartitioned{0},aborted{0};
struct Metric {
  std::atomic<uint64_t> calls{0},ns{0},max_ns{0};
  void Add(uint64_t elapsed) {
    ns.fetch_add(elapsed,std::memory_order_relaxed);
    uint64_t old=max_ns.load(std::memory_order_relaxed);
    while(old<elapsed && !max_ns.compare_exchange_weak(old,elapsed,std::memory_order_relaxed)) {}
    calls.fetch_add(1,std::memory_order_relaxed);
  }
};
enum Phase {MainHelp,BackendTotal,BeforeNotify,Notify,AfterNotify,PhaseCount};
std::array<Metric,PhaseCount> metrics;
thread_local Partition* active=nullptr;
uint64_t Now() {
  return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count());
}
bool Active(uint8_t* base) {
  if(!enabled)return false;
  static const bool supported=[&] {
    const bool matched=MatchesCode(base);
    verified.store(matched?1:2,std::memory_order_relaxed);
    REXLOG_INFO("[handoff] schema=1 code_verified={} hooks=3 original_calls_preserved=1",matched);
    return matched;
  }();
  return supported;
}
}

void cod6::handoff::Configure() {
  enabled=REXCVAR_GET(cod6_native_diagnostics);
  REXLOG_INFO("[handoff] schema=1 enabled={} clock=steady_ns scope=wall_time snapshot=nontransactional",enabled);
}
void cod6::handoff::Report(uint64_t sample,uint64_t frames) {
  cod6::main_wait::Report(sample,frames);
  if(!enabled)return;
  REXLOG_INFO("[handoff] schema=1 sample={} frames={} verified={} main_inflight={} backend_inflight={} unpartitioned={} aborted={}",
      sample,frames,verified.load(std::memory_order_relaxed),main_inflight.load(std::memory_order_relaxed),
      backend_inflight.load(std::memory_order_relaxed),unpartitioned.load(std::memory_order_relaxed),aborted.load(std::memory_order_relaxed));
  constexpr const char* names[]={"main_help","backend_total","before_notify","notify","after_notify"};
  for(unsigned i=0;i<PhaseCount;++i) {
    const auto& m=metrics[i];
    REXLOG_INFO("[handoff] sample={} phase={} calls={} ns={} max_ns={}",sample,names[i],
        m.calls.load(std::memory_order_relaxed),m.ns.load(std::memory_order_relaxed),m.max_ns.load(std::memory_order_relaxed));
  }
}

#endif

// These two wrappers also carry the Main wait scope/completion notification in
// production. Only the telemetry below is removed by COD6_DIAGNOSTICS.
extern "C" REX_FUNC(__imp__sub_8242CDC8);
extern "C" REX_FUNC(sub_8242CDC8) {
  cod6::main_wait::Scope scope(base,uint32_t(ctx.lr),ctx.r3.u32,ctx.r4.u32);
  cod6::main_wait::BackendScope backend_scope(base,uint32_t(ctx.lr),ctx.r3.u32,ctx.r4.u32);
#if COD6_DIAGNOSTICS
  if(!MainCall(uint32_t(ctx.lr),ctx.r3.u32,ctx.r4.u32) || !Active(base)) {
    __imp__sub_8242CDC8(ctx,base);return;
  }
  main_inflight.fetch_add(1,std::memory_order_relaxed);
  const auto begin=Now();
  try {__imp__sub_8242CDC8(ctx,base);}
  catch(...) {
    main_inflight.fetch_sub(1,std::memory_order_relaxed);
    aborted.fetch_add(1,std::memory_order_relaxed);throw;
  }
  metrics[MainHelp].Add(Now()-begin);
  main_inflight.fetch_sub(1,std::memory_order_relaxed);
#else
  __imp__sub_8242CDC8(ctx,base);
#endif
}
#if COD6_DIAGNOSTICS
extern "C" REX_FUNC(__imp__sub_82426578);
extern "C" REX_FUNC(sub_82426578) {
  if(!BackendCall(uint32_t(ctx.lr)) || !Active(base)) {
    __imp__sub_82426578(ctx,base);return;
  }
  // TLS associates the notification only with its enclosing Backend call.
  // No guest event query, retained kernel object or new guest synchronization.
  Partition span{Now()};
  auto* previous=active;active=&span;
  backend_inflight.fetch_add(1,std::memory_order_relaxed);
  try {__imp__sub_82426578(ctx,base);}
  catch(...) {
    active=previous;
    backend_inflight.fetch_sub(1,std::memory_order_relaxed);
    aborted.fetch_add(1,std::memory_order_relaxed);throw;
  }
  const auto end=Now();
  active=previous;
  metrics[BackendTotal].Add(end-span.begin);
  if(span.Valid(end)) {
    metrics[BeforeNotify].Add(span.notify_begin-span.begin);
    metrics[Notify].Add(span.notify_end-span.notify_begin);
    metrics[AfterNotify].Add(end-span.notify_end);
  } else unpartitioned.fetch_add(1,std::memory_order_relaxed);
  backend_inflight.fetch_sub(1,std::memory_order_relaxed);
}
#endif
extern "C" REX_FUNC(__imp__sub_82241880);
extern "C" REX_FUNC(sub_82241880) {
#if COD6_DIAGNOSTICS
  auto* span=active;
  if(span && uint32_t(ctx.lr)!=0x82426608)span=nullptr;
  if(span)span->NotifyBegin(Now());
#endif
  __imp__sub_82241880(ctx,base);
  cod6::main_wait::Notify(base,cod6::main_wait::Reason::BackendComplete);
#if COD6_DIAGNOSTICS
  if(span)span->NotifyEnd(Now());
#endif
}
