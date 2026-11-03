// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <span>

namespace cod6::native {
// Ring-thread only. A missing epoch is a GLOBAL invalidation, never permission
// to reuse. Records don't contain Texture pointers and never extend lifetimes.
class ResolveAnswerJournal {
 public:
  static constexpr uint64_t kCapacity = 64;
  void Record(uint64_t epoch, uint32_t address) {
    entries_[epoch % kCapacity] = {epoch, address & ~uint32_t(3)};
  }
  bool DisjointSince(uint64_t previous, uint64_t current,
                     std::span<const uint32_t> sampled_addresses) const {
    if (previous == current) return true;
    if (current < previous || current - previous > kCapacity) return false;
    for (uint64_t offset = 1; offset <= current - previous; ++offset) {
      const uint64_t epoch = previous + offset;
      const auto& entry = entries_[epoch % kCapacity];
      if (entry.epoch != epoch) return false;
      for (uint32_t address : sampled_addresses) {
        if (entry.address == address) return false;
      }
    }
    return true;
  }
 private:
  struct Entry { uint64_t epoch = 0; uint32_t address = 0; };
  std::array<Entry, kCapacity> entries_{};
};
static_assert(sizeof(ResolveAnswerJournal) <= 1024);
}  // namespace cod6::native
