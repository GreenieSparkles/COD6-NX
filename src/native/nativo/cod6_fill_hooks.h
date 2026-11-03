// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace cod6::fill {
void Configure();
struct Counters {uint64_t observed_fill=0,observed_block=0,calls=0,bytes=0,compared=0,fast=0,fallbacks=0,mismatches=0;};
Counters ReadCounters();
} // namespace cod6::fill
