// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6_local_reg_check_api.h"

#include <rex/logging.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_set>

#ifndef COD6_LOCAL_REG_CHECK_AUDIT_SHA
#error "COD6_LOCAL_REG_CHECK_AUDIT_SHA must identify the generated-source audit"
#endif

namespace {

constexpr size_t kUniqueMismatchLimit = 256;
constexpr uint32_t kMappedSites = 59374;
constexpr uint32_t kGeneratedComparisons = 127269;
constexpr const char* kSchema =
    "F45FEE6D905933B3347871378CC2C077810A6F547D0AE6A7B234B53F8053CAA9";
constexpr const char* kSourceAudit = COD6_LOCAL_REG_CHECK_AUDIT_SHA;

std::atomic<uint64_t> g_total_mismatches{0};
std::atomic<uint64_t> g_suppressed_mismatches{0};
std::mutex g_mismatch_mutex;
std::unordered_set<uint64_t> g_unique_mismatches;
bool g_limit_reported = false;

uint32_t RegisterHash(const char* text) {
  uint32_t hash = 2166136261u;
  while (*text) {
    hash = (hash ^ static_cast<uint8_t>(*text++)) * 16777619u;
  }
  return hash;
}

}  // namespace

void cod6_local_reg_check_startup() {
  REXLOG_INFO(
      "[local-reg-check] schema=1 enabled=1 sites={} comparisons={} classes=gpr,fpr,vector,cr,xer,ctr,lr,reserved "
      "unique_limit={} schema_sha256={} source_audit_sha256={}",
      kMappedSites, kGeneratedComparisons, kUniqueMismatchLimit, kSchema,
      kSourceAudit);
}

extern "C" void cod6_local_reg_check_fail(
    const char* function,
    const char* callee,
    uint32_t site,
    const char* register_name) {
  const uint64_t occurrence =
      g_total_mismatches.fetch_add(1, std::memory_order_relaxed) + 1;
  const uint64_t key =
      (static_cast<uint64_t>(site) << 32) | RegisterHash(register_name);
  size_t unique = 0;
  bool report_mismatch = false;
  bool report_limit = false;
  {
    std::lock_guard lock(g_mismatch_mutex);
    if (g_unique_mismatches.contains(key)) {
      return;
    }
    if (g_unique_mismatches.size() >= kUniqueMismatchLimit) {
      g_suppressed_mismatches.fetch_add(1, std::memory_order_relaxed);
      if (!g_limit_reported) {
        g_limit_reported = true;
        report_limit = true;
      }
    } else {
      g_unique_mismatches.insert(key);
      unique = g_unique_mismatches.size();
      report_mismatch = true;
    }
  }
  if (report_mismatch) {
    REXLOG_ERROR(
        "[local-reg-check] schema=1 mismatch={} unique={} site={:06X} function={} callee={} register={}",
        occurrence, unique, site, function, callee, register_name);
  } else if (report_limit) {
    REXLOG_ERROR(
        "[local-reg-check] schema=1 unique_limit_reached={} further_unique_mismatches_suppressed=1",
        kUniqueMismatchLimit);
  }
}

void cod6_local_reg_check_shutdown() {
  size_t unique = 0;
  {
    std::lock_guard lock(g_mismatch_mutex);
    unique = g_unique_mismatches.size();
  }
  REXLOG_INFO(
      "[local-reg-check] schema=1 shutdown total_mismatches={} unique_mismatches={} suppressed_mismatches={} "
      "acceptance_zero_mismatch={}",
      g_total_mismatches.load(std::memory_order_relaxed), unique,
      g_suppressed_mismatches.load(std::memory_order_relaxed),
      g_total_mismatches.load(std::memory_order_relaxed) == 0);
}
