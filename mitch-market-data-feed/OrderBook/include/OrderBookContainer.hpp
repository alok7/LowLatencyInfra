#pragma once

#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include <vector>
#include <utility>
#include <cstdint>
#include <iostream>
#include <MarketEvents.hpp>

enum OrderBookSide {
  BID,
  ASK,
  NONE
};

class PriceLevel {
public:
  struct Order {
    Order(int64_t qty, uint64_t orderId) :
            mQty(qty), mOrderId(orderId) {}

    Order() : mQty(-1), mPrice(-1) {}

    int64_t mQty;
    uint64_t mOrderId;
    int64_t mPrice;
    OrderBookSide mSide;
  };

public:
  PriceLevel() : mVolume(-1), mPrice(-1) {}

  PriceLevel(const PriceLevel &level) {
    mPrice = level.mPrice;
    mVolume = level.mVolume;
    mOrderList = level.mOrderList;
    for (auto it = mOrderList.begin(); it != mOrderList.end(); ++it) {
      mOrderIdMap.insert({it->mOrderId, it});
    }
  }

  PriceLevel &operator=(const PriceLevel &level) {
    if (this != &level) {
      mPrice = level.mPrice;
      mVolume = level.mVolume;
      mOrderList = level.mOrderList;
      for (auto it = mOrderList.begin(); it != mOrderList.end(); ++it) {
        mOrderIdMap.insert({it->mOrderId, it});
      }
    }
    return *this;
  }

  void addOrder(uint64_t orderId, int64_t qty) {
    if (mOrderIdMap.find(orderId) == mOrderIdMap.end()) // new order should be unique order id
    {
      Order order(qty, orderId);
      auto const it = mOrderList.insert(mOrderList.end(), order);
      mOrderIdMap.insert({order.mOrderId, it});
      mVolume += order.mQty;
    }
  }

  void deleteOrder(uint64_t orderId) {
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

  inline int64_t getLevelVolume() noexcept {
    return mVolume;
  }

  int64_t getVolume(uint64_t orderId) noexcept {
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

template<typename KEY, typename VALUE>
class OrderBookContainer {
private:
  std::map<KEY, VALUE, std::less<KEY>> _asks;
  std::map<KEY, VALUE, std::greater<KEY>> _bids;
  using iterator = typename std::map<const KEY, VALUE>::iterator;

  // by price level
  template<typename _container_type>
  void _insert(_container_type &container, const VALUE &price_level) {
    container[price_level.mPrice] = price_level;
  }

  template<typename _container_type>
  bool _remove(_container_type &container, const KEY &price) {
    return container.erase(price);
  }

  // by order book oid 
  template<typename _container_type>
  void _insert(_container_type &container, uint64_t oid, int64_t p, int64_t v) {
    if (container.find(p) == container.end()) {
      VALUE price_level;
      price_level.mPrice = p;
      price_level.addOrder(oid, v);
      _insert(container, price_level);
    } else {
      container[p].addOrder(oid, v);
    }
  }

  template<typename _container_type>
  void _remove(_container_type &container, uint64_t oid, int64_t p, bool &removed) {
    auto it = container.find(p);
    if (it != container.end()) {
      removed = true;
      (it->second).deleteOrder(oid);
      if ((it->second).mVolume <= 0)_remove(container, p);
    }
    removed = false;
  }

  template<typename _container_type>
  void _update(_container_type &container, uint64_t oid, int64_t orig_p, int64_t p, int64_t v) {
    auto it = container.find(orig_p);
    if (it != container.end()) {
      (it->second).deleteOrder(oid);
      if ((it->second).mVolume <= 0)_remove(container, it->first);
      if (v > 0) _insert(container, oid, p, v);
    }
  }

public:

  void insert(OrderBookSide side, uint64_t oid, int64_t p, int64_t v) {
    switch (side) {
      case OrderBookSide::BID:
        _insert(_bids, oid, p, v);
        break;
      case OrderBookSide::ASK:
        _insert(_asks, oid, p, v);
        break;
    }
  }

  void update(OrderBookSide side, uint64_t oid, int64_t orig_p, int64_t p, int64_t v) {
    switch (side) {
      case OrderBookSide::BID:
        _update(_bids, oid, orig_p, p, v);
        break;
      case OrderBookSide::ASK:
        _update(_asks, oid, orig_p, p, v);
        break;
    }
  }

  void remove(OrderBookSide side, uint64_t oid, int64_t p) {
    bool removed = false;
    switch (side) {
      case OrderBookSide::BID:
        _remove(_bids, oid, p, removed);
        break;
      case OrderBookSide::ASK:
        _remove(_asks, oid, p, removed);
        break;
    }
  }

  template<typename F>
  void visitBids(F &&f, int N) {
    uint i = 0;
    for (auto it = _bids.begin(); it != _bids.end() && i < N; it++) {
      f(it->first, it->second.getLevelVolume());
      i++;
    }
  }

  template<typename F>
  void visitAsks(F &&f, int N) {
    uint i = 0;
    for (auto it = _asks.begin(); it != _asks.end() && i < N; it++) {
      f(it->first, it->second.getLevelVolume());
      i++;
    }
  }
};
