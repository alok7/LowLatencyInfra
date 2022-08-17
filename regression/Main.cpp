#include <orderHandler/OrderHandler.h>
#include <orderHandler/Order.h>
#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

int main(int argc, char *argv[], char *envp[]){
 
  if(argc < 2){

  }
  else{
    sigset_t blockedSignal;
    sigemptyset(&blockedSignal);
    sigaddset(&blockedSignal, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);
    std::string csvFilepath(argv[1]);
    auto handler = [](const tts::Order& order)
    {
      // send the order event to downstream which needs to be regressed 	    
      std::cout << " marketId " << order.mMarketId << " instrumentId " << order.mInstrumentId << " orderType " << order.mOrderType 
	        << " price " << order.mPrice << " qty " << order.mQty << " clientOrderId " << order.mClientOrderId << " orderId " << order.mOrderId 
		<< " side " << order.mSide << '\n';
    
    };
    tts::Regression::OrderHandler dispatcher(csvFilepath);
    if(dispatcher.init())
    {
      dispatcher.run(handler);
      dispatcher.join();
    }
    else
    {
      std::cout << "order dispatcher is not initialized" << '\n';
    }	    
  }

  return 0;

}
