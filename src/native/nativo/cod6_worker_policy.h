// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace cod6::workers {
inline constexpr uint32_t kWorkerReturn=0x8242CEC8;
inline constexpr uint32_t kQueueBase=0x84E72880,kQueueStride=128,kQueueCount=27;
struct CodeCheck { uint32_t address,bytes;uint64_t fingerprint; };
// Explicit fingerprint windows, not a claim about recovered function sizes.
inline constexpr CodeCheck kCodeChecks[]={
  {0x8242CEA0,44,UINT64_C(0xB2FA31D052CBB457)},
  {0x8242CBD8,208,UINT64_C(0xE408F761ECB5C5D2)},
  {0x8242C870,776,UINT64_C(0xF109E1178CD2A645)},
  {0x8242CF40,500,UINT64_C(0x2FE8AAC28A5D42AB)},
  {0x8242C610,608,UINT64_C(0xF8D9222AFCD8997A)},
  {0x82241098,12,UINT64_C(0x5BABB7E826DD3497)},
  {0x8242CED0,140,UINT64_C(0xD5EE10FF802883A4)},
  {0x82240AD8,112,UINT64_C(0x0D4CD250FB2005BE)},
  {0x82240920,112,UINT64_C(0xE29329AD2B2E606B)},
  {0x82240990,144,UINT64_C(0x446A06E8714C7BA6)},
};
inline bool MatchesWorkerCode(const uint8_t* base) {
  for(const auto& check:kCodeChecks) {
    uint64_t hash=UINT64_C(14695981039346656037);
    for(uint32_t i=0;i<check.bytes;++i) hash=(hash^base[check.address+i])*UINT64_C(1099511628211);
    if(hash!=check.fingerprint)return false;
  }
  return true;
}
template<class Load> bool QueuesIdle(Load load) {
  for(uint32_t i=0;i<kQueueCount;++i) {
    const uint32_t queue=kQueueBase+i*kQueueStride;
    // In-flight jobs and temporary negative claims also forbid parking.
    if(load(queue+12)!=0 || load(queue+16)!=0)return false;
  }
  return true;
}

// Observations only: +12 counts outstanding work (publication/execution), +16
// is atomically claimed by consumers and can be transiently negative. Positive
// +16 does not prove that a dependency callback will allow a job to run.
struct QueueObservation {
  bool outstanding=false,available=false,negative=false;
  uint64_t fingerprint=UINT64_C(14695981039346656037);
  bool Idle()const{return !outstanding && !available && !negative;}
};
template<class Load> QueueObservation ObserveQueues(Load load) {
  QueueObservation result;
  for(uint32_t i=0;i<kQueueCount;++i) {
    const uint32_t queue=kQueueBase+i*kQueueStride;
    const uint32_t outstanding=load(queue+12),available=load(queue+16);
    result.negative|=(outstanding&0x80000000u) || (available&0x80000000u);
    result.outstanding|=outstanding!=0;result.available|=available!=0;
    result.fingerprint=(result.fingerprint^outstanding)*UINT64_C(1099511628211);
    result.fingerprint=(result.fingerprint^available)*UINT64_C(1099511628211);
  }
  return result;
}

// A bounded retry policy, not a proof that all dependencies are blocked. The
// complete original scan always runs first; no callback or queued job is skipped.
class StalledScans {
 public:
  void Reset(){streak_=0;}
  bool ShouldPark(const QueueObservation& queues,uint64_t before,uint64_t after,bool consumed) {
    if(queues.negative || queues.Idle() || consumed || before!=after) {Reset();return false;}
    if(streak_ && generation_==after && fingerprint_==queues.fingerprint) {
      if(streak_<4)++streak_;
    } else streak_=1;
    generation_=after;fingerprint_=queues.fingerprint;
    return streak_>=4;
  }
 private:
  unsigned streak_=0;
  uint64_t generation_=0,fingerprint_=0;
};

// Two workers, one independently arbitrated word each. Every notification
// changes the generation BEFORE inspecting sleeping words. A waiter arms its
// word BEFORE rechecking the generation. The kernel must atomically check the
// word while registering its wait (svcWaitForAddress on Horizon).
class WakeGate {
 public:
  static constexpr uint32_t kIdle=0,kSleeping=1,kNotified=2;
  struct Result { bool waited=false,changed=false,busy=false;uint32_t platform_result=0; };
  bool Claim(uint32_t slot) {
    if(slot>=slots_.size())return false;
    bool expected=false;
    return slots_[slot].claimed.compare_exchange_strong(expected,true,std::memory_order_seq_cst);
  }
  void Release(uint32_t slot) {
    slots_[slot].word.store(kIdle,std::memory_order_seq_cst);
    slots_[slot].claimed.store(false,std::memory_order_seq_cst);
  }
  uint64_t Snapshot()const{return generation_.load(std::memory_order_seq_cst);}
  template<class Wake> uint32_t Notify(Wake wake) {
    generation_.fetch_add(1,std::memory_order_seq_cst);
    uint32_t count=0;
    for(auto& slot:slots_) {
      uint32_t expected=kSleeping;
      if(slot.word.compare_exchange_strong(expected,kNotified,std::memory_order_seq_cst)) {
        wake(slot.word);++count;
      }
    }
    return count;
  }
  template<class Wait> Result Park(uint32_t slot,uint64_t seen,int64_t bound_ns,Wait wait) {
    if(slot>=slots_.size() || bound_ns<=0 || !slots_[slot].claimed.load(std::memory_order_seq_cst))
      return {.busy=true};
    auto& word=slots_[slot].word;
    uint32_t expected=kIdle;
    if(!word.compare_exchange_strong(expected,kSleeping,std::memory_order_seq_cst))return {.busy=true};
    struct Disarm {std::atomic<uint32_t>& word;~Disarm(){word.store(kIdle,std::memory_order_seq_cst);}} disarm{word};
    if(Snapshot()!=seen)return {.changed=true};
    const uint32_t result=wait(word,kSleeping,bound_ns);
    return {.waited=true,.changed=Snapshot()!=seen || word.load(std::memory_order_seq_cst)==kNotified,
            .platform_result=result};
  }
 private:
  struct alignas(64) Slot { std::atomic<uint32_t> word{kIdle};std::atomic<bool> claimed{false}; };
  std::array<Slot,2> slots_{};
  std::atomic<uint64_t> generation_{0};
};
static_assert(std::atomic<uint32_t>::is_always_lock_free && sizeof(std::atomic<uint32_t>)==4);
} // namespace cod6::workers
