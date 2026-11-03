// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
namespace cod6::skinning {
struct Counters {
  uint64_t calls=0,vertices=0,compared_vertices=0,fast_vertices=0,fallbacks=0,mismatches=0;
  std::array<uint64_t,4> vertices_by_bones{};
};
void Configure();
Counters ReadCounters();
} // namespace cod6::skinning
