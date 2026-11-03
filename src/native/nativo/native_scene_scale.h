// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cstdint>

namespace cod6::native {

// Fixed-XEX scene-resolution candidate. COD5's playable scene uses a 1040
// pixel EDRAM pitch, color base 0 and depth base 975, and resolves a 1024x600
// rectangle. Menus / Bink use the separate 960x544 path, while shadow maps
// use depth base 0, so this exact signature keeps those surfaces untouched.
struct SceneScale {
  uint32_t numerator = 1;
  uint32_t denominator = 1;

  static SceneScale FromPercent(uint32_t percent) {
    switch (percent) {
      case 50:
        return {1, 2};
      case 75:
        return {3, 4};
      case 100:
      default:
        return {1, 1};
    }
  }

  bool enabled() const { return numerator != denominator; }

  // The fixed-XEX target identity is useful to optimizations that don't
  // change the scene resolution (for example, the Switch NVK ZCULL path).
  // Keep it independent from enabled(), while IsSceneTarget remains the
  // resolution-scaling predicate.
  static bool MatchesFixedTarget(uint32_t base_tiles, uint32_t pitch,
                                 bool is_depth, uint32_t host_msaa) {
    if (pitch != 1040 || host_msaa != 0) {
      return false;
    }
    return is_depth ? base_tiles == 975 : base_tiles == 0;
  }

  uint32_t Floor(uint32_t value) const {
    return uint32_t((uint64_t(value) * numerator) / denominator);
  }

  uint32_t Ceil(uint32_t value) const {
    return uint32_t((uint64_t(value) * numerator + denominator - 1) /
                    denominator);
  }

  void ScaleInterval(uint32_t& offset, uint32_t& extent) const {
    const uint32_t end = offset + extent;
    const uint32_t scaled_offset = Floor(offset);
    const uint32_t scaled_end = Ceil(end);
    offset = scaled_offset;
    extent = scaled_end - scaled_offset;
  }

  bool IsSceneTarget(uint32_t base_tiles, uint32_t pitch, bool is_depth,
                     uint32_t host_msaa) const {
    return enabled() &&
           MatchesFixedTarget(base_tiles, pitch, is_depth, host_msaa);
  }
};

}  // namespace cod6::native
