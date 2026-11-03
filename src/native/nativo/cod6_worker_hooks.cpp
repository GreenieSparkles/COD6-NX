// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
#include <rex/hook.h>  // COD5_DEAD_HOOKS: REX_FUNC lives here
// COD5-specific idle-worker gate; no guest event/queue state is replaced.
#include "cod6_worker_hooks.h"
#include "cod6_worker_policy.h"
#include "cod6_main_wait.h"
#include "cod6_ppc_interrupts.h"
#include "cod6_diagnostics.h"
#include <rex/cvar.h>
#if COD6_DIAGNOSTICS
#include <chrono>
#define COD6_WORKER_NOTE(...) do { __VA_ARGS__; } while (0)
#else
#define COD6_WORKER_NOTE(...) do {} while (0)
#endif

REXCVAR_DEFINE_BOOL(cod6_worker_idle_wait, true, "COD5",
                    "Experimental bounded notification waits only for the two verified idle workers; "
                    "false retains the original worker loop")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_worker_idle_wait_us, 100, "COD5",
                      "Maximum idle-worker address-arbiter wait before the original scan runs again")
    .allowed({"50","100","200"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_worker_stalled_wait, true, "COD5",
                    "With idle waits enabled, bound retries after four stable original worker scans with no consumption; "
                    "dependency changes without a notification are rechecked after the finite timeout")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

extern "C" uint32_t svcWaitForAddress(void*,uint32_t,int64_t,int64_t);
extern "C" uint32_t svcSignalToAddress(void*,uint32_t,int32_t,int32_t);

namespace {
using namespace cod6::workers;
#if COD6_DIAGNOSTICS
using Clock=std::chrono::steady_clock;
#endif
bool enabled=false;
bool stalled_enabled=false;
uint32_t bound_us=100;
std::atomic<bool> failed{false};
WakeGate gate;
thread_local int worker_slot=-1;
thread_local bool scanning=false;
thread_local uint64_t consumed_jobs=0;
thread_local StalledScans stalled_scans;
#if COD6_DIAGNOSTICS
std::atomic<uint64_t> scans{0},parks{0},changed{0},quiet_returns{0},queued{0},unsafe{0},unsupported{0};
std::atomic<uint64_t> notifications{0},wakes{0},sleep_ns{0},errors{0},late{0};
std::atomic<uint32_t> last_error{0};
std::atomic<uint64_t> inflight_scans{0},available_scans{0},negative_scans{0},stalled_parks{0};
#endif
bool Supported(uint8_t* base) {
  static const bool supported=[&] {
    const bool ok=MatchesWorkerCode(base);
    if(ok)REXLOG_INFO("[workers] fixed-XEX worker, queue publication and completion code verified");
    else {
      COD6_WORKER_NOTE(unsupported.fetch_add(1,std::memory_order_relaxed));
      REXLOG_ERROR("[workers] code fingerprint mismatch; retaining original worker loop");
    }
    return ok;
  }();
  return supported;
}
bool Active(uint8_t* base) {return enabled && !failed.load(std::memory_order_relaxed) && Supported(base);}
void Notify() {
  COD6_WORKER_NOTE(notifications.fetch_add(1,std::memory_order_relaxed));
  [[maybe_unused]] const uint32_t count=gate.Notify([](std::atomic<uint32_t>& word) {
    const uint32_t result=svcSignalToAddress(&word,0,0,1);
    if(result) {
      COD6_WORKER_NOTE(errors.fetch_add(1,std::memory_order_relaxed);last_error.store(result,std::memory_order_relaxed));
      failed.store(true,std::memory_order_relaxed);
    }
  });
  COD6_WORKER_NOTE(wakes.fetch_add(count,std::memory_order_relaxed));
}
uint32_t LoadQueue(const uint8_t* base,uint32_t address) {
  return __builtin_bswap32(__atomic_load_n(reinterpret_cast<const uint32_t*>(base+address),__ATOMIC_ACQUIRE));
}
struct WorkerScope {
  int previous=worker_slot;uint32_t slot;
  StalledScans previous_stalls=stalled_scans;
  explicit WorkerScope(uint32_t value):slot(value){worker_slot=int(slot);stalled_scans.Reset();}
  ~WorkerScope(){worker_slot=previous;stalled_scans=previous_stalls;gate.Release(slot);}
};
} // namespace

void cod6::workers::Configure() {
  const bool wait_requested=REXCVAR_GET(cod6_worker_idle_wait);
  enabled=wait_requested && cod6::cpu::ThreadLocalInterruptMode();
  stalled_enabled=enabled && REXCVAR_GET(cod6_worker_stalled_wait);
  const uint32_t requested_us=REXCVAR_GET(cod6_worker_idle_wait_us);
  bound_us=(requested_us==50 || requested_us==100 || requested_us==200)?requested_us:100;
  if(wait_requested && !enabled) {
    REXLOG_INFO("[workers] idle wait request disabled: audited thread-local interrupt mode is inactive");
  }
  REXLOG_INFO("[workers] idle wait {}, bound {} us; original scans and nonempty queues preserved",enabled,bound_us);
  REXLOG_INFO("[workers] stalled retries {}; four stable scans, no successful consumption, negative claims excluded; finite recheck",
              enabled && stalled_enabled);
}
cod6::workers::Counters cod6::workers::ReadCounters() {
#if COD6_DIAGNOSTICS
  return {enabled,scans.load(),parks.load(),changed.load(),quiet_returns.load(),queued.load(),unsafe.load(),unsupported.load(),
          notifications.load(),wakes.load(),sleep_ns.load(),errors.load(),late.load(),last_error.load(),
          inflight_scans.load(),available_scans.load(),negative_scans.load(),stalled_parks.load()};
#else
  return {};
#endif
}

extern "C" REX_FUNC(__imp__sub_8242CEA0);
extern "C" REX_FUNC(sub_8242CEA0) {
  const uint32_t entry_slot=ctx.r3.u32;
  if(!Active(base) || entry_slot<2 || entry_slot>3 || !gate.Claim(entry_slot-2)) {
    __imp__sub_8242CEA0(ctx,base);return;
  }
  WorkerScope scope(entry_slot-2);
  __imp__sub_8242CEA0(ctx,base);
}

extern "C" REX_FUNC(__imp__sub_8242CBD8);
extern "C" REX_FUNC(sub_8242CBD8) {
  const bool candidate=enabled && worker_slot>=0 && ctx.lr==kWorkerReturn && !scanning;
  if(!candidate || !Active(base)) {__imp__sub_8242CBD8(ctx,base);return;}
  // Sample BEFORE the original scan; a publication/completion during it must
  // prevent parking even if another consumer has already emptied that queue.
  const uint64_t seen=gate.Snapshot();
  const uint64_t consumed_before=consumed_jobs;
  struct ScanScope {ScanScope(){scanning=true;}~ScanScope(){scanning=false;}} scope;
  __imp__sub_8242CBD8(ctx,base);
  COD6_WORKER_NOTE(scans.fetch_add(1,std::memory_order_relaxed));
  if(failed.load(std::memory_order_relaxed) || !cod6::cpu::ThreadLocalInterruptMode() ||
      cod6::cpu::InterruptDepth()!=0 || !(ctx.msr&0x8000)) {
    stalled_scans.Reset();COD6_WORKER_NOTE(unsafe.fetch_add(1,std::memory_order_relaxed));return;
  }
  bool stalled=false;
  auto load=[&](uint32_t address){return LoadQueue(base,address);};
  if(stalled_enabled) {
    const auto queues=ObserveQueues(load);
    COD6_WORKER_NOTE(if(queues.negative)negative_scans.fetch_add(1,std::memory_order_relaxed);
    else if(queues.available)available_scans.fetch_add(1,std::memory_order_relaxed);
    else if(queues.outstanding)inflight_scans.fetch_add(1,std::memory_order_relaxed));
    stalled=stalled_scans.ShouldPark(queues,seen,gate.Snapshot(),consumed_before!=consumed_jobs);
    if(!queues.Idle() && !stalled) {COD6_WORKER_NOTE(queued.fetch_add(1,std::memory_order_relaxed));return;}
  } else if(!QueuesIdle(load)) {
    COD6_WORKER_NOTE(queued.fetch_add(1,std::memory_order_relaxed));return;
  }
#if COD6_DIAGNOSTICS
  const auto begin=Clock::now();
#endif
  const auto result=gate.Park(uint32_t(worker_slot),seen,int64_t(bound_us)*1000,
    [](std::atomic<uint32_t>& word,uint32_t expected,int64_t ns) {
      return svcWaitForAddress(&word,2,int64_t(expected),ns);
    });
  COD6_WORKER_NOTE(if(result.changed)changed.fetch_add(1,std::memory_order_relaxed));
  if(result.waited) {
#if COD6_DIAGNOSTICS
    if(stalled)stalled_parks.fetch_add(1,std::memory_order_relaxed);
    parks.fetch_add(1,std::memory_order_relaxed);
    const auto elapsed=uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-begin).count());
    sleep_ns.fetch_add(elapsed,std::memory_order_relaxed);
    if(elapsed>uint64_t(bound_us)*1000+1000000)late.fetch_add(1,std::memory_order_relaxed);
    if(!result.changed)quiet_returns.fetch_add(1,std::memory_order_relaxed);
#endif
    // libnx result.h: kernel module 1, descriptions 117 (timeout), 125
    // (the word changed before registration). Other errors disable parking.
    if(result.platform_result && result.platform_result!=0xEA01 && result.platform_result!=0xFA01) {
      COD6_WORKER_NOTE(errors.fetch_add(1,std::memory_order_relaxed);last_error.store(result.platform_result,std::memory_order_relaxed));
      failed.store(true,std::memory_order_relaxed);
    }
  }
}

extern "C" REX_FUNC(__imp__sub_8242CF40);
extern "C" REX_FUNC(sub_8242CF40) {
  __imp__sub_8242CF40(ctx,base);
  cod6::main_wait::Notify(base,cod6::main_wait::Reason::Publication);
  if(Active(base))Notify(); // Publication is complete, including synchronous fallback.
}
extern "C" REX_FUNC(__imp__sub_8242C870);
extern "C" REX_FUNC(sub_8242C870) {
  __imp__sub_8242C870(ctx,base);
  if(ctx.r3.u32==1)cod6::main_wait::Notify(base,cod6::main_wait::Reason::Consumption);
  if(stalled_enabled && worker_slot>=0 && scanning && ctx.r3.u32==1)++consumed_jobs;
  if(enabled && ctx.r3.u32==1 && Active(base))Notify(); // Actual successful consumption/completion only.
}
extern "C" REX_FUNC(__imp__sub_8242C610);
extern "C" REX_FUNC(sub_8242C610) {
  __imp__sub_8242C610(ctx,base);
  cod6::main_wait::Notify(base,cod6::main_wait::Reason::Dispatch);
  if(Active(base))Notify(); // Includes the queue-full direct-dispatch path.
}
extern "C" REX_FUNC(__imp__sub_82241098);
extern "C" REX_FUNC(sub_82241098) {
  __imp__sub_82241098(ctx,base);
  cod6::main_wait::Notify(base,cod6::main_wait::Reason::WorkerEvent);
  if(Active(base))Notify();
}

#undef COD6_WORKER_NOTE
