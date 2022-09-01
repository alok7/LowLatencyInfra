#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <atomic>
#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include <cstdint>
#include <optional>
#include <csvUtils/fastReader.h>

struct Event
{
  Event(const std::string& marketId, const std::string& instrumentId, const std::string& orderType, double price, double qty,
	 const std::string& clientOrderId, const std::string& orderId, const std::string& side, const std::string& tif, const std::string& orderAction)
       :mMarketId(marketId),mInstrumentId(instrumentId),mOrderType(orderType),mPrice(price),mQty(qty),mClientOrderId(clientOrderId),
	  mOrderId(mOrderId),mTIF(tif),mSide(side),mOrderAction(orderAction) 
  {}
  std::string mMarketId;
  std::string mInstrumentId;
  std::string mOrderType; 
  double mPrice;
  double mQty;
  std::string mClientOrderId; 
  std::string mOrderId;
  std::string mTIF;
  std::string mSide;
  std::string mOrderAction; 
};

class EventHandler
{
  public:
    EventHandler(const std::string& file): mEventBufferFile(file)
    {
    }
    void join()
    {
      if(mWorkerThread.joinable()) mWorkerThread.join();
    }
    bool init()
    {
      return mEventBufferReader.init(mEventBufferFile,
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
    template<typename CALLBACK>
    void run(CALLBACK&& cb)
    {
      isRunning.store(true);
      mWorkerThread = std::thread([this, moved_cb = std::move(cb)]() {
        std::string marketId, instrumentId, orderType;
        double price, qty;
        std::string clientOrderId, orderId, tif;
	std::string side, orderAction;
        while (isRunning.load() && mEventBufferReader.readRow(
				    marketId, instrumentId, orderType,
				    price, qty, clientOrderId, orderId, side, tif, orderAction))
	{
	  Event event(marketId, instrumentId, orderType, price, qty, clientOrderId, orderId, side, tif, orderAction);
          moved_cb(event);
        }
      });
    }
    void stop() {
      isRunning.store(false);
    }
    ~EventHandler() {
      if(mWorkerThread.joinable())
      {
        mWorkerThread.join();
      }

    }
  private:
      std::thread mWorkerThread;
      std::atomic<bool> isRunning = false;
      std::string mEventBufferFile;
      csv::Reader<10> mEventBufferReader;
  };

int main(int argc, char *argv[], char *envp[]){
 
  if(argc < 2){

  }
  else{
    sigset_t blockedSignal;
    sigemptyset(&blockedSignal);
    sigaddset(&blockedSignal, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);
    std::string csvFilepath(argv[1]);
    auto handler = [](const Event& order)
    {
      std::cout << " marketId " << order.mMarketId << " instrumentId " << order.mInstrumentId << " orderType " << order.mOrderType 
	        << " price " << order.mPrice << " qty " << order.mQty << " clientOrderId " << order.mClientOrderId << " orderId " << order.mOrderId 
		<< " side " << order.mSide << '\n';
    
    };
    EventHandler dispatcher(csvFilepath);
    if(dispatcher.init())
    {
      dispatcher.run(handler);
      dispatcher.join();
    }
    else
    {
      std::cout << "Event dispatcher is not initialized" << '\n';
    }	    
  }

  return 0;

}
