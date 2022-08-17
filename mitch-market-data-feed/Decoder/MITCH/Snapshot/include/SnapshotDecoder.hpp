#pragma once

#include <MarketEvents.hpp>
#include <EventProcessingQueue.hpp>
#include <MitchSchema.hpp>
#include <SubscriptionHandler.hpp>
#include <OrderBookBuilding.hpp>
#include <iostream>
#include <cassert>
#include <MDSessionConfig.hpp>
#include <SubscriptionHandler.hpp>

namespace AAX {
  namespace MITCH {
    namespace Snapshot {
      class Decoder {
      public:
        Decoder() {}

        template<typename LogInCallBack>
        void registerLogInCallBack(LogInCallBack &&f) {
          mdLogInCallBack = f;
        }

        template<typename SymbolSnapshotCompleteCallback>
        void registerSymbolSnapshotCompleteCallback(SymbolSnapshotCompleteCallback &&f) {
          mdSymbolSnapshotCompleteCallback = f;
        }

        template<typename OrderbookSnapshotCompleteCallback>
        void registerOrderbookSnapshotCompleteCallback(OrderbookSnapshotCompleteCallback &&f) {
          mdOrderbookSnapshotCompleteCallback = f;
        }

        void registerMDSession(AAX::TCP::MDSession *session) {
          mSnapshotSession = session;
        }

        void decode(char *buffer, size_t len);

        void handleLogin(const AAX::MITCH::Message::LoginResponse &loginResponse);

        void setOrderBook(OrderBookBuilding *book) {
          mOrderBookManager = book;
        }

      private:
        AAX::TCP::MDSession *mSnapshotSession;
        SubscriptionHandler &handler = AAX::SubscriptionHandler::instance();
        OrderBookBuilding *mOrderBookManager;
        std::function<void()> mdLogInCallBack;
        std::function<void()> mdSymbolSnapshotCompleteCallback;
        std::function<void()> mdOrderbookSnapshotCompleteCallback;
        std::atomic<bool> mInit = {false};
      };
    }
  }
}
