// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>

namespace cod6::native {
// Immutable layout metadata only, never guest bytes or content hashes. Hashes
// select a bounded slot; the complete key determines whether its answer fits.
template <typename Key> class LayoutExtents {
 public:
  static constexpr uint32_t kCapacity = 1024;
  bool Find(uint64_t hash, const Key& key, uint32_t& base, uint32_t& mips) const {
    const auto& e = entries_[hash & (kCapacity - 1)];
    if (!e.valid || !(e.key == key)) return false;
    base = e.base; mips = e.mips;
    return true;
  }
  void Store(uint64_t hash, const Key& key, uint32_t base, uint32_t mips) {
    entries_[hash & (kCapacity - 1)] = {key, base, mips, true};
  }
 private:
  struct Entry { Key key{}; uint32_t base=0, mips=0; bool valid=false; };
  std::array<Entry, kCapacity> entries_{};
};
}
