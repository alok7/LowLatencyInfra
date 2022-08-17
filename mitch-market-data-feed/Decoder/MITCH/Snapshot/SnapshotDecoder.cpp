#include "SnapshotDecoder.hpp"
#include "spdlog/spdlog.h"
#include <unistd.h>

namespace AAX {
  namespace MITCH {
    namespace Snapshot {
      void Decoder::decode(char *buffer, size_t len) {
        Message::MessageHeader &msgHeader = *reinterpret_cast<Message::MessageHeader *>(buffer);
        //SPDLOG_INFO("Decoding {} message", Message::kMsgTypeHexToString[msgHeader.mMsgType]);
        switch (msgHeader.mMsgType) {
          case Message::kLoginResponse: {
            SPDLOG_INFO("Received login response!");
            AAX::MITCH::Message::LoginResponse &loginResponseEvent =
                    *reinterpret_cast<AAX::MITCH::Message::LoginResponse *>(buffer);
            handleLogin(loginResponseEvent);
            break;
          }
          case Message::kReplayResponse: {
            //SPDLOG_INFO("Received replay response");
            break;
          }
          case Message::kSnapshotResponse: {
            AAX::MITCH::Message::SnapshotResponse &event =
                    *reinterpret_cast<AAX::MITCH::Message::SnapshotResponse *>(buffer);
            SPDLOG_DEBUG("Received snapshot response: seq {}, status {}, snapshot_type {}", event.mSequenceNo,
                         event.mStatus, event.mSnapshotType);
            break;
          }
          case Message::kAddOrder: {
            SPDLOG_DEBUG("Received Add order ");
            AAX::MITCH::Message::AddOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::AddOrder *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            OrderBookSide side = event.mSide == 'B' ? OrderBookSide::BID : OrderBookSide::ASK;
            //SPDLOG_INFO("add- seq: {}, symbol:{}", event.mOrderID, event.mSymbol);
            mOrderBookManager->onAdd(side, symbol, event.mOrderID, event.mPrice, event.mQuantity);
            break;
          }
          case Message::kSnapshotComplete: {
            AAX::MITCH::Message::SnapshotComplete &event =
                    *reinterpret_cast<AAX::MITCH::Message::SnapshotComplete *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            SPDLOG_DEBUG("Snapshot complete: seq {}, symbol {}", event.mSequenceNo, symbol);
            auto &handler = AAX::SubscriptionHandler::instance();
            if (event.mSnapshotType == 0) {
              //Final Snapshot Complete message has a sequence number of 0.
              if (event.mSequenceNo == 0) {
                SPDLOG_INFO("Orderbook snapshots complete!");
                handler.mSnapshotCompleted.store(true);
                handler.startRealtimeConsuming();
              } else {
                handler.mInstrumentData[symbol].mSymbol = symbol;
                handler.mInstrumentData[symbol].mSnapshotSequenceNo.store(event.mSequenceNo);
                handler.mInstrumentData[symbol].mTradingStatus = event.mTradingStatus;
              }
            }
            break;
          }
          case Message::kTime: {
            //SPDLOG_INFO("Received Time");
            //mSession->sendHeartbeat();
            break;
          }
          case Message::kSystemEvent: {
            //SPDLOG_INFO("Received System event");
            break;
          }
          case Message::kSymbolDirectory: {
            AAX::MITCH::Message::SymbolDirectory &event =
                    *reinterpret_cast<AAX::MITCH::Message::SymbolDirectory *>(buffer);
            SPDLOG_DEBUG("{} {}", event.mSymbol, event.mSegment, event.mSubBook);
            break;
          }
          case Message::kSymbolStatus: {
            SPDLOG_DEBUG("Received Symbol status");
            break;
          }
          case Message::kAddAttributedOrder: {
            SPDLOG_DEBUG("Received add attributed order");
            AAX::MITCH::Message::AddAttributedOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::AddAttributedOrder *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            OrderBookSide side = event.mSide == 'B' ? OrderBookSide::BID : OrderBookSide::ASK;
            //SPDLOG_INFO("add- seq: {}, symbol:{}", event.mOrderID, event.mSymbol);
            mOrderBookManager->onAdd(side, symbol, event.mOrderID, event.mPrice, event.mQuantity);
            break;
          }
          case Message::kOrderDeleted: {
            SPDLOG_DEBUG("Received delete order");
            break;
          }
          case Message::kOrderModified: {
            SPDLOG_DEBUG("Received modify order");
            break;
          }
          case Message::kOrderBookClear: {
            SPDLOG_INFO("Received order book clear");
            AAX::MITCH::Message::ClearBook &event =
                    *reinterpret_cast<AAX::MITCH::Message::ClearBook *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            mOrderBookManager->clear(symbol);
            break;
          }
          case Message::kOrderExecuted: {
            SPDLOG_DEBUG("Received order executed");
            break;
          }
          case Message::kOrderExecutedWithPriceSize: {
            SPDLOG_DEBUG("Received order executed with price & size");
            break;
          }
          case Message::kTrade: {
            SPDLOG_DEBUG("Received trade");
            break;
          }
          case Message::kAuctionTrade: {
            SPDLOG_DEBUG("Received auction trade");
            break;
          }
          case Message::kOffBookTrade: {
            SPDLOG_DEBUG("Received offbook trade");
            break;
          }
          case Message::kRecoveryTrade: {
            SPDLOG_DEBUG("Received recovery trade");
            break;
          }
          case Message::kAuctionInfo: {
            SPDLOG_DEBUG("Received auction info");
            break;
          }
          case Message::kExtendedStatistics: {
            SPDLOG_DEBUG("Received extended statistics");
            break;
          }
          case Message::kNews: {
            SPDLOG_DEBUG("Received news");
            break;
          }
          case Message::kUpdatePricePoint: {
            SPDLOG_DEBUG("Received price point");
            break;
          }
          case Message::kDeletePricePoint: {
            SPDLOG_DEBUG("Received delete price point");
            break;
          }
          case Message::kTopOfBook: // TOB
          {
            SPDLOG_DEBUG("Received top of book");
            break;
          }
          case Message::kIndicativeQuoteInfo: {
            SPDLOG_DEBUG("Received indicative quote");
            break;
          }
          default: {
            SPDLOG_WARN("No case to handle message of type {}!", Message::kMsgTypeHexToString[msgHeader.mMsgType]);
            break;
          }
        }
      }

      void Decoder::handleLogin(const AAX::MITCH::Message::LoginResponse &loginResponse) {
        switch (loginResponse.mStatus) {
          case 'A': {
            SPDLOG_INFO("Logged in successfully!");
            /**
            auto &handler = AAX::SubscriptionHandler::instance();
            while (!handler.mUDPSessionStarted.load()) {
              usleep(100);
            }
            **/
            mSnapshotSession->sendOrderBookRequestPacket();
            //mSession->sendRequestInstrumentsPacket();
            break;
          }

            /**
            case 'a': {
              break;
            }
            case 'b': {
              break;
            }
            case 'c': {
              break;
            }
            case 'd': {
              break;
            }
            case 'e': {
              break;
            }
            **/
          default: {
            SPDLOG_WARN("Failed to login, with status response {}!", loginResponse.mStatus);
            break;
          }
        }
      }
    }
  }
}



