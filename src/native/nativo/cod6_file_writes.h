// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

// The SDK supplies its validated canonical physical address, not a guessed
// guest alias. A successful begin must be paired even if reading fails.
extern "C" bool Cod5BeginFileReadWrite(uint32_t physical, uint32_t bytes);
extern "C" void Cod5EndFileReadWrite(uint32_t physical, uint32_t bytes);
namespace cod6::writes {
struct FileCounters {uint64_t calls=0,bytes=0,invalid=0;};
FileCounters ReadFileCounters();
}
