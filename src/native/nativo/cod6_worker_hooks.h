// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace cod6::workers {
void Configure();
struct Counters {
  bool active=false;
  uint64_t scans=0,parks=0,changed=0,quiet_returns=0,queued=0,unsafe=0,unsupported=0;
  uint64_t notifications=0,wakes=0,sleep_ns=0,errors=0,late=0;
  uint32_t last_error=0;
  uint64_t inflight_scans=0,available_scans=0,negative_scans=0,stalled_parks=0;
};
Counters ReadCounters();
} // namespace cod6::workers
