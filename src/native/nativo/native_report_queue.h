// SPDX-License-Identifier: GPL-3.0-or-later
// Bounded single-producer/single-consumer handoff for immutable diagnostic
// snapshots. Compiled only into test/diagnostic artifacts.
#pragma once

#include "cod6_diagnostics.h"

#if COD6_DIAGNOSTICS

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cod6::native {

template <typename Snapshot, size_t Capacity>
class ReportSnapshotQueue {
 public:
  static_assert(Capacity >= 2);
  static_assert(std::is_trivially_copyable_v<Snapshot>);

  // Producer only. No snapshot state may be destructively captured until this
  // succeeds. A full queue never waits for the formatter.
  Snapshot* TryReserve() {
    assert(!producer_reserved_);
    const uint64_t consumed = consumed_.load(std::memory_order_acquire);
    if (producer_position_ - consumed >= Capacity) {
      dropped_.fetch_add(1, std::memory_order_relaxed);
      return nullptr;
    }
    producer_reserved_ = true;
    return &slots_[producer_position_ % Capacity];
  }

  // Producer only. Everything written to the reserved slot becomes immutable
  // before the release publication.
  void Publish() {
    assert(producer_reserved_);
    producer_reserved_ = false;
    ++producer_position_;
    published_.store(producer_position_, std::memory_order_release);
  }

  // Consumer only. The returned slot stays immutable until Release.
  const Snapshot* TryAcquire() {
    assert(!consumer_acquired_);
    const uint64_t published = published_.load(std::memory_order_acquire);
    if (consumer_position_ == published) return nullptr;
    consumer_acquired_ = true;
    return &slots_[consumer_position_ % Capacity];
  }

  // Consumer only.
  void Release() {
    assert(consumer_acquired_);
    consumer_acquired_ = false;
    ++consumer_position_;
    consumed_.store(consumer_position_, std::memory_order_release);
  }

  uint64_t dropped() const { return dropped_.load(std::memory_order_relaxed); }
  uint64_t published() const { return published_.load(std::memory_order_acquire); }
  uint64_t consumed() const { return consumed_.load(std::memory_order_acquire); }

 private:
  std::array<Snapshot, Capacity> slots_{};
  // Producer and consumer positions are touched by one thread each. The
  // published/consumed cursors are on separate cache lines to avoid the ring
  // and formatter invalidating the same line for every snapshot.
  alignas(64) std::atomic<uint64_t> published_{0};
  alignas(64) std::atomic<uint64_t> consumed_{0};
  alignas(64) std::atomic<uint64_t> dropped_{0};
  uint64_t producer_position_ = 0;
  uint64_t consumer_position_ = 0;
  bool producer_reserved_ = false;
  bool consumer_acquired_ = false;
};

}  // namespace cod6::native

#endif  // COD6_DIAGNOSTICS
