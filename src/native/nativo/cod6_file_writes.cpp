// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6_file_writes.h"
#include "cod6_page_writes.h"

namespace {
std::atomic<uint64_t> calls{0},bytes_requested{0},invalid_ranges{0};
cod6::writes::Range Range(uint32_t physical,uint32_t bytes) {
  return {physical,bytes,cod6::writes::g_pages.Valid(physical,bytes)};
}
}
extern "C" bool Cod5BeginFileReadWrite(uint32_t physical,uint32_t bytes) {
  if(!cod6::writes::Enabled() || !bytes)return false;
  const auto range=Range(physical,bytes);
  if(!range.valid) {
    invalid_ranges.fetch_add(1,std::memory_order_relaxed);
    cod6::writes::g_healthy.store(false,std::memory_order_relaxed);
  }
  calls.fetch_add(1,std::memory_order_relaxed);
  bytes_requested.fetch_add(bytes,std::memory_order_relaxed);
  cod6::writes::g_pages.Begin(range);
  return true;
}
extern "C" void Cod5EndFileReadWrite(uint32_t physical,uint32_t bytes) {
  cod6::writes::g_pages.End(Range(physical,bytes));
}
cod6::writes::FileCounters cod6::writes::ReadFileCounters() {
  return {calls.load(std::memory_order_relaxed),bytes_requested.load(std::memory_order_relaxed),
          invalid_ranges.load(std::memory_order_relaxed)};
}
