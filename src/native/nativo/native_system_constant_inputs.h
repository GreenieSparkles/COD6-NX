// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

namespace cod6::native {

// Exact generations and masks that can change the native system UBO. The
// setup token is either the original rebuild generation or, for the candidate,
// the immutable identity of a cached DrawSetup entry.
struct SystemConstantInputIdentity {
  uint64_t setup_token = ~UINT64_C(0);
  uint64_t fetch_generation = 0;
  uint64_t index_generation = 0;
  uint32_t used_texture_mask = 0;

  bool operator==(const SystemConstantInputIdentity&) const = default;
};

inline uint64_t SelectSystemSetupToken(bool stable, uint64_t setup_identity,
                                       uint64_t setup_generation) {
  return stable ? setup_identity : setup_generation;
}

}  // namespace cod6::native
