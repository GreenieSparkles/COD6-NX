// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>
#include <array>
#include <algorithm>

namespace cod6::game {

inline constexpr uint32_t kDrawSurfsAddress = 0x82422508;
inline constexpr std::array<uint8_t, 32> kDrawSurfsPrologue = {
    0x7D,0x88,0x02,0xA6,0x48,0x02,0x8B,0x29,
    0xDB,0xE1,0xFF,0xC8,0x94,0x21,0xFF,0x20,
    0x7C,0xBE,0x2B,0x78,0xF8,0x61,0x00,0xF0,
    0x7C,0x9C,0x23,0x78,0x80,0x61,0x00,0xF0};

inline bool MatchesDrawSurfsPrologue(const uint8_t* bytes) {
  return std::equal(kDrawSurfsPrologue.begin(), kDrawSurfsPrologue.end(), bytes);
}

struct PrepassResult {
  uint32_t surfaces = 0;
  uint32_t technique = UINT32_MAX;
  bool suppressed = false;
};

// The fixed-XEX 0x82422508 traversal receives the list in r5. Its count is
// read at +4; material selection reads the technique at +8. This is a guarded
// experimental policy, not a general-purpose "drop all depth draws" rule.
template <typename Load, typename Store, typename Original>
PrepassResult RunDrawSurfList(uint32_t list, bool suppress_type_zero,
                             Load load, Store store, Original original) {
  PrepassResult result;
  if (list < 0x10000 || (list & 3) || list > UINT32_MAX - 0x28) {
    original();
    return result;
  }
  result.surfaces = load(list + 4);
  result.technique = load(list + 8);
  // An unexpected count falls back. Do not conceal invalid guest data or
  // extrapolate this policy to another technique/region/version.
  if (!suppress_type_zero || result.technique != 0 || !result.surfaces ||
      result.surfaces > (1u << 20)) {
    original();
    return result;
  }
  struct RestoreCount {
    Store& store;
    uint32_t address, value;
    ~RestoreCount() { store(address, value); }
  } restore{store, list + 4, result.surfaces};
  store(list + 4, 0);
  original();
  result.suppressed = true;
  return result;
}

}  // namespace cod6::game
