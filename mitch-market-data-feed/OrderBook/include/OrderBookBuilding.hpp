#pragma once

#include <OrderBookContainer.hpp>

class OrderBookBuilding {
private:
  using Side = OrderBookSide;
  using OrderBookType = OrderBookContainer<int64_t, PriceLevel>; // key = price
  std::unordered_map<std::string, OrderBookType> mOrderBookContainers; // key = symbol, use fixedstring
public:
  void onAdd(Side side, const std::string &symbol, uint64_t oid, int64_t p, int64_t v) noexcept {
    PriceLevel::Order order;
    order.mOrderId = oid;
    order.mQty = v;
    order.mPrice = p;
    order.mSide = side;

    mOrders[oid] = order;
    mOrderSymbol[oid] = symbol;
    mOrderBookContainers[symbol].insert(side, oid, p, v);
  }

  void onDelete(uint64_t oid) noexcept {
    auto it_order = mOrders.find(oid);
    auto it_symbol = mOrderSymbol.find(oid);
    if (it_order != mOrders.end() && it_symbol != mOrderSymbol.end()) {
      auto p = (it_order->second).mPrice;
      auto side = (it_order->second).mSide;
      auto symbol = (it_symbol->second);
      mOrders.erase(oid);
      mOrderSymbol.erase(oid);
      mOrderBookContainers[symbol].remove(side, oid, p);
    }
  }

  void onAmend(uint64_t oid, int64_t p, int64_t v) noexcept {
    auto it_order = mOrders.find(oid);
    auto it_symbol = mOrderSymbol.find(oid);
    if (it_order != mOrders.end() && it_symbol != mOrderSymbol.end()) {
      auto orig_p = (it_order->second).mPrice;
      auto side = (it_order->second).mSide;
      auto symbol = (it_symbol->second);
      (it_order->second).mPrice = p;
      (it_order->second).mQty = v;
      if (v == 0) {
        mOrders.erase(oid);
        mOrderSymbol.erase(oid);
      }
      mOrderBookContainers[symbol].update(side, oid, orig_p, p, v);
    }
  }

  void onExecute(uint64_t oid, int64_t v, int64_t &orig_p) {
    auto it_order = mOrders.find(oid);
    auto it_symbol = mOrderSymbol.find(oid);
    if (it_order != mOrders.end() && it_symbol != mOrderSymbol.end()) {
      auto p = (it_order->second).mPrice;
      orig_p = p;
      auto side = (it_order->second).mSide;
      auto symbol = (it_symbol->second);
      (it_order->second).mPrice = p;
      (it_order->second).mQty -= v;
      if ((it_order->second).mQty <= 0) {
        mOrders.erase(oid);
        mOrderSymbol.erase(oid);
      }
      mOrderBookContainers[symbol].update(side, oid, p, p, (it_order->second).mQty);
    }
  }

  void onExecute(uint64_t oid, int64_t v) {
    auto it_order = mOrders.find(oid);
    auto it_symbol = mOrderSymbol.find(oid);
    if (it_order != mOrders.end() && it_symbol != mOrderSymbol.end()) {
      auto p = (it_order->second).mPrice;
      auto side = (it_order->second).mSide;
      auto symbol = (it_symbol->second);
      (it_order->second).mQty -= v;
      if ((it_order->second).mQty <= 0) {
        mOrders.erase(oid);
        mOrderSymbol.erase(oid);
      }
      mOrderBookContainers[symbol].update(side, oid, p, p, (it_order->second).mQty);
    }
  }

  void clear(const std::string &symbol) {
    mOrderBookContainers.erase(symbol);
  }

  std::string getSymbol(uint64_t oid) {
    auto it = mOrderSymbol.find(oid);
    return it != mOrderSymbol.end() ? it->second : "";
  }

  template<typename F>
  void visitBids(const std::string& symbol, F &&f, int N) {
    mOrderBookContainers[symbol].visitBids(f, N);
  }

  template<typename F>
  void visitAsks(const std::string &symbol, F &&f, int N) {
    mOrderBookContainers[symbol].visitAsks(f, N);
  }

private:
  std::map<uint64_t, PriceLevel::Order> mOrders; // key = oid
  std::map<uint64_t, std::string> mOrderSymbol; // use FIxedString - later
};
