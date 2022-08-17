#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <functional>
#include <spdlog/spdlog.h>
#include <MitchSchema.hpp>
#include <MDSessionConfig.hpp>

namespace AAX {
  namespace UDP {
    enum RealtimeType {
      PRIMARY,
      SECONDARY
    };

    class MDSession {
    public:
      MDSession(const char *group, int port);

      ~MDSession();

      void run();

      template<typename MessageCallback>
      void registerCallBack(MessageCallback &&f) {
        mMessageCallback = f;
      }

      void stop() {
        mStop.store(true);
      }

      //TO-DO, find some way just dierctly callign decode from subscription handler.
      AAX::DecodeStatus decode(char *buffer, size_t len, uint64_t seq) {
        return mDecodeCallback(buffer, len, seq);
      }

      template<typename PacketDropCallBack>
      void registerPacketDropCallBack(PacketDropCallBack &&f) {
        mPacketDropCallback = f;
      }

      template<typename DecodeCallBack>
      void registerDecoderCallBack(DecodeCallBack &&f) {
        mDecodeCallback = f;
      }

    public:
      uint64_t mPacketSequence{0};
      RealtimeType mRealtimeSessionType;
    private:
      int udpsock;
      struct sockaddr_in _address;
      std::thread worker_thread;
      std::atomic<bool> mStop = {false};
      std::function<AAX::DecodeStatus(char *, size_t, uint64_t)> mDecodeCallback;
      std::function<void(char *, size_t, uint64_t)> mMessageCallback;
      std::function<void(uint64_t, uint64_t)> mPacketDropCallback;
      static const size_t MAX_PACKET_LENGTH = AAX::Config::MAX_PACKET_LENGTH;
    };

  }
}
