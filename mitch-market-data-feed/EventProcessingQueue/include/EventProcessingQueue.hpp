#pragma once

#include <mutex>
#include <atomic>
#include <chrono>
#include <utility>
#include <array>
#include <MDSessionConfig.hpp>
#include <thread>

namespace AAX {

  template<typename T>
  struct RBSize {
  };

  template<>
  struct RBSize<Message::Trade> {
    static const int64_t size = AAX::Config::TRADE_QUEUE_SIZE;
  };

  template<>
  struct RBSize<Message::PriceDepthBook> {
    static const int64_t size = AAX::Config::DEPTHBOOK_QUEUE_SIZE;
  };

  template<typename EventType, int64_t RB_CAPACITY = RBSize<EventType>::size>
  class MarketDataEventQueue {
  private:
    MarketDataEventQueue() {

    }

  public:
    static auto &Instance() {
      static MarketDataEventQueue<EventType> instance;
      return instance;
    };

    bool try_push(EventType &&event) {
      std::lock_guard<SpinLock> lock(_syncRequest);
      int64_t current_write_pos = writer_pos.load();
      int64_t next_write_pos = increaseIndex(current_write_pos);
      queueImpl[current_write_pos] = event;
      writer_pos.store(next_write_pos);
      return true;
    }

    void push(EventType &&event) {
      while (!try_push(std::move(event))) {
        std::this_thread::yield();
      }
    }

    std::pair<EventType, bool> try_pop() {
      static EventType dummy;
      std::lock_guard<SpinLock> lock(_syncRequest);
      if (reader_pos.load() == writer_pos.load()) // If reader crosses writer, cross check
      {
        return std::make_pair(dummy, false);
      }
      auto ret = std::make_pair(queueImpl[reader_pos.load()], true);
      reader_pos.store(increaseIndex(reader_pos.load()));
      return ret;
    }

    std::pair<EventType, bool> pop() {
      return try_pop();
    }

  private:
    inline int64_t increaseIndex(int64_t pos) {
      return (pos + 1) & (queue_capacity - 1); //(pos+1)%queue_capacity
    }

    const static int64_t queue_capacity = RB_CAPACITY;
    std::array<EventType, queue_capacity> queueImpl; // replace with any other other implementation here if needed
    alignas(64) std::atomic<int64_t> writer_pos = 0;
    alignas(64) std::atomic<int64_t> reader_pos = 0;

    struct SpinLock {
      void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) { ; }
      }

      void unlock() {
        locked.clear(std::memory_order_release);
      }

    private:
      std::atomic_flag locked = ATOMIC_FLAG_INIT;
    };

    SpinLock _syncRequest;
  };

}
