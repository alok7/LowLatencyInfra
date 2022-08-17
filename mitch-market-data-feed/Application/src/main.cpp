#include <string>
#include <memory>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <MDSessionConfig.hpp>
#include <ClientFeedHandlers.hpp>
#include <RealtimeDecoder.hpp>
#include <SnapshotDecoder.hpp>
#include <TCPTransport.hpp>
#include <UDPTransport.hpp>
#include <SubscriptionHandler.hpp>
#include <OrderBookBuilding.hpp>
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Application {
public:
  Application() {
    sigset_t blockedSignal;
    sigemptyset(&blockedSignal);
    sigaddset(&blockedSignal, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);
  }

  ~Application() {

  }

  void init() {
    std::string PRIMARY_REALTIME_HOST(AAX::Config::PRIMARY_REALTIME_HOST);
    std::string PRIMARY_REALTIME_PORT(AAX::Config::PRIMARY_REALTIME_PORT);
    std::string SECONDARY_REALTIME_HOST(AAX::Config::SECONDARY_REALTIME_HOST);
    std::string SECONDARY_REALTIME_PORT(AAX::Config::SECONDARY_REALTIME_PORT);
    std::string SNAPSHOT_HOST(AAX::Config::SNAPSHOT_HOST);
    std::string SNAPSHOT_PORT(AAX::Config::SNAPSHOT_PORT);

    mSnapshotSession = std::make_unique<AAX::TCP::MDSession>(AAX::Config::SNAPSHOT_HOST, std::stoi(SNAPSHOT_PORT));

    mPrimaryRealtimeSession = std::make_unique<AAX::UDP::MDSession>(AAX::Config::PRIMARY_REALTIME_HOST,
                                                                    std::stoi(PRIMARY_REALTIME_PORT));
    mPrimaryRealtimeSession->mRealtimeSessionType = AAX::UDP::RealtimeType::PRIMARY;

    mSecondaryRealtimeSession = std::make_unique<AAX::UDP::MDSession>(AAX::Config::SECONDARY_REALTIME_HOST,
                                                                      std::stoi(SECONDARY_REALTIME_PORT));
    mSecondaryRealtimeSession->mRealtimeSessionType = AAX::UDP::RealtimeType::SECONDARY;
    mOrderBookManager = std::make_unique<OrderBookBuilding>();

    mRealtimeDecoder.setOrderBook(mOrderBookManager.get());
    mSnapshotDecoder.setOrderBook(mOrderBookManager.get());
    // async
    handler.init(mSnapshotSession.get(), mPrimaryRealtimeSession.get(), mSecondaryRealtimeSession.get());
  }

  void start() {
    SPDLOG_INFO("Starting MITCH market data feed!");
    auto mTCPSessiondPacketsCallBack = [this](const char *buffer, size_t len) {
      mSnapshotDecoder.decode(const_cast<char *>(buffer), len);
    };


    auto pricedepthBookHandler = [this](auto event) {
      // consume event or sent to downstream
      SPDLOG_TRACE("Received price depth for: {}", event.mInstrumentName.toString());
      SPDLOG_TRACE("Bid price   Bid qty");
      for (int i = 0; i < 10; ++i) {
        SPDLOG_TRACE("{},  {}", static_cast<double>(event.mBids[i].mPrice) / 100000000,
                     static_cast<double>(event.mBids[i].mQty) / 1000000);
      }
      SPDLOG_TRACE("Ask price   Ask qty");
      for (int i = 0; i < 10; ++i) {
        SPDLOG_TRACE("{},  {}", static_cast<double>(event.mAsks[i].mPrice) / 100000000,
                     static_cast<double>(event.mAsks[i].mQty) / 1000000);
      }
    };

    auto tradeHandler = [this](auto event) {
      // consume event or sent to downstream
      SPDLOG_TRACE("Received trade for: {}", event.mInstrumentName.toString());
      SPDLOG_TRACE("price   qty");
      SPDLOG_TRACE("{},  {}", static_cast<double>(event.mPrice) / 100000000, static_cast<double>(event.mQty) / 1000000);
    };

    auto mSimultaneousMissCallback = [this](uint64_t seq, uint64_t expected) -> void {
      spdlog::get("sim_miss_logger")->info("[SIMULTANEOUS MISS] [SEQ:{}] [EXP:{}]", seq, expected);
      spdlog::get("sim_miss_logger")->flush();
    };

    auto mPrimaryUDPMissedPacketCallback = [this](uint64_t seq, uint64_t expected) -> void {
      spdlog::get("primary_udp_miss_logger")->info("[UDP PACKET MISS] [SEQ:{}] [EXP:{}]", seq, expected);
      spdlog::get("primary_udp_miss_logger")->flush();
    };

    auto mSecondaryUDPMissedPacketCallback = [this](uint64_t seq, uint64_t expected) -> void {
      spdlog::get("secondary_udp_miss_logger")->info("[UDP PACKET MISS] [SEQ:{}] [EXP:{}]", seq, expected);
      spdlog::get("secondary_udp_miss_logger")->flush();
    };

    auto mPri2SecChannelCallback = [this](uint64_t seq) -> void {
      spdlog::get("channel_swap_logger")->info("[PRI2SEC CHANNEL SWAP] [SEQ:{}]", seq);
      spdlog::get("channel_swap_logger")->flush();
    };

    auto mSec2PriChannelCallback = [this](uint64_t seq) -> void {
      spdlog::get("channel_swap_logger")->info("[SEC2PRI CHANNEL SWAP] [SEQ:{}]", seq);
      spdlog::get("channel_swap_logger")->flush();
    };

    mRealtimeDecoder.registerSimultaneousMissCallBack(mSimultaneousMissCallback);
    mPrimaryRealtimeSession->registerPacketDropCallBack(mPrimaryUDPMissedPacketCallback);
    mSecondaryRealtimeSession->registerPacketDropCallBack(mSecondaryUDPMissedPacketCallback);
    handler.registerPri2SecCallBack(mPri2SecChannelCallback);
    handler.registerSec2PriCallBack(mSec2PriChannelCallback);

    initLogging();

    // async
    mFeedHandlers.registerDepthHandler(pricedepthBookHandler);
    mFeedHandlers.registerTradeHandler(tradeHandler);
    mFeedHandlers.run();


    //async
    mPrimaryRealtimeSession->registerCallBack([this](const char *buffer, size_t len, uint64_t seq_no) {
      SPDLOG_TRACE("Buffered primary message {}.", seq_no);
      handler.mPrimaryRealtimeMessageBuffer.insertBuffer(buffer, len, seq_no);
    });
    mPrimaryRealtimeSession->registerDecoderCallBack(
            [this](char *buffer, size_t len, uint64_t seq) -> AAX::DecodeStatus {
              return mRealtimeDecoder.decode(buffer, len, seq);
            });
    mPrimaryRealtimeSession->run();

    mSecondaryRealtimeSession->registerCallBack([this](const char *buffer, size_t len, uint64_t seq_no) {
      SPDLOG_TRACE("Buffered secondary message. {}", seq_no);
      handler.mSecondaryRealtimeMessageBuffer.insertBuffer(buffer, len, seq_no);
    });
    mSecondaryRealtimeSession->registerDecoderCallBack(
            [this](char *buffer, size_t len, uint64_t seq) -> AAX::DecodeStatus {
              return mRealtimeDecoder.decode(buffer, len, seq);
            });
    mSecondaryRealtimeSession->run();

    //ensures that snapshot starts after UDP
    while (handler.mPrimaryRealtimeMessageBuffer.sizeBuffer() < AAX::Config::INITIAL_MESSAGE_BUFFER_SIZE) {
      SPDLOG_INFO("Waiting to build up initial buffer of realtime messages...");
      SPDLOG_INFO("{} messages currently...", handler.mPrimaryRealtimeMessageBuffer.sizeBuffer());
      usleep(500000);
    }
    SPDLOG_INFO("{} messages currently...", handler.mPrimaryRealtimeMessageBuffer.sizeBuffer());
    SPDLOG_INFO("First UDP message is {}", handler.getFirstPrimaryRealtimeMessageSeqNo());
    handler.mSnapshotRequestSequenceNo.store(handler.getFirstPrimaryRealtimeMessageSeqNo());
    mRealtimeDecoder.mExpectedSequence = handler.getFirstPrimaryRealtimeMessageSeqNo();

    // aysnc
    mSnapshotSession->registerCallBack(mTCPSessiondPacketsCallBack);
    mSnapshotDecoder.registerMDSession(mSnapshotSession.get());
    mSnapshotSession->connect();
  }

  void initLogging() {
    mInitLogging.store(true);
    mLoggingThread = std::thread([this]() {

      auto collectPercentageLog = [this]() -> void {
        auto d = mRealtimeDecoder.mDroppedCount.load();
        auto t = mRealtimeDecoder.mTotalCount.load();
        if (!t) {
          //avoids division by 0 when percentage is called without udp buffer processing messages yet
          return;
        }
        spdlog::get("percentage_logger")->info("[DROPPED COUNT:{}] [TOTAL COUNT:{}] [PERCENTAGE PACKET DROP RATE:{}%]",
                                               d, t, (double) d / (double) t * (double) 100);
        spdlog::get("percentage_logger")->flush();
      };

      auto collectBufferSizeLog = [this]() -> void {
        spdlog::get("buffer_size_logger")->info("[PRIMARY BUFFER SIZE:{}] [SECONDARY BUFFER SIZE:{}]",
                                                handler.mPrimaryRealtimeMessageBuffer.sizeBuffer(),
                                                handler.mSecondaryRealtimeMessageBuffer.sizeBuffer());
        spdlog::get("buffer_size_logger")->flush();
      };

      auto checkActiveLog = [this]() -> void {
        if (handler.mPrimaryCanConsume.load()) {
          spdlog::get("active_logger")->info("[PRIMARY ACTIVE]");
        }
        if (handler.mSecondaryCanConsume.load()) {
          spdlog::get("active_logger")->info("[SECONDARY ACTIVE]");
        }
        spdlog::get("active_logger")->flush();
      };

      while (mInitLogging.load()) {
        usleep(AAX::Config::COLLECT_LOGS_INTERVAL);
        collectPercentageLog();
        collectBufferSizeLog();
        checkActiveLog();
      }
    });
  }

private:
  std::thread mLoggingThread;
  std::unique_ptr<AAX::TCP::MDSession> mSnapshotSession;
  std::unique_ptr<AAX::UDP::MDSession> mPrimaryRealtimeSession;
  std::unique_ptr<AAX::UDP::MDSession> mSecondaryRealtimeSession;
  AAX::FeedHandlers::ClientFeedHandler mFeedHandlers;
  AAX::MITCH::Realtime::Decoder mRealtimeDecoder;
  AAX::MITCH::Snapshot::Decoder mSnapshotDecoder;
  std::unique_ptr<OrderBookBuilding> mOrderBookManager;
  std::atomic<bool> mOrderbookInitialised;
  std::atomic<bool> mInitLogging{true};
  AAX::SubscriptionHandler &handler = AAX::SubscriptionHandler::instance();
};

int main() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("mitch_handler", console_sink);
  logger->set_pattern("T%t %Y-%m-%d %H:%M:%S.%e [%^%l%$] %v");
  //logger->set_level(spdlog::level::info); //prefer to set as compile option so build can remove unnecessary macros
  spdlog::flush_every(std::chrono::seconds(1));
  spdlog::set_default_logger(logger);

  auto mStartTime = std::time(nullptr);
  //https://stackoverflow.com/questions/69762366/spdlog-for-c-what-failed-opening-file-logs-log-txt-for-writing-no-such
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S", localtime(&mStartTime));
  const char *home_dir = std::getenv("HOME");
  auto sim_miss_logger = spdlog::basic_logger_mt("sim_miss_logger", std::string{home_dir} + fmt::format(
          "/logs/{}/sim_miss_logger.txt", buffer));
  auto primary_udp_miss_logger = spdlog::basic_logger_mt("primary_udp_miss_logger", std::string{home_dir} +
                                                                                    fmt::format(
                                                                                            "/logs/{}/primary_udp_miss_logger.txt",
                                                                                            buffer));
  auto secondary_udp_miss_logger = spdlog::basic_logger_mt("secondary_udp_miss_logger", std::string{home_dir} +
                                                                                        fmt::format(
                                                                                                "/logs/{}/secondary_udp_miss_logger.txt",
                                                                                                buffer));
  auto channel_swap_logger = spdlog::basic_logger_mt("channel_swap_logger", std::string{home_dir} + fmt::format(
          "/logs/{}/channel_swap_logger.txt", buffer));
  auto percentage_logger = spdlog::basic_logger_mt("percentage_logger", std::string{home_dir} + fmt::format(
          "/logs/{}/percentage_logger.txt", buffer));
  auto buffer_size_logger = spdlog::basic_logger_mt("buffer_size_logger", std::string{home_dir} + fmt::format(
          "/logs/{}/buffer_size_logger.txt", buffer));
  auto active_logger = spdlog::basic_logger_mt("active_logger", std::string{home_dir} +
                                                                fmt::format(
                                                                        "/logs/{}/active_logger.txt",
                                                                        buffer));
  Application app;

#ifdef NDEBUG
  SPDLOG_INFO("This binary was built in RELEASE mode!");
  spdlog::set_level(spdlog::level::info);
#else
  SPDLOG_WARN("This binary was built in DEBUG mode!");
  spdlog::set_level(
          spdlog::level::trace);  //SPDLOG_ACTIVE_LEVEL define only gets rid of macros under set logging level, but does not set logging level of default logger.
  // https://github.com/gabime/spdlog/issues/1269
  logger->set_pattern("T%t %Y-%m-%d %H:%M:%S.%e [%^%l%$] %v %@");
#endif
  SPDLOG_INFO("SPDLOG_ACTIVE_LEVEL at {}", SPDLOG_ACTIVE_LEVEL);


  app.init();
  app.start();

  while (1) {
  }
  return 0;
}
