#pragma once

#include <MarketEvents.hpp>
#include <EventProcessingQueue.hpp>
#include <MitchSchema.hpp>
#include <SubscriptionHandler.hpp>
#include <OrderBookBuilding.hpp>
#include <iostream>
#include <cassert>
#include <SubscriptionHandler.hpp>
#include <spdlog/common.h>

namespace AAX {
  namespace MITCH {
    namespace Realtime {

      class Decoder {
      public:
        Decoder() {}

        void setOrderBook(OrderBookBuilding *book) {
          mOrderBookManager = book;
        }

        void parseMessage(char *buffer, size_t len, uint64_t seq);

        DecodeStatus decode(char *buffer, size_t len, uint64_t seq);

        template<typename SimultaneousMissCallBack>
        void registerSimultaneousMissCallBack(SimultaneousMissCallBack &&f) {
          mSimultaneousMissCallback = f;
        }

      public:
        uint64_t mExpectedSequence{0};
        uint64_t mMissedCount{0};
        std::atomic<bool> mInit = {false};

        std::atomic<uint64_t> mDroppedCount{0};
        std::atomic<uint64_t> mTotalCount{0};
      private:
        OrderBookBuilding *mOrderBookManager;
        std::function<void()> mPacketDropCallback;
        std::function<void(uint64_t, uint64_t)> mSimultaneousMissCallback;
        AAX::SubscriptionHandler &handler = AAX::SubscriptionHandler::instance();
      };
    }
  }
}
