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
#include <string>
#include <netdb.h>
#include <MitchSchema.hpp>
#include <spdlog/spdlog.h>
#include <MDSessionConfig.hpp>

namespace AAX {
  namespace TCP {

    class MDSession {
    public:
      MDSession(const char *ip, int port);

      ~MDSession();

      template<typename MessageCallback>
      void registerCallBack(MessageCallback &&f) {
        mMessageCallback = f;
      }

      bool connect();

      bool disconnect();

      bool send(const char *msg, size_t len);

      void sendLoginPacket();

      void sendRequestInstrumentsPacket();

      void sendOrderBookRequestPacket();

      void stop() {
        mStop.store(true);
      }

    public:
      uint64_t mPacketSequence;
    private:
      void startReceiver();

      template<typename ... MsgPtrs>
      void sendMITCHPacket(const MITCH::Message::UnitHeader *unitHeader, MsgPtrs &&... msgptrs) {
        SPDLOG_DEBUG("Sent packet to server.");
        char buffer[AAX::Config::MAX_PACKET_LENGTH];
        auto bufptr = buffer;
        memset(bufptr, 0, AAX::Config::MAX_PACKET_LENGTH);
        memcpy(bufptr, reinterpret_cast<const char *>(unitHeader), sizeof(MITCH::Message::UnitHeader));
        bufptr += sizeof(MITCH::Message::UnitHeader);
        //lambda applied to fold expression (for-looping the msgptrs basically)
        ([&](auto &mptr) {
          memcpy(bufptr, reinterpret_cast<const char *>(mptr), sizeof(*mptr));
          bufptr += sizeof(*mptr);
          //buf.Append(reinterpret_cast<const char *>(mptr), mptr->mLength);
        }(msgptrs), ...);
        send(buffer, unitHeader->mLength);
      }

    private:
      int tcpsock;
      struct sockaddr_in _address;
      std::thread worker_thread;
      std::atomic<bool> mStop = {false};
      std::function<void(const char *, size_t)> mMessageCallback;
    };

  }
}
