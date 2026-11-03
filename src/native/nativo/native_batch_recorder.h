// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>

namespace cod6::native {
// One producer owns Enqueue/Publish/Drain/Stop. The consumer only executes
// copied commands. Resources referenced by a command remain the renderer's
// responsibility: drain before recording immediate commands or retiring them.
/*
 * COD6_BATCH256
 *
 * El lote de publicacion era 64. Con ~5.800 dibujos por fotograma eso son unas 90
 * publicaciones por fotograma, y cada una es un store atomico mas un possible
 * notify. La tanda 254 medía capacity_waits=0, asi que el productor nunca se
 * quedaba sin sitio: subir el lote solo quita publicaciones, no anade espera.
 *
 * Las dos cotas son potencias de dos y Capacity >= Batch, que es lo que exige el
 * static_assert de abajo.
 */
template<class Command,uint32_t Capacity=4096,uint32_t Batch=256>
class BatchRecorder {
 public:
  static_assert(Capacity>=Batch && Batch>0 && (Capacity&(Capacity-1))==0);
  static_assert(std::is_trivially_copyable_v<Command>);
  static constexpr uint32_t kBatchSize=Batch;
  struct Stats {
    uint64_t queued=0,publications=0,wakes=0,capacity_waits=0,drains=0;
    uint64_t capacity_wait_ns=0,drain_wait_ns=0,executed=0,executed_batches=0,execute_ns=0;
  };
  ~BatchRecorder(){Stop();}
  // Lifecycle handoff only: the old producer must have stopped, and its
  // writes must be externally synchronized (thread start/join in the host).
  void AdoptProducer(){owner_=std::this_thread::get_id();}
  void Start(std::function<void(const Command&)> execute,std::function<void()> startup={}) {
    assert(!active_ && !worker_.joinable());
    commands_=std::make_unique<Command[]>(Capacity);
    execute_=std::move(execute);startup_=std::move(startup);
    AdoptProducer();
    written_=published_seen_=done_seen_=0;
    stats_={};published_.store(0);done_.store(0);stop_.store(false);
    consumer_sleeping_.store(false);producer_waiting_.store(false);
    executed_batches_.store(0);execute_ns_.store(0);
    worker_=std::thread([this]{Run();});
    active_=true;
  }
  void Enqueue(const Command& command) {
    AssertOwner();assert(active_);
    if(written_+1-done_seen_>Capacity) {
      done_seen_=done_.load(std::memory_order_acquire);
      if(written_+1-done_seen_>Capacity) {
        ++stats_.capacity_waits;const auto begin=Now();
        Publish();WaitDone(written_+1-Capacity);
        stats_.capacity_wait_ns+=Now()-begin;
        done_seen_=done_.load(std::memory_order_acquire);
      }
    }
    commands_[written_%Capacity]=command;
    ++written_;++stats_.queued;
    if(written_-published_seen_>=Batch)Publish();
  }
  void Publish() {
    AssertOwner();
    if(!active_ || written_==published_seen_)return;
    // The consumer announces sleep before rechecking this cursor. SC orders
    // the cursor store before our sleeping load, so one side sees the other.
    published_.store(written_,std::memory_order_seq_cst);
    published_seen_=written_;++stats_.publications;
    if(consumer_sleeping_.load(std::memory_order_seq_cst)) {
      ++stats_.wakes;std::lock_guard lock(mutex_);work_.notify_one();
    }
  }
  void Drain() {
    AssertOwner();
    if(!active_ || done_.load(std::memory_order_acquire)==written_)return;
    ++stats_.drains;const auto begin=Now();
    Publish();WaitDone(written_);
    stats_.drain_wait_ns+=Now()-begin;
    done_seen_=done_.load(std::memory_order_acquire);
  }
  void Stop() {
    if(!active_)return;
    AssertOwner();Drain();stop_.store(true,std::memory_order_release);
    {std::lock_guard lock(mutex_);work_.notify_one();}
    worker_.join();active_=false;commands_.reset();
  }
  Stats ReadStats()const {
    AssertOwner();Stats result=stats_;
    result.executed=done_.load(std::memory_order_acquire);
    result.executed_batches=executed_batches_.load(std::memory_order_relaxed);
    result.execute_ns=execute_ns_.load(std::memory_order_relaxed);
    return result;
  }
 private:
  static uint64_t Now() {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count());
  }
  void AssertOwner()const {assert(!active_ || owner_==std::this_thread::get_id());}
  void WaitDone(uint64_t target) {
    if(done_.load(std::memory_order_acquire)>=target)return;
    std::unique_lock lock(mutex_);
    producer_waiting_.store(true,std::memory_order_seq_cst);
    progress_.wait(lock,[&]{return done_.load(std::memory_order_seq_cst)>=target;});
    producer_waiting_.store(false,std::memory_order_seq_cst);
  }
  void Run() {
    if(startup_)startup_();
    uint64_t tail=0;
    for(;;) {
      const uint64_t head=published_.load(std::memory_order_acquire);
      if(tail<head) {
        const uint64_t end=std::min(head,tail+Batch);const auto begin=Now();
        for(;tail<end;++tail)execute_(commands_[tail%Capacity]);
        execute_ns_.fetch_add(Now()-begin,std::memory_order_relaxed);
        executed_batches_.fetch_add(1,std::memory_order_relaxed);
        done_.store(tail,std::memory_order_seq_cst);
        if(producer_waiting_.load(std::memory_order_seq_cst)) {
          std::lock_guard lock(mutex_);progress_.notify_one();
        }
        continue;
      }
      std::unique_lock lock(mutex_);
      consumer_sleeping_.store(true,std::memory_order_seq_cst);
      work_.wait(lock,[&]{return stop_.load(std::memory_order_acquire) ||
                               published_.load(std::memory_order_seq_cst)>tail;});
      consumer_sleeping_.store(false,std::memory_order_seq_cst);
      if(stop_.load(std::memory_order_acquire) && published_.load(std::memory_order_acquire)==tail)return;
    }
  }
  std::unique_ptr<Command[]> commands_;
  std::function<void(const Command&)> execute_;
  std::function<void()> startup_;
  std::thread worker_;
  std::thread::id owner_;
  bool active_=false;
  alignas(64) uint64_t written_=0,published_seen_=0,done_seen_=0;
  Stats stats_;
  alignas(64) std::atomic<uint64_t> published_{0};
  alignas(64) std::atomic<uint64_t> done_{0};
  alignas(64) std::atomic<uint64_t> executed_batches_{0},execute_ns_{0};
  std::atomic<bool> stop_{false},consumer_sleeping_{false},producer_waiting_{false};
  std::mutex mutex_;
  std::condition_variable work_,progress_;
};
} // namespace cod6::native
