#pragma once

#include <atomic>
#include <thread>
#include <functional>
#include <MarketEvents.hpp>
#include <EventProcessingQueue.hpp>

namespace AAX {
  namespace FeedHandlers {
    class ClientFeedHandler {
    public:
      ClientFeedHandler() {

      }

      ~ClientFeedHandler() {
        init.store(false);
        if (m_thread.joinable()) {
          m_thread.join();
        }
      }

      template<typename HandlerTrade>
      void registerTradeHandler(HandlerTrade handler) {
        trade_callback = handler;
      }

      template<typename HandlerDepth>
      void registerDepthHandler(HandlerDepth handler) {
        depth_callback = handler;
      }

      void run() {

        init.store(true);
        m_thread = std::thread([this]() {

          MarketDataEventQueue<AAX::Message::Trade> &trade_event_queue = MarketDataEventQueue<AAX::Message::Trade>::Instance();
          MarketDataEventQueue<AAX::Message::PriceDepthBook> &depth_event_queue = MarketDataEventQueue<AAX::Message::PriceDepthBook>::Instance();

          while (init.load()) {

            auto trade_event = trade_event_queue.pop();
            if (trade_event.second && trade_callback) {
              trade_callback(std::move(trade_event.first));
            }

            auto depth_event = depth_event_queue.pop();
            if (depth_event.second && depth_callback) {
              depth_callback(std::move(depth_event.first));
            }

          }
        });

      }

      //! Sets internal thread to run only on `cpuset`. Returns 0 if successful
      int set_cpu_affinity(cpu_set_t cpuset) {
        int rc = pthread_setaffinity_np(m_thread.native_handle(), sizeof(cpu_set_t), &cpuset);
        return rc;
      }

    private:
      std::function<void(AAX::Message::PriceDepthBook)> depth_callback;
      std::function<void(AAX::Message::Trade)> trade_callback;

      std::thread m_thread;
      std::atomic<bool> init = {false};
    };

  }
}
