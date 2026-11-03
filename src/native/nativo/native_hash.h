// SPDX-License-Identifier: GPL-3.0-or-later
//
// Change detection for guest data the native renderer copies (textures and
// vertex buffers).
//
// Hashing every byte of everything a frame uses was half of the renderer's CPU
// time in Free Skate. Most of that data never changes after loading, so once a
// range has been identical for kStableChecks full hashes it is checked each
// frame by hashing a few small samples spread over it, and hashed in full only
// one frame in kFullEvery (staggered, so the full hashes do not all land on the
// same frame). A change the samples miss is picked up by the next full hash,
// at most kFullEvery frames later. The emulated path trusts stable guest pages
// the same way (sampled page hashing in the SDK's shared memory).
//
// Ranges unchanged for kTrustedChecks checks are only sampled one frame in
// kTrustedEvery: each sample is a cache miss on guest memory, and with a few
// thousand ranges in a frame the sampling alone cost milliseconds. A change
// to such a range shows up at most kTrustedEvery frames late.

#pragma once

#include <algorithm>
#include <cstddef>
#include <bit>
#include <cstdint>
#include <vector>

#include <xxhash.h>

#include "cod6_diagnostics.h"

namespace cod6::native {

#if COD6_DIAGNOSTICS
// Record-only classification of ChangeDetector work. The caller owns the
// counters so separate texture and vertex paths can be reported without
// atomics. A production build compiles the type and all updates out.
struct ChangeDetectorStats {
  uint64_t trusted_skips = 0;
  uint64_t sample_checks = 0;
  uint64_t sample_hits = 0;
  uint64_t full_checks = 0;
  uint64_t full_initial = 0;
  uint64_t full_unchanged = 0;
  uint64_t full_changed = 0;
  uint64_t sample_input_bytes = 0;
  uint64_t full_input_bytes = 0;
};

struct FlatMapClearStats {
  uint64_t clears = 0;
  uint64_t entries = 0;
  uint64_t capacity = 0;
  uint64_t ns = 0;
};
#endif

class ChangeDetector {
 public:
  static constexpr uint32_t kStableChecks = 8;
  // 12, not 32: until trusted, a texture's lookup runs in full every frame,
  // and a busy scene's thousands of textures stayed on that path for a second
  // after appearing (15 ms of texture lookups in 3000-draw frames).
  static constexpr uint32_t kTrustedChecks = 12;
  static constexpr uint64_t kFullEvery = 16;
  static constexpr uint64_t kTrustedEvery = 8;
  // cod5-nx: data that changed at least this many times is rewritten in place by the game (COD5's model lighting
  // and other per-frame CPU data): it stays on full hashes every frame, since a partial rewrite slips past the
  // samples and showed as characters flickering. Only for ranges up to dynamic_max_bytes, so textures loaded into
  // reused memory once or twice are not affected. 0 turns this off.
  static inline uint32_t dynamic_changes = 4;
  static inline size_t dynamic_max_bytes = 4u * 1024 * 1024;
  // Diagnostics: hash everything in full every frame.
  static inline bool always_full = false;

  // Returns the content hash of [data, data + size), hashing it all only when
  // needed. `stagger` spreads the full hashes over frames.
  // The last frame, from `frame` on, whose Check is certain to return the hash
  // it returned for `frame` without looking at the data: `frame` itself
  // unless the content is trusted.
  uint64_t TrustedUntil(uint64_t frame, uint64_t stagger) const {
    if (unchanged_ < TrustedChecks()) return frame;
    return frame + (kTrustedEvery - (frame + stagger) % kTrustedEvery) - 1;
  }

  // A guest synchronization may publish new bytes within the same frame.
  // Keep the previous hash/change history so an unchanged range can reuse its
  // uploaded copy, but require a full content check on the next Check call.
  void ResetTrust() { unchanged_ = 0; }

  uint64_t Check(const uint8_t* data, size_t size, uint64_t frame, uint64_t stagger,
                 uint64_t& bytes_hashed
#if COD6_DIAGNOSTICS
                 , ChangeDetectorStats* stats = nullptr
#endif
                 ) {
    size_ = size;
    const uint32_t trusted_checks = TrustedChecks();
    if (unchanged_ >= trusted_checks && (frame + stagger) % kTrustedEvery) {
#if COD6_DIAGNOSTICS
      if (stats) ++stats->trusted_skips;
#endif
      return hash_;
    }
    uint64_t sample = Sample(data, size);
#if COD6_DIAGNOSTICS
    if (stats) {
      ++stats->sample_checks;
      stats->sample_input_bytes +=
          size <= kSamples * kSampleBytes * 2 ? size : kSamples * kSampleBytes;
    }
#endif
    if (unchanged_ >= StableChecks() && sample == sample_ && (frame + stagger) % kFullEvery) {
      if (unchanged_ < trusted_checks) {
        ++unchanged_;
      }
      bytes_hashed += kSamples * kSampleBytes;
#if COD6_DIAGNOSTICS
      if (stats) ++stats->sample_hits;
#endif
      return hash_;
    }
    uint64_t hash = XXH3_64bits(data, size);
    bytes_hashed += size;
#if COD6_DIAGNOSTICS
    if (stats) {
      ++stats->full_checks;
      stats->full_input_bytes += size;
    }
#endif
    if (hash == hash_ && checked_) {
#if COD6_DIAGNOSTICS
      if (stats) ++stats->full_unchanged;
#endif
      if (unchanged_ < trusted_checks) {
        ++unchanged_;
      }
    } else {
#if COD6_DIAGNOSTICS
      if (stats) {
        if (checked_) ++stats->full_changed;
        else ++stats->full_initial;
      }
#endif
      if (checked_ && changes_ < 0xFFFF) ++changes_;
      unchanged_ = 0;
    }
    hash_ = hash;
    sample_ = sample;
    checked_ = true;
    return hash;
  }

 private:
  static constexpr size_t kSamples = 8;
  static constexpr size_t kSampleBytes = 64;

  static uint64_t Sample(const uint8_t* data, size_t size) {
    if (size <= kSamples * kSampleBytes * 2) {
      return XXH3_64bits(data, size);
    }
    uint64_t hash = size;
    size_t step = (size - kSampleBytes) / (kSamples - 1);
    for (size_t i = 0; i < kSamples; ++i) {
      hash = XXH3_64bits_withSeed(data + i * step, kSampleBytes, hash);
    }
    return hash;
  }

  // Data that changed before was once kept on full checks for 10 s after: it
  // made every texture a level loads into reused memory hash in full every
  // frame (7% of the ring thread), and the cutscene it was meant for turned
  // out to be decoded correctly into textures that do reach the renderer.
  bool Dynamic() const {
    return always_full ||
           (dynamic_changes && changes_ >= dynamic_changes && size_ <= dynamic_max_bytes);
  }
  uint32_t StableChecks() const { return Dynamic() ? UINT32_MAX : kStableChecks; }
  uint32_t TrustedChecks() const { return Dynamic() ? UINT32_MAX : kTrustedChecks; }

 public:
  uint32_t changes() const { return changes_; }

 private:
  size_t size_ = 0;

  uint64_t hash_ = 0;
  uint64_t sample_ = 0;
  uint32_t unchanged_ = 0;
  uint32_t changes_ = 0;
  bool checked_ = false;
};

// A hash map for 64-bit keys that are already hashes or unique numbers:
// open addressing with linear probing in one array. std::unordered_map's node
// per entry cost a cache miss or two per lookup, and the renderer does several
// lookups per draw. Per-slot generations distinguish empty slots, so every
// 64-bit key is valid. Growing moves the values, so references from operator[]
// last only until the next insertion.
template <typename V>
class FlatMap64 {
 public:
  static constexpr uint64_t kEmpty = ~uint64_t(0);

  const V* find(uint64_t key) const { return const_cast<FlatMap64*>(this)->find(key); }

  V* find(uint64_t key) {
    if (!capacity_) return nullptr;
    size_t i = Slot(key);
    while (true) {
      if (generations_[i] != generation_) return nullptr;
      if (keys_[i] == key) return &values_[i];
      i = (i + 1) & (capacity_ - 1);
    }
  }

#if COD6_DIAGNOSTICS
  const V* find_with_probes(uint64_t key, uint64_t& probes) const {
    return const_cast<FlatMap64*>(this)->find_with_probes(key, probes);
  }

  V* find_with_probes(uint64_t key, uint64_t& probes) {
    probes = 0;
    if (!capacity_) return nullptr;
    size_t i = Slot(key);
    while (true) {
      ++probes;
      if (generations_[i] != generation_) return nullptr;
      if (keys_[i] == key) return &values_[i];
      i = (i + 1) & (capacity_ - 1);
    }
  }
#endif

  V& operator[](uint64_t key) {
    // Do not grow merely because a lookup of an existing key happens while
    // the table is at its insertion threshold. This operator is used on every
    // draw for texture bindings; the old pre-lookup growth check could move
    // the whole table even though no entry was being added.
    if (capacity_) {
      size_t i = Slot(key);
      while (true) {
        if (generations_[i] != generation_) {
          if ((size_ + 1) * 2 <= capacity_) {
            keys_[i] = key;
            values_[i] = V();
            generations_[i] = generation_;
            ++size_;
            return values_[i];
          }
          break;
        }
        if (keys_[i] == key) return values_[i];
        i = (i + 1) & (capacity_ - 1);
      }
    }
    Grow();
    size_t i = Slot(key);
    while (true) {
      if (generations_[i] != generation_) {
        keys_[i] = key;
        values_[i] = V();
        generations_[i] = generation_;
        ++size_;
        return values_[i];
      }
      i = (i + 1) & (capacity_ - 1);
    }
  }

  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

  void clear() {
    size_ = 0;
    // Logical O(1) clear. Only the practically unreachable 32-bit wrap needs
    // to touch every slot so no entry from an earlier epoch can reappear.
    if (++generation_ == 0) {
      std::fill(generations_.begin(), generations_.end(), 0);
      generation_ = 1;
    }
  }

 private:
  // The top bits of the product: a product bit depends only on the key bits
  // at or below it, so the middle bits used before ignored the high half of
  // keys like (address << 32 | size), and every vertex buffer of one size
  // probed the same chain.
  size_t Slot(uint64_t key) const {
    uint64_t h = key * 0x9E3779B97F4A7C15ull;
    return size_t(h >> shift_);
  }

  void Grow() {
    size_t new_capacity = capacity_ ? capacity_ * 2 : 1024;
    std::vector<uint64_t> old_keys = std::move(keys_);
    std::vector<V> old_values = std::move(values_);
    std::vector<uint32_t> old_generations = std::move(generations_);
    const uint32_t old_generation = generation_;
    keys_.assign(new_capacity, kEmpty);
    values_.assign(new_capacity, V());
    generations_.assign(new_capacity, 0);
    capacity_ = new_capacity;
    shift_ = 64 - uint32_t(std::countr_zero(uint64_t(new_capacity)));
    size_ = 0;
    generation_ = 1;
    for (size_t i = 0; i < old_keys.size(); ++i) {
      if (old_generations[i] == old_generation) {
        (*this)[old_keys[i]] = std::move(old_values[i]);
      }
    }
  }

  std::vector<uint64_t> keys_;
  std::vector<V> values_;
  std::vector<uint32_t> generations_;
  size_t capacity_ = 0;
  size_t size_ = 0;
  uint32_t shift_ = 64;
  uint32_t generation_ = 1;
};

}  // namespace cod6::native
