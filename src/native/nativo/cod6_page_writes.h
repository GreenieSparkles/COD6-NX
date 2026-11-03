// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <atomic>
#include <cstdint>
#include <limits>

namespace cod6::writes {
struct Range { uint32_t address=0,bytes=0;bool valid=false; };
inline Range GuestPhysicalRange(uint32_t guest,uint64_t bytes) {
  if(!bytes || guest<0xA0000000u)return {};
  const uint64_t limit=guest<0xC0000000u?UINT64_C(0xC0000000):
                       guest<0xE0000000u?UINT64_C(0xE0000000):UINT64_C(0xFFD00000);
  if(uint64_t(guest)>=limit || bytes>limit-guest)return {};
  const uint32_t physical=(guest&0x1FFFFFFFu)+(guest>=0xE0000000u?0x1000u:0);
  if(bytes>UINT64_C(0x20000000)-physical)return {};
  return {physical,uint32_t(bytes),true};
}

// No frame epochs: a write before OR after a cached read must invalidate that
// read, including two writes in the same frame. A global active-writer count
// prevents observing partially published page stamps. Overlapping writers use
// monotonic CAS-max, never a store that could move a page's stamp backwards.
template<size_t PageCount=(1u<<17)>
class PageWrites {
 public:
  struct Observation {uint64_t serial=0;bool quiet=false;};
  bool Valid(uint32_t address,uint64_t bytes)const {
    return bytes && uint64_t(address)<uint64_t(PageCount)*4096 &&
           bytes<=uint64_t(PageCount)*4096-address;
  }
  Observation Observe()const {
    const uint64_t serial=serial_.load(std::memory_order_seq_cst);
    return {serial,unknown_.load(std::memory_order_acquire)!=std::numeric_limits<uint64_t>::max() &&
                   active_.load(std::memory_order_seq_cst)==0 &&
                   serial==serial_.load(std::memory_order_seq_cst)};
  }
  bool StillQuiet(Observation observation)const {
    return observation.quiet && unknown_.load(std::memory_order_acquire)!=std::numeric_limits<uint64_t>::max() &&
           active_.load(std::memory_order_seq_cst)==0 &&
           serial_.load(std::memory_order_seq_cst)==observation.serial;
  }
  bool Unchanged(uint32_t address,uint32_t bytes,uint64_t since)const {
    const uint64_t unknown=unknown_.load(std::memory_order_acquire);
    if(!since || !Valid(address,bytes) || unknown==std::numeric_limits<uint64_t>::max() || unknown>since)return false;
    const uint32_t first=address>>12,last=uint32_t((uint64_t(address)+bytes-1)>>12);
    for(uint32_t page=first;page<=last;++page)
      if(pages_[page].load(std::memory_order_acquire)>since)return false;
    return true;
  }
  void Begin(Range range) {active_.fetch_add(1,std::memory_order_seq_cst);Stamp(range);}
  void End(Range range) {Stamp(range);active_.fetch_sub(1,std::memory_order_seq_cst);}
  void InvalidateAll() {Begin({});End({});}
  uint32_t Active()const{return active_.load(std::memory_order_seq_cst);}
 private:
  static void Raise(std::atomic<uint64_t>& target,uint64_t value) {
    uint64_t old=target.load(std::memory_order_relaxed);
    while(old<value && !target.compare_exchange_weak(old,value,std::memory_order_release,std::memory_order_relaxed)){}
  }
  void Stamp(Range range) {
    const uint64_t serial=serial_.fetch_add(1,std::memory_order_seq_cst)+1;
    // Serial wrap is practically unreachable, but must never make old ranges
    // look clean. Poison all reuse rather than accepting a wrapped epoch.
    if(serial==0) {Raise(unknown_,std::numeric_limits<uint64_t>::max());return;}
    if(!range.valid || !Valid(range.address,range.bytes)) {Raise(unknown_,serial);return;}
    const uint32_t last=uint32_t((uint64_t(range.address)+range.bytes-1)>>12);
    for(uint32_t page=range.address>>12;page<=last;++page)Raise(pages_[page],serial);
  }
  std::array<std::atomic<uint64_t>,PageCount> pages_{};
  std::atomic<uint64_t> serial_{1},unknown_{0};
  std::atomic<uint32_t> active_{0};
};
inline PageWrites<> g_pages;
inline std::atomic<bool> g_enabled{false},g_healthy{true};
inline bool g_verify_only=true;
inline bool Enabled(){return g_enabled.load(std::memory_order_relaxed);}
template<class Slots>
bool InvalidateBindingsOnWrites(uint64_t& observed_serial,uint64_t& memory_epoch,Slots& resolved_slots) {
  if(!Enabled())return false;
  const auto observation=g_pages.Observe();
  if(observation.quiet && observed_serial==observation.serial)return false;
  observed_serial=observation.serial;++memory_epoch;
  resolved_slots.fill(UINT64_MAX);
  return true;
}

class WriteScope {
 public:
  WriteScope(Range range,bool trusted=true):range_(range),enabled_(Enabled()) {
    if(enabled_) {
      if(!trusted) {g_healthy.store(false,std::memory_order_relaxed);range_={};}
      g_pages.Begin(range_);
    }
  }
  ~WriteScope(){if(enabled_)g_pages.End(range_);}
  WriteScope(const WriteScope&)=delete;
  WriteScope& operator=(const WriteScope&)=delete;
 private:
  Range range_;bool enabled_;
};
// Virtual addresses below A0000000 are not asserted to be physical aliases.
// An invalid physical-view range conservatively invalidates all tracked copies.
class GuestWriteScope {
 public:
  GuestWriteScope(uint32_t address,uint64_t bytes,bool trusted=true,uint32_t margin=0)
      :range_(GuestPhysicalRange(address,bytes)),active_(Enabled() && bytes && address>=0xA0000000u) {
    if(active_) {
      if(margin && range_.valid) {
        if(range_.address<margin || uint64_t(range_.address)+range_.bytes+margin>0x20000000u)range_={};
        else {range_.address-=margin;range_.bytes+=margin*2;}
      }
      if(!trusted) {g_healthy.store(false,std::memory_order_relaxed);range_={};}
      g_pages.Begin(range_);
    }
  }
  ~GuestWriteScope(){if(active_)g_pages.End(range_);}
  GuestWriteScope(const GuestWriteScope&)=delete;
  GuestWriteScope& operator=(const GuestWriteScope&)=delete;
 private:
  Range range_;bool active_;
};

struct HashResult {uint64_t hash=0;bool reused=false,compared=false,mismatch=false,busy=false;};
class WriteHashRecord {
 public:
  template<class Tracker,class Hash>
  HashResult Check(Tracker& tracker,uint32_t address,uint32_t bytes,bool verify_only,
                   std::atomic<bool>& healthy,Hash full_hash) {
    const auto observation=tracker.Observe();
    const bool same_range=address==address_ && bytes==bytes_;
    if(!same_range)validated_=0;
    const bool eligible=same_range && valid_ && healthy.load(std::memory_order_relaxed) && observation.quiet &&
                        tracker.Unchanged(address,bytes,since_);
    const bool verify=verify_only || validated_<256 || !(queries_++%64);
    if(eligible && !verify && tracker.StillQuiet(observation))return {hash_,true,false,false,false};
    const uint64_t actual=full_hash();
    const bool quiet=tracker.StillQuiet(observation);
    const bool compared=eligible && quiet;
    const bool mismatch=compared && actual!=hash_;
    if(mismatch)healthy.store(false,std::memory_order_relaxed);
    if(compared && !mismatch) {if(validated_<256)++validated_;}
    else if(actual!=hash_)validated_=0;
    hash_=actual;since_=observation.serial;
    address_=address;bytes_=bytes;
    valid_=quiet && tracker.Valid(address,bytes) && healthy.load(std::memory_order_relaxed);
    return {actual,false,compared,mismatch,!quiet};
  }
 private:
  uint64_t since_=0,hash_=0,queries_=0;
  uint32_t validated_=0;
  uint32_t address_=0,bytes_=0;
  bool valid_=false;
};
void Configure();
} // namespace cod6::writes
