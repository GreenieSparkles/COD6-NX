// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "cod6_diagnostics.h"

#if COD6_DIAGNOSTICS

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cod6::native {

// Ring-thread-only fixed top-K. Frame must be trivially copyable and expose a
// uint64-like frame_ns member. No allocation, formatting, locking or logging.
template <typename Frame, size_t Capacity>
class SlowFrameSamples {
 public:
  static_assert(Capacity > 0);
  static_assert(std::is_trivially_copyable_v<Frame>);

  void Consider(const Frame& sample) {
    uint32_t index;
    if (count_ < Capacity) {
      index = count_++;
    } else {
      index = uint32_t(Capacity - 1);
      if (sample.frame_ns <= samples_[index].frame_ns) return;
    }
    samples_[index] = sample;
    while (index && samples_[index].frame_ns > samples_[index - 1].frame_ns) {
      const Frame temporary = samples_[index];
      samples_[index] = samples_[index - 1];
      samples_[index - 1] = temporary;
      --index;
    }
  }

  uint32_t count() const { return count_; }
  const Frame& operator[](uint32_t index) const { return samples_[index]; }
  void Reset() { count_ = 0; }

 private:
  std::array<Frame, Capacity> samples_{};
  uint32_t count_ = 0;
};

}  // namespace cod6::native

#endif  // COD6_DIAGNOSTICS
