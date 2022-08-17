#pragma once

namespace tts
{

struct Order
{

  Order(const std::string& marketId, const std::string& instrumentId, const std::string& orderType,
                              double price, double qty, const std::string& clientOrderId, const std::string& orderId,
                              const std::string& side, const std::string& tif, const std::string& actionType)
  {
    
    mMarketId = marketId;
    mInstrumentId = instrumentId;
    mOrderType = orderType;
    mPrice = price;
    mQty = qty;
    mClientOrderId = clientOrderId;
    mOrderId = orderId;
    mSide = side;
    mTIF = tif;
    mActionType = actionType;
  
  }	  
  std::string mMarketId;
  std::string mInstrumentId;
  std::string mOrderType;
  double mPrice;
  double mQty;
  std::string mClientOrderId;
  std::string mOrderId;
  std::string mSide;
  std::string mTIF;
  std::string mActionType;

};

}
