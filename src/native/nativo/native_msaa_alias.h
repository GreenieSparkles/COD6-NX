// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <cstdint>

namespace cod6::native {

// Host MSAA may be disabled without changing how the guest addresses EDRAM.
// Keep the guest modes that have used a host image so 4x half-pitch clear and
// resolve views can still find their 2x/1x full-pitch aliases.
class GuestMsaaModes {
 public:
  static constexpr std::array<uint32_t, 2> kFourXFullPitchAliases = {1, 0};

  void Note(uint32_t mode) {
    if (mode < 3) mask_ |= uint8_t(1u << mode);
  }

  bool Has(uint32_t mode) const {
    return mode < 3 && (mask_ & uint8_t(1u << mode));
  }

  static constexpr uint32_t HostMode(bool enabled, uint32_t guest_mode) {
    return enabled ? guest_mode : 0;
  }

  static constexpr uint32_t AliasScaleY(uint32_t full_pitch_guest_mode) {
    return full_pitch_guest_mode == 0 ? 2 : 1;
  }

 private:
  uint8_t mask_ = 0;
};

}  // namespace cod6::native
