// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace cod6::native {

// Evidence key for a resolve that has already been recorded successfully.
// This is deliberately stricter than the Vulkan copy parameters: global
// answer/fanout epochs are included so unrelated cache topology changes may
// cause false negatives, but can never turn an uncertain resolve into a
// repeat candidate.
struct ResolveRepeatKey {
  uint64_t source_identity = 0;
  uint64_t source_content_generation = 0;
  uint64_t destination_identity = 0;
  uint64_t answers_epoch = 0;
  uint64_t fanout_generation = 0;
  uint32_t destination_address = 0;
  uint32_t source_x = 0;
  uint32_t source_y = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t destination_width = 0;
  uint32_t destination_height = 0;
  uint32_t source_format = 0;
  uint32_t destination_format = 0;
  uint32_t destination_swizzle = 0;
  uint32_t copy_control = 0;
  uint32_t destination_info = 0;
  uint32_t path = 0;

  bool operator==(const ResolveRepeatKey&) const = default;
};

enum class ResolveRepeatResult : uint8_t {
  kFirstOrChanged,
  kRepeat,
  kCollision,
};

// Ring-thread-only, fixed-size diagnostic. Hashing only selects a slot; a
// repeat is reported only after every field compares equal. A collision
// overwrites one observation and therefore loses opportunities rather than
// producing a false positive.
template <size_t Capacity = 128>
class ResolveRepeatTracker {
 public:
  static_assert(Capacity && !(Capacity & (Capacity - 1)),
                "capacity must be a power of two");

  ResolveRepeatResult Observe(const ResolveRepeatKey& key) {
    Entry& entry = entries_[Slot(key)];
    ResolveRepeatResult result = ResolveRepeatResult::kFirstOrChanged;
    if (entry.valid) {
      if (entry.key == key) {
        result = ResolveRepeatResult::kRepeat;
      } else if (entry.key.destination_identity != key.destination_identity ||
                 entry.key.destination_address != key.destination_address) {
        result = ResolveRepeatResult::kCollision;
      }
    }
    entry.key = key;
    entry.valid = true;
    return result;
  }

  static constexpr size_t Bytes() { return sizeof(ResolveRepeatTracker); }

 private:
  struct Entry {
    ResolveRepeatKey key;
    bool valid = false;
  };

  static uint64_t Mix(uint64_t value) {
    value += UINT64_C(0x9E3779B97F4A7C15);
    value = (value ^ (value >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94D049BB133111EB);
    return value ^ (value >> 31);
  }

  static size_t Slot(const ResolveRepeatKey& key) {
    uint64_t hash = key.destination_identity;
    hash ^= uint64_t(key.destination_address) << 17;
    return size_t(Mix(hash)) & (Capacity - 1);
  }

  std::array<Entry, Capacity> entries_{};
};

static_assert(ResolveRepeatTracker<>::Bytes() <= 16 * 1024);

}  // namespace cod6::native
