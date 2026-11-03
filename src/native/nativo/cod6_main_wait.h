// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "cod6_diagnostics.h"
#include <cstdint>

namespace cod6::main_wait {
void Configure();
enum class Reason : uint32_t {
  Publication,
  Dispatch,
  Consumption,
  WorkerEvent,
  BackendComplete,
  RingProgress,
  Count
};
void Notify(uint8_t* base,Reason reason);
// Scope only the independently identified Main caller; no guest event retained,
// guest waiter registration changed, or PPC register written by this object.
class Scope {
 public:
  Scope(uint8_t* base,uint32_t lr,uint32_t predicate,uint32_t repeat);
  ~Scope();
  Scope(const Scope&)=delete;
  Scope& operator=(const Scope&)=delete;
 private:
  bool owned_=false;
};
// Scope only the fixed Backend pending-difference helper call. It owns a gate
// independent from Main so ring progress cannot disturb Main's wait cadence.
class BackendScope {
 public:
  BackendScope(uint8_t* base,uint32_t lr,uint32_t predicate,uint32_t repeat);
  ~BackendScope();
  BackendScope(const BackendScope&)=delete;
  BackendScope& operator=(const BackendScope&)=delete;
 private:
  bool owned_=false;
};
#if COD6_DIAGNOSTICS
void Report(uint64_t sample,uint64_t frames);
#else
inline void Report(uint64_t,uint64_t) {}
#endif
}
