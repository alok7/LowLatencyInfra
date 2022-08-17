#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <future>
#include <deque>
#include <MitchSchema.hpp>
#include <unordered_map>
#include <MDSessionConfig.hpp>
#include <TCPTransport.hpp>
#include <UDPTransport.hpp>
#include <spdlog/common.h>

using Clock = std::chrono::high_resolution_clock;

namespace AAX {
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

  struct MessageBuffer {
    struct Packet {
      std::string mBuffer;
      uint64_t mSeq;
    };

    void insertBuffer(const char *buffer, size_t len, uint64_t seq) {
      //SPDLOG_INFO("Buffered one message!");
      std::lock_guard<SpinLock> lock(mSyncRequest);
      Packet pckt;
      pckt.mBuffer = std::string(buffer, len);
      pckt.mSeq = seq;
      mUDPBuffer.push_back(pckt);
    }

    Packet &getFrontBuffer() {
      std::lock_guard<SpinLock> lock(mSyncRequest);
      return mUDPBuffer.front();
    }

    void removeFrontBuffer() {
      std::lock_guard<SpinLock> lock(mSyncRequest);
      mUDPBuffer.pop_front();
    }

    size_t sizeBuffer() {
      std::lock_guard<SpinLock> lock(mSyncRequest);
      return mUDPBuffer.size();
    }

    std::deque<Packet> mUDPBuffer;
    SpinLock mSyncRequest;
  };

  struct InstrumentData {
    std::string mSymbol;
    char mTradingStatus;
    int32_t mStikePrice;
    std::string mExpirationDate;
    std::string mUnderlying;
    std::atomic<uint64_t> mSnapshotSequenceNo;
  };


  class SubscriptionHandler {
  private:
    SubscriptionHandler() {
    }

  public:
    static auto &instance() {
      static SubscriptionHandler handler;
      return handler;
    }

    ~SubscriptionHandler() {
      if (mPrimaryRealtimeThread.joinable()) // make sure, thread is not stuck here
      {
        mPrimaryRealtimeThread.join();
      }
    }

    void init(AAX::TCP::MDSession *tcp_session, AAX::UDP::MDSession *udp_session,
              AAX::UDP::MDSession *udp_session_secondary) {
      mSnapshotSession = tcp_session;
      mPrimaryRealtimeSession = udp_session;
      mSecondaryRealtimeSession = udp_session_secondary;
      isInit.store(true);
    }

    void startRealtimeConsuming() {
      mStartTime = Clock::now();
      mRunning.store(true);

      mPrimaryCanConsume.store(true);
      mSecondaryCanConsume.store(false);

      mPrimaryRealtimeThread = std::thread([this]() {
        SPDLOG_INFO("Primary realtime UDP consumption started!");
        while (isInit.load()) {
          //optimise this to return a false dummy event instead of waiting on sizeBuffer.
          if (mPrimaryCanConsume.load()) {
            if (mPrimaryRealtimeMessageBuffer.sizeBuffer() > Config::ACTIVE_REALTIME_CACHE_SIZE) {
              auto &packet = mPrimaryRealtimeMessageBuffer.getFrontBuffer();
              switch (mPrimaryRealtimeSession->decode(packet.mBuffer.data(), packet.mBuffer.size(), packet.mSeq)) {
                case AAX::DecodeStatus::EXPECTED_PACKET: {
                  mPrimaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
                case AAX::DecodeStatus::PACKET_GAP: {
                  SPDLOG_INFO("Packet gap detected, swapping over to secondary channel.");
                  mPrimaryCanConsume.store(false);
                  mSecondaryCanConsume.store(true);
                  mPri2SecSwitchChannelCallback(packet.mSeq);
                  mPrimaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
                case AAX::DecodeStatus::DUPLICATE_PACKET: {
                  mPrimaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
              }
            }
          } else {
            if (mPrimaryRealtimeMessageBuffer.sizeBuffer() > Config::BACKUP_REALTIME_CACHE_SIZE) {
              mPrimaryRealtimeMessageBuffer.removeFrontBuffer();
            }
          }
        }
      });

      mSecondaryRealtimeThread = std::thread([this]() {
        SPDLOG_INFO("Secondary UDP consumption started!");
        while (isInit.load()) {
          //optimise this to return a false dummy event instead of waiting on sizeBuffer.
          if (mSecondaryCanConsume.load()) {
            if (mSecondaryRealtimeMessageBuffer.sizeBuffer() > Config::ACTIVE_REALTIME_CACHE_SIZE) {
              auto &packet = mSecondaryRealtimeMessageBuffer.getFrontBuffer();
              switch (mSecondaryRealtimeSession->decode(packet.mBuffer.data(), packet.mBuffer.size(), packet.mSeq)) {
                case AAX::DecodeStatus::EXPECTED_PACKET: {
                  mSecondaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
                case AAX::DecodeStatus::PACKET_GAP: {
                  SPDLOG_INFO("Packet gap detected, swapping over to primary channel.");
                  mSecondaryCanConsume.store(false);
                  mPrimaryCanConsume.store(true);
                  mSec2PriSwitchChannelCallback(packet.mSeq);
                  mSecondaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
                case AAX::DecodeStatus::DUPLICATE_PACKET: {
                  mSecondaryRealtimeMessageBuffer.removeFrontBuffer();
                  break;
                }
              }
            }
          } else {
            if (mSecondaryRealtimeMessageBuffer.sizeBuffer() > Config::BACKUP_REALTIME_CACHE_SIZE) {
              mSecondaryRealtimeMessageBuffer.removeFrontBuffer();
            }
          }
        }
      });
    }

    uint64_t getFirstPrimaryRealtimeMessageSeqNo() {
      auto p = mPrimaryRealtimeMessageBuffer.getFrontBuffer();
      return p.mSeq;
    }
    uint64_t getFirstSecondaryRealtimeMessageSeqNo() {
      auto p = mSecondaryRealtimeMessageBuffer.getFrontBuffer();
      return p.mSeq;
    }

    void stop() {
      mRunning.store(false);
      isInit.store(false);
    }

    template<typename Pri2SecCallBack>
    void registerPri2SecCallBack(Pri2SecCallBack &&f) {
      mPri2SecSwitchChannelCallback = f;
    }

    template<typename Sec2PriCallBack>
    void registerSec2PriCallBack(Sec2PriCallBack &&f) {
      mSec2PriSwitchChannelCallback = f;
    }

  public:
    std::unordered_map<std::string, InstrumentData> mInstrumentData;
    std::atomic<bool> mTCPSessionStarted = {false};
    std::atomic<bool> mUDPSessionStarted = {false};
    std::atomic<bool> mSnapshotCompleted = {false};
    std::atomic<bool> mPrimaryCanConsume = {false};
    std::atomic<bool> mSecondaryCanConsume = {false};
    std::atomic<uint64_t> mSnapshotRequestSequenceNo;
    MessageBuffer mPrimaryRealtimeMessageBuffer;
    MessageBuffer mSecondaryRealtimeMessageBuffer;

  private:
    std::function<void(uint64_t)> mPri2SecSwitchChannelCallback;
    std::function<void(uint64_t)> mSec2PriSwitchChannelCallback;
    AAX::TCP::MDSession *mSnapshotSession;
    AAX::UDP::MDSession *mPrimaryRealtimeSession;
    AAX::UDP::MDSession *mSecondaryRealtimeSession;
    std::atomic<bool> isInit = false;
    std::atomic<bool> mRunning = false;
    std::thread mPrimaryRealtimeThread;
    std::thread mSecondaryRealtimeThread;
    Clock::time_point mStartTime;
  };
}
