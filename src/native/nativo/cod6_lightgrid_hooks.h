// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace cod6::lightgrid {
struct Counters {
  uint64_t calls=0,compared=0,fast=0,fallbacks=0,mismatches=0;
  uint32_t last_source=0,last_destination=0;
};
void Configure();
Counters ReadCounters();
} // namespace cod6::lightgrid
