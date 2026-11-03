// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "cod6_diagnostics.h"

#if COD6_DIAGNOSTICS

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cod6::native {

struct GpuProfileEntry {
  uint64_t key = 0;
  double ns = 0;
  uint64_t count = 0;
  uint64_t draws = 0;
};

// Ring-thread-only fixed top-K snapshot. The input totals are already
// aggregated by key; selecting the largest entries here avoids copying and
// sorting the whole map on the frame boundary.
template <size_t Capacity>
struct GpuProfileSnapshot {
  static_assert(Capacity > 0);

  void Reset() {
    size = 0;
    frames = 0;
    total_ns = 0;
  }

  void Consider(const GpuProfileEntry& sample) {
    uint32_t index;
    if (size < Capacity) {
      index = size++;
    } else {
      index = uint32_t(Capacity - 1);
      if (sample.ns <= entries[index].ns) return;
    }
    entries[index] = sample;
    while (index && entries[index].ns > entries[index - 1].ns) {
      const GpuProfileEntry temporary = entries[index];
      entries[index] = entries[index - 1];
      entries[index - 1] = temporary;
      --index;
    }
  }

  std::array<GpuProfileEntry, Capacity> entries{};
  uint32_t size = 0;
  uint32_t frames = 0;
  double total_ns = 0;
};

static_assert(std::is_trivially_copyable_v<GpuProfileSnapshot<14>>);

}  // namespace cod6::native

#endif  // COD6_DIAGNOSTICS
