#pragma once

#include <atomic>
#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include <cstdint>
#include <optional>
#include <vector>
#include <future>
#include <csvUtils/fastReader.h>
#include <orderHandler/Order.h>
#include <unordered_map>

using Clock = std::chrono::high_resolution_clock;

namespace tts
{
namespace Regression
{


  class OrderHandler 
  {
  public:
    OrderHandler(const std::string& file): mOrderBufferFile(file) 
    {
    
    }
    OrderHandler(const std::string& file, const long &interval): mOrderBufferFile(file),mInterval(interval) 
    {

    }
    void join()
    {
      if(mWorkerThread.joinable()) mWorkerThread.join();
    }
    bool init()
    {  
      return mOrderBufferReader.init(mOrderBufferFile, 
		      "marketId",
		      "instrumetId",
		      "orderType",
		      "price",
		      "qty",
		      "clientOid",
		      "orderId",
		      "side",
		      "tif",
		      "orderAction"
		      ); 
    } 
    void start() {
      isRunning.store(true);
      mWorkerThread = std::thread([this]() {
        std::string marketId, instrumentId, orderType;
        double price, qty;
        std::string clientOrderId, orderId, tif;
	std::string side, orderAction;	
        while (isRunning.load() && isInit.load() && mOrderBufferReader.readRow(
				marketId, instrumentId, orderType, 
				price, qty, clientOrderId, orderId, side, tif, orderAction)) 
	{
	  tts::Order order(marketId, instrumentId, orderType, price, qty, clientOrderId, orderId, side, tif, orderAction); 
          mCallBack(order);
          std::this_thread::sleep_for (std::chrono::milliseconds(mInterval));
        }
      });
    }
    template<typename CALLBACK>
    void run(CALLBACK&& cb)
    {
      isRunning.store(true);
      isInit.store(true);
      mWorkerThread = std::thread([this, moved_cb = std::move(cb)]() {
        std::string marketId, instrumentId, orderType;
        double price, qty;
        std::string clientOrderId, orderId, tif;
	std::string side, orderAction;	
        while (isRunning.load() && isInit.load() && mOrderBufferReader.readRow(
				    marketId, instrumentId, orderType,
				    price, qty, clientOrderId, orderId, side, tif, orderAction))
	{
	  tts::Order order(marketId, instrumentId, orderType, price, qty, clientOrderId, orderId, side, tif, orderAction); 
          moved_cb(order);
        }
      });
    }
    void stop() {
      isInit.store(false);
      isRunning.store(false);
    }
    void restart() {
      stop();
      start();
    }
    bool running() { 
      return isRunning.load(); 
    }
    void setFunc(std::function<void(const tts::Order&)>&& func) {
      mCallBack = std::move(func);
    }
    long getInterval() { 
      return mInterval; 
    }
    void setInterval(const long &interval) {
      mInterval = interval;
    }

    ~OrderHandler() {
      if(mWorkerThread.joinable())
      {
        mWorkerThread.join();
      }

    }
  private:
      std::function<void(const tts::Order&)> mCallBack;
      long mInterval; // in millis
      std::thread mWorkerThread;
      std::atomic<bool> isRunning = false;
      std::atomic<bool> isInit = false;
      std::string mOrderBufferFile;
      tts::csv::Reader<10> mOrderBufferReader;
  };


}
}
