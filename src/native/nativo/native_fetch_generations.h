// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <bit>
#include <cstdint>

namespace cod6::native {

// Fetch constants are grouped in 32 six-word slots. A monotonically newer
// sequence is assigned to every written slot. The maximum sequence of the
// slots a shader uses changes if and only if one of those slots was written;
// writes to unrelated slots therefore do not invalidate texture-binding work.
class FetchGenerations {
 public:
  void Written(uint32_t mask) {
    if (!mask) return;
    const uint64_t generation = ++all_;
    while (mask) {
      const uint32_t slot = uint32_t(std::countr_zero(mask));
      mask &= mask - 1;
      slots_[slot] = generation;
    }
  }

  uint64_t all() const { return all_; }

  uint64_t ForMask(uint32_t mask) const {
    uint64_t generation = 1;
    while (mask) {
      const uint32_t slot = uint32_t(std::countr_zero(mask));
      mask &= mask - 1;
      if (slots_[slot] > generation) generation = slots_[slot];
    }
    return generation;
  }

 private:
  uint64_t all_ = 1;
  std::array<uint64_t, 32> slots_ = [] {
    std::array<uint64_t, 32> values{};
    values.fill(1);
    return values;
  }();
};

}  // namespace cod6::native
