// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace cod6::threading {
struct ZeroWaitCounters {uint64_t fast_empty=0,legacy_empty=0,ready=0,contended=0;};
#if defined(COD6_SWITCH_ZERO_WAIT_OVERLAY)
extern "C" void Cod5ReadZeroWaitCounters(uint64_t*,uint64_t*,uint64_t*,uint64_t*);
inline ZeroWaitCounters ReadZeroWaitCounters() {
  ZeroWaitCounters result;
  Cod5ReadZeroWaitCounters(&result.fast_empty,&result.legacy_empty,&result.ready,&result.contended);
  return result;
}
#endif
} // namespace cod6::threading
