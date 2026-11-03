// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6_cpu_runtime.h"
#include "cod6_ppc_interrupts.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/logging/assert.h>

#if defined(COD6_PPC_INTERRUPT_OVERLAY)
static constexpr bool kDefaultThreadLocalMsr = true;
#else
static constexpr bool kDefaultThreadLocalMsr = false;
#endif

REXCVAR_DEFINE_BOOL(cod6_ppc_thread_local_msr, kDefaultThreadLocalMsr, "COD5",
                    "Use per-thread interrupt state for build-audited CAS regions; false restores the global mutex")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

extern "C" [[noreturn]] void Cod5InterruptDepthFailure(void) {
  REX_FATAL("[cpu] Unbalanced generated PPC interrupt region");
}

void cod6::cpu::Configure() {
  const bool requested = REXCVAR_GET(cod6_ppc_thread_local_msr);
#if !defined(COD6_PPC_INTERRUPT_OVERLAY)
  if (requested) REX_FATAL("[cpu] Thread-local MSR requested without the audited PCH overlay");
#endif
  ThreadLocalInterruptMode() = requested;
  REXLOG_INFO("[cpu] Generated PPC interrupt regions: {} (CAS and memory barriers unchanged)",
              requested ? "thread-local state" : "legacy global mutex");
}
