// SPDX-License-Identifier: GPL-3.0-or-later
// Thread-local MSR scheme adapted from VasilkovND/gears2-nx's
// gow2/tools/make_nx_pch.py (GPL-3.0). Keep ReXGlue's global mode as a fallback.
// The build audits all generated interrupt regions before enabling this header.
#pragma once

#include <atomic>
#include <cstdint>
#include <rex/thread/mutex.h>

extern "C" [[noreturn]] void Cod5InterruptDepthFailure(void);

namespace cod6::cpu {

// Set once in OnPreSetup, before the runtime creates guest/interrupt threads.
inline bool& ThreadLocalInterruptMode() {
  static bool enabled = false;
  return enabled;
}

inline int32_t& InterruptDepth() {
  static thread_local int32_t depth = 0;
  return depth;
}

inline uint64_t CheckInterruptState(std::atomic<int32_t>& legacy_count) {
  if (ThreadLocalInterruptMode()) return InterruptDepth() ? 0 : UINT64_C(0x8000);
  auto lock = rex::thread::global_critical_region::AcquireDirect();
  return legacy_count.load() ? 0 : UINT64_C(0x8000);
}

inline void EnterInterruptRegion(std::atomic<int32_t>& legacy_count) {
  if (ThreadLocalInterruptMode()) {
    ++InterruptDepth();
  } else {
    rex::thread::global_critical_region::mutex().lock();
    legacy_count.fetch_add(1);
  }
}

inline void LeaveInterruptRegion(std::atomic<int32_t>& legacy_count) {
  if (ThreadLocalInterruptMode()) {
    if (InterruptDepth() <= 0) Cod5InterruptDepthFailure();
    --InterruptDepth();
  } else {
    if (legacy_count.fetch_sub(1) <= 0) Cod5InterruptDepthFailure();
    rex::thread::global_critical_region::mutex().unlock();
  }
}

}  // namespace cod6::cpu
