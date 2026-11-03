// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include "cod6_main_wait.h"
#include "cod6_main_wait_policy.h"
#include "cod6_backend_wait_policy.h"
#include "cod6_ppc_interrupts.h"
#include <rex/cvar.h>
#if COD6_DIAGNOSTICS
#include <chrono>
#define COD6_MAIN_NOTE(...) do {if(diagnostics_enabled) {__VA_ARGS__;}} while(0)
#else
#define COD6_MAIN_NOTE(...) do {} while(0)
#endif

REXCVAR_DEFINE_BOOL(cod6_main_idle_wait,true,"COD5",
    "Bounded wait only after original Main helping has no progress and all queues are empty; false restores original polling")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_main_idle_wait_us,100,"COD5",
    "Main idle wait timeout in microseconds; original predicate and helping resume after every wake or timeout")
    .allowed({"50","100","200"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_backend_idle_wait,false,"COD5",
    "Candidate: bounded wait after the fixed COD5 Backend pending query and a complete empty helping scan")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_backend_idle_wait_us,50,"COD5",
    "Backend pending wait timeout in microseconds; the original predicate and helping loop always resume")
    .allowed({"50","100"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);
#if COD6_DIAGNOSTICS
REXCVAR_DECLARE(bool,cod6_native_diagnostics);
#endif
extern "C" uint32_t svcWaitForAddress(void*,uint32_t,int64_t,int64_t);
extern "C" uint32_t svcSignalToAddress(void*,uint32_t,int32_t,int32_t);

namespace {
static_assert(std::atomic<uint64_t>::is_always_lock_free);
bool enabled=false;
uint32_t bound_us=100;
std::atomic<bool> failed{false};
cod6::workers::WakeGate gate; // independent of the two worker slots; use slot 0
thread_local bool main_scope=false,help_active=false;
bool backend_enabled=false;
uint32_t backend_bound_us=50;
std::atomic<bool> backend_failed{false};
cod6::workers::WakeGate backend_gate;
thread_local bool backend_scope=false,backend_help_active=false;
#if COD6_DIAGNOSTICS
bool diagnostics_enabled=false;
using Clock=std::chrono::steady_clock;
std::atomic<uint32_t> verified{0},last_error{0};
std::atomic<uint64_t> scopes{0},scans{0},ready{0},queued{0},unsafe{0},changed{0},parks{0},timeouts{0},errors{0},sleep_ns{0},late{0};
std::array<std::atomic<uint64_t>,size_t(cod6::main_wait::Reason::Count)> notices{};
std::atomic<uint32_t> backend_verified{0},backend_last_error{0};
std::atomic<uint64_t> backend_scopes{0},backend_scans{0},backend_ready{0},backend_queued{0},
    backend_unsafe{0},backend_changed{0},backend_parks{0},backend_timeouts{0},
    backend_errors{0},backend_sleep_ns{0},backend_late{0};
std::array<std::atomic<uint64_t>,size_t(cod6::main_wait::Reason::Count)> backend_notices{};
#endif
bool Active(uint8_t* base) {
  if(!enabled || failed.load(std::memory_order_relaxed))return false;
  static const bool supported=[&] {
    const bool ok=cod6::main_wait::MatchesCode(base);
    COD6_MAIN_NOTE(verified.store(ok?1:2,std::memory_order_relaxed));
    return ok;
  }();
  return supported;
}
void Fail(uint32_t error) {
  failed.store(true,std::memory_order_relaxed);
  COD6_MAIN_NOTE(errors.fetch_add(1,std::memory_order_relaxed);last_error.store(error,std::memory_order_relaxed));
  (void)error;
}
bool BackendActive(uint8_t* base) {
  if(!backend_enabled || backend_failed.load(std::memory_order_relaxed))return false;
  static const bool supported=[&] {
    const bool ok=cod6::backend_wait::MatchesCode(base);
    COD6_MAIN_NOTE(backend_verified.store(ok?1:2,std::memory_order_relaxed));
    return ok;
  }();
  return supported;
}
void BackendFail(uint32_t error) {
  backend_failed.store(true,std::memory_order_relaxed);
  COD6_MAIN_NOTE(backend_errors.fetch_add(1,std::memory_order_relaxed);
    backend_last_error.store(error,std::memory_order_relaxed));
  (void)error;
}
uint32_t LoadQueue(const uint8_t* base,uint32_t address) {
  return __builtin_bswap32(__atomic_load_n(reinterpret_cast<const uint32_t*>(base+address),__ATOMIC_ACQUIRE));
}
}

void cod6::main_wait::Configure() {
  const bool wait_requested=REXCVAR_GET(cod6_main_idle_wait);
  enabled=wait_requested && cod6::cpu::ThreadLocalInterruptMode();
  const auto requested=REXCVAR_GET(cod6_main_idle_wait_us);
  bound_us=(requested==50 || requested==100 || requested==200)?requested:100;
  if(wait_requested && !enabled) {
    REXLOG_INFO("[main-wait-config] request disabled: audited thread-local interrupt mode is inactive");
  }
  const bool backend_requested=REXCVAR_GET(cod6_backend_idle_wait);
  backend_enabled=backend_requested && cod6::cpu::ThreadLocalInterruptMode();
  const auto backend_requested_us=REXCVAR_GET(cod6_backend_idle_wait_us);
  backend_bound_us=(backend_requested_us==50 || backend_requested_us==100)?backend_requested_us:50;
  if(backend_requested && !backend_enabled) {
    REXLOG_INFO("[backend-wait-config] request disabled: audited thread-local interrupt mode is inactive");
  }
#if COD6_DIAGNOSTICS
  diagnostics_enabled=REXCVAR_GET(cod6_native_diagnostics);
  COD6_MAIN_NOTE(REXLOG_INFO("[main-wait] schema=1 enabled={} bound_us={} outer=8242CDC8 caller=8241925C helper=8242CCA8 completion=82241880 idle_only=1 original_scan=1",enabled,bound_us));
  COD6_MAIN_NOTE(REXLOG_INFO("[backend-wait] schema=1 enabled={} bound_us={} outer=8242CDC8 caller=82425748 predicate=824256A8 helper=8242CCA8 ring_notify=interrupt-return idle_only=1 original_scan=1",backend_enabled,backend_bound_us));
#endif
}
cod6::main_wait::Scope::Scope(uint8_t* base,uint32_t lr,uint32_t predicate,uint32_t repeat) {
  if(main_scope || !cod6::handoff::MainCall(lr,predicate,repeat) || !Active(base) || !gate.Claim(0))return;
  owned_=true;main_scope=true;
  COD6_MAIN_NOTE(scopes.fetch_add(1,std::memory_order_relaxed));
}
cod6::main_wait::Scope::~Scope() {
  if(!owned_)return;
  main_scope=false;gate.Release(0);
}
void cod6::main_wait::Notify(uint8_t* base,Reason reason) {
  if(reason!=Reason::RingProgress && Active(base)) {
    COD6_MAIN_NOTE(notices[size_t(reason)].fetch_add(1,std::memory_order_relaxed));
    gate.Notify([](std::atomic<uint32_t>& word) {
      const auto result=svcSignalToAddress(&word,0,0,1);
      if(result)Fail(result);
    });
  }
  if(BackendActive(base)) {
    COD6_MAIN_NOTE(backend_notices[size_t(reason)].fetch_add(1,std::memory_order_relaxed));
    backend_gate.Notify([](std::atomic<uint32_t>& word) {
      const auto result=svcSignalToAddress(&word,0,0,1);
      if(result)BackendFail(result);
    });
  }
}
cod6::main_wait::BackendScope::BackendScope(uint8_t* base,uint32_t lr,uint32_t predicate,uint32_t repeat) {
  if(backend_scope || !cod6::backend_wait::Call(lr,predicate,repeat) ||
     !BackendActive(base) || !backend_gate.Claim(0))return;
  owned_=true;backend_scope=true;
  COD6_MAIN_NOTE(backend_scopes.fetch_add(1,std::memory_order_relaxed));
}
cod6::main_wait::BackendScope::~BackendScope() {
  if(!owned_)return;
  backend_scope=false;backend_gate.Release(0);
}
#if COD6_DIAGNOSTICS
void cod6::main_wait::Report(uint64_t sample,uint64_t frames) {
  if(!diagnostics_enabled)return;
  REXLOG_INFO("[main-wait] schema=1 sample={} frames={} enabled={} bound_us={} verified={} failed={} scopes={} scans={} ready={} queued={} unsafe={} changed={} parks={} timeouts={} errors={} last_error={} sleep_ns={} late={} cumulative=1",
      sample,frames,enabled,bound_us,verified.load(),failed.load(),scopes.load(),scans.load(),ready.load(),queued.load(),unsafe.load(),changed.load(),parks.load(),timeouts.load(),errors.load(),last_error.load(),sleep_ns.load(),late.load());
  REXLOG_INFO("[main-wait-notify] schema=1 sample={} publication={} dispatch={} consumption={} worker_event={} backend_complete={} cumulative=1",
      sample,notices[0].load(),notices[1].load(),notices[2].load(),notices[3].load(),notices[4].load());
  REXLOG_INFO("[backend-wait] schema=1 sample={} frames={} enabled={} bound_us={} verified={} failed={} scopes={} scans={} ready={} queued={} unsafe={} changed={} parks={} timeouts={} errors={} last_error={} sleep_ns={} late={} cumulative=1",
      sample,frames,backend_enabled,backend_bound_us,backend_verified.load(),backend_failed.load(),
      backend_scopes.load(),backend_scans.load(),backend_ready.load(),backend_queued.load(),
      backend_unsafe.load(),backend_changed.load(),backend_parks.load(),backend_timeouts.load(),
      backend_errors.load(),backend_last_error.load(),backend_sleep_ns.load(),backend_late.load());
  REXLOG_INFO("[backend-wait-notify] schema=1 sample={} publication={} dispatch={} consumption={} worker_event={} backend_complete={} ring_progress={} cumulative=1",
      sample,backend_notices[0].load(),backend_notices[1].load(),backend_notices[2].load(),
      backend_notices[3].load(),backend_notices[4].load(),backend_notices[5].load());
}
#endif

extern "C" REX_FUNC(__imp__sub_8242CCA8);
extern "C" REX_FUNC(sub_8242CCA8) {
  if(backend_scope && !backend_help_active && ctx.lr==0x8242CDFC &&
     ctx.r3.u32==0x824256A8 && BackendActive(base)) {
    const auto seen=backend_gate.Snapshot();
    struct Helping {Helping(){backend_help_active=true;}~Helping(){backend_help_active=false;}} helping;
    __imp__sub_8242CCA8(ctx,base);
    COD6_MAIN_NOTE(backend_scans.fetch_add(1,std::memory_order_relaxed));
    if(ctx.r3.u32!=0) {COD6_MAIN_NOTE(backend_ready.fetch_add(1,std::memory_order_relaxed));return;}
    if(backend_failed.load(std::memory_order_relaxed) ||
       !cod6::cpu::ThreadLocalInterruptMode() || cod6::cpu::InterruptDepth()!=0 ||
       !(ctx.msr&0x8000)) {
      COD6_MAIN_NOTE(backend_unsafe.fetch_add(1,std::memory_order_relaxed));return;
    }
    if(backend_gate.Snapshot()!=seen) {
      COD6_MAIN_NOTE(backend_changed.fetch_add(1,std::memory_order_relaxed));return;
    }
    if(!cod6::workers::QueuesIdle([&](uint32_t a){return LoadQueue(base,a);})) {
      COD6_MAIN_NOTE(backend_queued.fetch_add(1,std::memory_order_relaxed));return;
    }
#if COD6_DIAGNOSTICS
    const auto begin=diagnostics_enabled?Clock::now():Clock::time_point{};
#endif
    const auto result=backend_gate.Park(0,seen,int64_t(backend_bound_us)*1000,
      [](std::atomic<uint32_t>& word,uint32_t expected,int64_t ns) {
        return svcWaitForAddress(&word,2,int64_t(expected),ns);
      });
    COD6_MAIN_NOTE(if(result.changed)backend_changed.fetch_add(1,std::memory_order_relaxed));
    if(result.waited) {
      COD6_MAIN_NOTE(backend_parks.fetch_add(1,std::memory_order_relaxed);
        if(result.platform_result==0xEA01)backend_timeouts.fetch_add(1,std::memory_order_relaxed);
        const auto elapsed=uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-begin).count());
        backend_sleep_ns.fetch_add(elapsed,std::memory_order_relaxed);
        if(elapsed>uint64_t(backend_bound_us)*1000+1000000)backend_late.fetch_add(1,std::memory_order_relaxed));
      if(result.platform_result && result.platform_result!=0xEA01 &&
         result.platform_result!=0xFA01)BackendFail(result.platform_result);
    }
    return;
  }
  if(!main_scope || help_active || ctx.lr!=0x8242CDFC || ctx.r3.u32!=0x82240C00 || !Active(base)) {
    __imp__sub_8242CCA8(ctx,base);return;
  }
  const auto seen=gate.Snapshot();
  struct Helping {Helping(){help_active=true;}~Helping(){help_active=false;}} helping;
  __imp__sub_8242CCA8(ctx,base); // preserve all predicates, callbacks and return registers
  COD6_MAIN_NOTE(scans.fetch_add(1,std::memory_order_relaxed));
  if(ctx.r3.u32!=0) {COD6_MAIN_NOTE(ready.fetch_add(1,std::memory_order_relaxed));return;}
  if(failed.load(std::memory_order_relaxed) || !cod6::cpu::ThreadLocalInterruptMode() ||
      cod6::cpu::InterruptDepth()!=0 || !(ctx.msr&0x8000)) {
    COD6_MAIN_NOTE(unsafe.fetch_add(1,std::memory_order_relaxed));return;
  }
  // Includes successful consumption during this very helper. No additional
  // dependency or completion-event query is allowed in the observation path.
  if(gate.Snapshot()!=seen) {COD6_MAIN_NOTE(changed.fetch_add(1,std::memory_order_relaxed));return;}
  if(!cod6::workers::QueuesIdle([&](uint32_t a){return LoadQueue(base,a);})) {
    COD6_MAIN_NOTE(queued.fetch_add(1,std::memory_order_relaxed));return;
  }
#if COD6_DIAGNOSTICS
  const auto begin=diagnostics_enabled?Clock::now():Clock::time_point{};
#endif
  const auto result=gate.Park(0,seen,int64_t(bound_us)*1000,
    [](std::atomic<uint32_t>& word,uint32_t expected,int64_t ns) {
      return svcWaitForAddress(&word,2,int64_t(expected),ns);
    });
  COD6_MAIN_NOTE(if(result.changed)changed.fetch_add(1,std::memory_order_relaxed));
  if(result.waited) {
    COD6_MAIN_NOTE(parks.fetch_add(1,std::memory_order_relaxed);
      if(result.platform_result==0xEA01)timeouts.fetch_add(1,std::memory_order_relaxed);
      const auto elapsed=uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-begin).count());
      sleep_ns.fetch_add(elapsed,std::memory_order_relaxed);
      if(elapsed>uint64_t(bound_us)*1000+1000000)late.fetch_add(1,std::memory_order_relaxed));
    if(result.platform_result && result.platform_result!=0xEA01 && result.platform_result!=0xFA01)Fail(result.platform_result);
  }
}
#undef COD6_MAIN_NOTE
