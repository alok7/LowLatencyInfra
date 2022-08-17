#pragma once

#include <string>
#include <ctime>
#include <vector>
#include <array>
#include <list>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <cmath>
#include <Instrument.hpp>


using priceType = int64_t;
using qtyType = int64_t;
using TimePointType = int64_t;

namespace AAX {
  namespace Message {

    struct MarketEventHeader {
      TimePointType mApiIncomingTime;
      TimePointType mApiOutgoingTime; // api internal latency = apiOutgoingTime - apiIncomingTime
      TimePointType mExchangeOutgoingTime; // network latency = apiIncomingTime - exchangeOutgoingTime;
      size_t mPacketSequence;
      size_t mMessageSequence;

    };

    struct MarketEvent : MarketEventHeader {
      //MarketType mVenue;
      InstrumentId mInstrumentId;
      InstrumentName mInstrumentName;
    };


    template<size_t N>
    struct DepthBookL2 : MarketEvent // L2
    {
      struct PriceLevel {
        priceType mPrice;
        qtyType mQty;

        priceType getPrice() { return mPrice; }

        qtyType getQuantity() { return mQty; }

        PriceLevel(priceType price_, qtyType qty_) : mPrice(price_), mQty(qty_) {}

        PriceLevel() : mPrice(-1), mQty(-1) {}
      };

      std::array<PriceLevel, N> mBids;
      std::array<PriceLevel, N> mAsks;
    };

    template<size_t N>
    struct DepthBookL3 : MarketEvent // L3
    {
      class PriceLevel {
      public:
        struct Order {
          Order(int64_t qty, uint64_t orderId) :
                  mQty(qty), mOrderId(orderId) {}

          int64_t mQty;
          uint64_t mOrderId;
        };

      public:
        PriceLevel() : mVolume(0) {}

        PriceLevel(const PriceLevel &level) {
          mOrderList = level.mOrderList;
          for (auto it = mOrderList.begin(); it != mOrderList.end(); ++it) {
            mOrderIdMap.insert({it->mOrderId, it});
          }
        }

        PriceLevel &operator=(const PriceLevel &level) {
          if (this != &level) {
            mOrderList = level.mOrderList;
            for (auto it = mOrderList.begin(); it != mOrderList.end(); ++it) {
              mOrderIdMap.insert({it->mOrderId, it});
            }
          }
          return *this;
        }

        void insert(uint64_t orderId, int64_t qty) {
          if (mOrderIdMap.find(orderId) == mOrderIdMap.end()) // new order should be unique order id
          {
            Order order(qty, orderId);
            auto const it = mOrderList.insert(mOrderList.end(), order);
            mOrderIdMap.insert({order.mOrderId, it});
            mVolume += order.mQty;
          }
        }

        void erase(uint64_t orderId) {
          auto p = mOrderIdMap.find(orderId);
          if (p != mOrderIdMap.end()) {
            auto orderPtr = p->second;
            mVolume -= orderPtr->mQty;
            mOrderList.erase(orderPtr);
            mOrderIdMap.erase(p);
          }
        }

        void modify(uint64_t orderId, int64_t qty) {
          auto p = mOrderIdMap.find(orderId);
          if (p != mOrderIdMap.end()) {
            auto orderPtr = p->second;
            mVolume += (qty - orderPtr->mQty);
            orderPtr->mQty = qty;
          }
        }

        void cancel(uint64_t orderId, int64_t qty) {
          auto p = mOrderIdMap.find(orderId);
          if (p != mOrderIdMap.end()) {
            auto orderPtr = p->second;
            mVolume -= qty;
            orderPtr->mQty -= qty;
            if (orderPtr->mQty <= 0) {
              mOrderList.erase(orderPtr);
              mOrderIdMap.erase(p);
            }
          }
        }

        inline bool empty() noexcept {
          return mOrderList.empty();
        }

        inline bool contains(uint64_t orderId) {
          return mOrderIdMap.find(orderId) != mOrderIdMap.end();
        }

        inline uint64_t getLevelVolume() noexcept {
          return mVolume;
        }

        uint64_t getVolume(uint64_t orderId) noexcept {
          auto p = mOrderIdMap.find(orderId);
          if (p != mOrderIdMap.end()) {
            auto orderPtr = p->second;
            return orderPtr->mQty;
          }
          return 0;
        }

      public:
        int64_t mPrice;
        int64_t mVolume;
      private:
        std::list<Order> mOrderList;
        std::unordered_map<uint64_t, typename std::list<Order>::iterator> mOrderIdMap;
      };

      std::array<PriceLevel, N> mBids;
      std::array<PriceLevel, N> mAsks;

    };

    using PriceDepthBook = DepthBookL2<10>;
    using OrderBook = DepthBookL3<10>;

    class Trade : public MarketEvent {
    public:
      priceType mPrice;
      qtyType mQty;
      char mSide; // 'B', 'S', 'N'
      Trade(priceType pr, qtyType sz, char s) : mPrice(pr), mQty(sz), mSide(s) {}

      Trade() {}
    };

  }
}
