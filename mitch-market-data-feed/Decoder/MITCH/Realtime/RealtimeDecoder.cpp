#include "RealtimeDecoder.hpp"

namespace AAX {
  namespace MITCH {
    namespace Realtime {
      DecodeStatus Decoder::decode(char *buffer, size_t len, uint64_t seq) {
        /**
        //Should put this in main.cpp, so only needs to check once.
        if (!mInit.load()) [[unlikely]] {
          mInit.store(true);
          mExpectedSequence = seq;
          auto &handler = AAX::SubscriptionHandler::instance();
          SPDLOG_INFO("First UDP Incoming seq {} ", seq);
          handler.mSnapshotRequestSequenceNo.store(mExpectedSequence);
          handler.mUDPSessionStarted.store(true);
        }
        **/
        if (seq > mExpectedSequence) [[unlikely]] {
          SPDLOG_WARN("Packet gap detected. Incoming seq {}, Expected seq {}", seq,
                      mExpectedSequence);
          mMissedCount++;
          if (mMissedCount == 2) {
            SPDLOG_WARN("Simultaneous miss from both primary and secondary! Expected seq {}, received {}",
                        mExpectedSequence, seq);
            mDroppedCount += seq - mExpectedSequence;
            mTotalCount += seq - mExpectedSequence;
            mSimultaneousMissCallback(seq, mExpectedSequence);

            parseMessage(buffer, len, seq);

            mExpectedSequence = seq;
            ++mExpectedSequence;
          }
          return PACKET_GAP;
        } else if (seq < mExpectedSequence) [[unlikely]] {
          /*
          SPDLOG_INFO("Duplicate packet detected. Incoming seq {}, Expected seq {}", seq,
                      mExpectedSequence);
                      */
          return DUPLICATE_PACKET;
        } else {
          parseMessage(buffer, len, seq);
          ++mExpectedSequence;
          mMissedCount = 0;
          ++mTotalCount;
          return EXPECTED_PACKET;
        }
      }

      void Decoder::parseMessage(char *buffer, size_t len, uint64_t seq) {
        Message::MessageHeader &msgHeader = *reinterpret_cast<Message::MessageHeader *>(buffer);
        switch (msgHeader.mMsgType) {
          case Message::kLoginResponse: {
            SPDLOG_INFO("Received login response");
            break;
          }
          case Message::kReplayResponse: {
            SPDLOG_DEBUG("Received replay response");
            break;
          }
          case Message::kSnapshotResponse: {
            SPDLOG_DEBUG("Received snapshot response");
            break;
          }
          case Message::kAddOrder: {
            auto &handler = AAX::SubscriptionHandler::instance();
            AAX::MITCH::Message::AddOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::AddOrder *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            SPDLOG_DEBUG("Received AddOrder message for {}", symbol);
            OrderBookSide side = event.mSide == 'B' ? OrderBookSide::BID : OrderBookSide::ASK;
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              SPDLOG_WARN("Added order twice!");
              return;
            }
            mOrderBookManager->onAdd(side, symbol, event.mOrderID, event.mPrice, event.mQuantity);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));
            break;
          }
          case Message::kSnapshotComplete: {
            SPDLOG_DEBUG("Received snapshot complete");
            break;
          }
          case Message::kTime: {
            SPDLOG_DEBUG("Received Time");
            break;
          }
          case Message::kSystemEvent: {
            SPDLOG_DEBUG("Received System event");
            break;
          }
          case Message::kSymbolDirectory: {
            SPDLOG_DEBUG("Received Symbol directory");
            break;
          }
          case Message::kSymbolStatus: {
            SPDLOG_DEBUG("Received Symbol status");
            break;
          }
          case Message::kAddAttributedOrder: {
            auto &handler = AAX::SubscriptionHandler::instance();
            AAX::MITCH::Message::AddAttributedOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::AddAttributedOrder *>(buffer);
            std::string symbol(reinterpret_cast<char *>(event.mSymbol), sizeof(event.mSymbol));
            SPDLOG_DEBUG("Received AddOrder message for {}", symbol);
            OrderBookSide side = event.mSide == 'B' ? OrderBookSide::BID : OrderBookSide::ASK;
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              return;
            }
            mOrderBookManager->onAdd(side, symbol, event.mOrderID, event.mPrice, event.mQuantity);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));
            break;
          }
          case Message::kOrderDeleted: {
            AAX::MITCH::Message::DeleteOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::DeleteOrder *>(buffer);
            std::string symbol = mOrderBookManager->getSymbol(event.mOrderID);
            if (symbol.empty()) {
              SPDLOG_WARN("Order doesn't exist");
              return;
            }
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              return;
            }
            mOrderBookManager->onDelete(event.mOrderID);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));
            break;
          }
          case Message::kOrderModified: {
            SPDLOG_DEBUG("Received modify order");
            AAX::MITCH::Message::ModifyOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::ModifyOrder *>(buffer);
            std::string symbol = mOrderBookManager->getSymbol(event.mOrderID);
            if (symbol.empty()) {
              SPDLOG_WARN("Order doesn't exist");
              return;
            }
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              return;
            }
            mOrderBookManager->onAmend(event.mOrderID, event.mPrice, event.mQuantity);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));
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
            AAX::MITCH::Message::ExecuteOrder &event =
                    *reinterpret_cast<AAX::MITCH::Message::ExecuteOrder *>(buffer);
            std::string symbol = mOrderBookManager->getSymbol(event.mOrderID);
            if (symbol.empty()) {
              SPDLOG_WARN("Order doesn't exist");
              return;
            }
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              return;
            }
            int64_t price;
            mOrderBookManager->onExecute(event.mOrderID, event.mQuantity, price);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));
            // generate trade 
            AAX::Message::Trade trade;
            trade.mInstrumentName = symbol;
            trade.mPrice = price;
            trade.mQty = event.mQuantity;
            AAX::MarketDataEventQueue<AAX::Message::Trade>::Instance().push(std::move(trade));
            break;
          }
          case Message::kOrderExecutedWithPriceSize: {
            SPDLOG_INFO("Received order executed with price & size");
            AAX::MITCH::Message::ExecuteOrderPriceSize &event =
                    *reinterpret_cast<AAX::MITCH::Message::ExecuteOrderPriceSize *>(buffer);
            std::string symbol = mOrderBookManager->getSymbol(event.mOrderID);
            if (symbol.empty()) {
              SPDLOG_WARN("Order doesn't exist");
              return;
            }
            if (handler.mInstrumentData[symbol].mSnapshotSequenceNo.load() >= seq) {
              return;
            }
            mOrderBookManager->onExecute(event.mOrderID, event.mPrice, event.mQuantity);
            AAX::Message::PriceDepthBook depth;
            depth.mInstrumentName = symbol;
            int i = 0;
            mOrderBookManager->visitBids(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mBids[i].mPrice = p;
              depth.mBids[i].mQty = v;
              ++i;
            }, 10);
            i = 0;
            mOrderBookManager->visitAsks(symbol, [&depth, &i](int64_t p, uint64_t v) {
              depth.mAsks[i].mPrice = p;
              depth.mAsks[i].mQty = v;
              ++i;
            }, 10);
            AAX::MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance().push(std::move(depth));

            // generate trade 
            AAX::Message::Trade trade;
            trade.mInstrumentName = symbol;
            trade.mPrice = event.mPrice;
            trade.mQty = event.mQuantity;
            AAX::MarketDataEventQueue<AAX::Message::Trade>::Instance().push(std::move(trade));
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
            SPDLOG_INFO("Received news");
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
    }
  }
}



