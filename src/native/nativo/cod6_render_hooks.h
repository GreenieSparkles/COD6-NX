// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>

namespace cod6::game {
struct DrawSurfCounters {
  uint64_t lists = 0;
  uint64_t surfaces = 0;
  uint64_t suppressed_lists = 0;
  uint64_t suppressed_surfaces = 0;
  std::array<uint64_t, 32> lists_by_technique{};
  std::array<uint64_t, 32> surfaces_by_technique{};
};
void ConfigureRenderHooks();
DrawSurfCounters ReadDrawSurfCounters();
struct LodCounters {
  uint64_t calls=0,rigid_changed=0,skinned_changed=0,invalid=0,unsupported=0;
};
LodCounters ReadLodCounters();
}  // namespace cod6::game
