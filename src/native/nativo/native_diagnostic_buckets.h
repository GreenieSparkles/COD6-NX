// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace cod6::native {

// Ring-thread only. Fixed storage keeps diagnostic logging out of draw calls.
template <size_t Capacity, size_t Words = 10>
class DiagnosticBuckets {
 public:
  static_assert(Capacity > 0);
  using Key = std::array<uint64_t, Words>;
  struct Entry { Key key{}; uint64_t count = 0; };

  void Record(const Key& key) {
    ++total_;
    for (size_t i = 0; i < size_; ++i) {
      if (entries_[i].key == key) { ++entries_[i].count; return; }
    }
    if (size_ == Capacity) { ++omitted_; return; }
    entries_[size_++] = {key, 1};
  }
  const auto& entries() const { return entries_; }
  size_t size() const { return size_; }
  uint64_t total() const { return total_; }
  uint64_t omitted() const { return omitted_; }
  void Reset() { size_ = 0; total_ = omitted_ = 0; }

 private:
  std::array<Entry, Capacity> entries_{};
  size_t size_ = 0;
  uint64_t total_ = 0, omitted_ = 0;
};

}  // namespace cod6::native
