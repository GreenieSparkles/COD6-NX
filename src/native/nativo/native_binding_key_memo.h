// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace cod6::native {

// The texture-binding hash is derived from up to 32 six-word fetch slots.  A
// dependency identity that has not changed produces exactly the same hash, so
// keep the already-computed value in a bounded direct-map table.  Hashing only
// chooses a slot: every field is compared before an answer is returned.
struct BindingKeyIdentity {
  uint64_t vs = 0;
  uint64_t ps = 0;
  uint64_t layout = 0;
  uint64_t used_fetch_generation = 0;
  uint32_t slot_mask = 0;

  bool operator==(const BindingKeyIdentity&) const = default;
};

template <size_t Capacity = 256>
class BindingKeyMemo {
 public:
  static_assert(Capacity && !(Capacity & (Capacity - 1)), "capacity must be a power of two");
  static constexpr size_t kCapacity = Capacity;

  // The object keeps a fixed maximum allocation so same-NRO A/B tests change
  // only the lookup mask, not allocator behavior or object lifetime. Invalid
  // runtime values fail closed to the original 256-slot table (or Capacity for
  // deliberately smaller host fixtures).
  explicit BindingKeyMemo(size_t active_capacity = Capacity)
      : active_capacity_(ValidCapacity(active_capacity) ? active_capacity
                                                        : FallbackCapacity()) {}

  bool Lookup(const BindingKeyIdentity& key, uint64_t& value) const {
    const Entry& entry = entries_[Slot(key)];
    if (!entry.valid || !(entry.key == key)) return false;
    value = entry.value;
    return true;
  }

  // Returns true when another valid identity occupied the selected slot.
  bool Store(const BindingKeyIdentity& key, uint64_t value) {
    Entry& entry = entries_[Slot(key)];
    const bool collision = entry.valid && !(entry.key == key);
    entry.key = key;
    entry.value = value;
    entry.valid = true;
    return collision;
  }

  size_t capacity() const { return active_capacity_; }
  static constexpr size_t Bytes() { return sizeof(BindingKeyMemo); }

 private:
  struct Entry {
    BindingKeyIdentity key;
    uint64_t value = 0;
    bool valid = false;
  };

  static uint64_t Mix(uint64_t value) {
    value += UINT64_C(0x9E3779B97F4A7C15);
    value = (value ^ (value >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94D049BB133111EB);
    return value ^ (value >> 31);
  }

  static constexpr size_t FallbackCapacity() {
    return Capacity >= 256 ? 256 : Capacity;
  }

  static constexpr bool ValidCapacity(size_t value) {
    return value && value <= Capacity && !(value & (value - 1));
  }

  size_t Slot(const BindingKeyIdentity& key) const {
    uint64_t hash = key.vs;
    const auto combine = [&hash](uint64_t value) {
      hash ^= value + UINT64_C(0x9E3779B97F4A7C15) + (hash << 6) + (hash >> 2);
    };
    combine(key.ps);
    combine(key.layout);
    combine(key.used_fetch_generation);
    combine(key.slot_mask);
    return size_t(Mix(hash)) & (active_capacity_ - 1);
  }

  size_t active_capacity_ = FallbackCapacity();
  std::array<Entry, Capacity> entries_{};
};

}  // namespace cod6::native
