#include <UDPTransport.hpp>

namespace AAX {
  namespace UDP {

    MDSession::MDSession(const char *group, int port) {
      SPDLOG_INFO("Starting UDP session with group {} and port {}!", group, port);
      memset(&_address, 0, sizeof(_address));
      _address.sin_family = AF_INET;
      //_address.sin_addr.s_addr = inet_addr(group);
      _address.sin_addr.s_addr = htonl(INADDR_ANY);
      _address.sin_port = htons(port);
      if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        SPDLOG_WARN("Could not create UDP client socket! Errno: {} {}", errno, strerror(errno));
      }
      int reuse = 1; // multiple sockets to use the same port
      if (setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) {
        SPDLOG_WARN("Reusing address failed! Errno: {} {}", errno, strerror(errno));
      }
      // binding to receive
      if (bind(udpsock, (struct sockaddr *) &_address, sizeof(_address)) < 0)//check UDP socket is bind correctly
      {
        SPDLOG_WARN("Cannot bind to receive address! Errno: {} {}", errno, strerror(errno));
      }

      // request that the kernel join a multicast group
      struct ip_mreq mreq;
      mreq.imr_multiaddr.s_addr = inet_addr(group);
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);
      if (setsockopt(udpsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq)) < 0) {
        SPDLOG_WARN("Multicast group joining failed! Errno: {} {}", errno, strerror(errno));
      }
      uint64_t sockopt = 0;
      socklen_t sockoptlen = sizeof(sockopt);
      if (getsockopt(udpsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt, &sockoptlen) >= 0) {
        SPDLOG_INFO("UDP socket buffer size: {} bytes", sockopt);
        if (sockopt < 16777216) {
          SPDLOG_WARN("Small size of UDP buffer might lead to dropped packets. Consider increasing buffer size.");
        }
      } else {
        SPDLOG_WARN("Error getting socket size!");
      }
    }

    MDSession::~MDSession() {
      mStop.store(true);
      if (worker_thread.joinable()) {
        worker_thread.join();
      }
      close(udpsock);
    }

    void MDSession::run() {
      worker_thread = std::thread([this]() {
        while (!mStop.load()) {
          char msg[MAX_PACKET_LENGTH];
          int receivedBytes;
          if ((receivedBytes = recvfrom(udpsock, msg, MAX_PACKET_LENGTH, 0, (struct sockaddr *) NULL, NULL)) == -1) {
            SPDLOG_WARN("Error in receiving UDP packets! Errno: {} {}", errno, strerror(errno));
          } else if (receivedBytes >= 0) {
            char *buffer = &msg[0];
            uint16_t packetLength;
            memcpy(&packetLength, buffer, sizeof(packetLength));
            if (packetLength != receivedBytes) {
              SPDLOG_WARN("Malformed packet dropped!");
              break;
            }
            uint16_t bytesRemaining = packetLength;
            const MITCH::Message::UnitHeader *unitHeader = reinterpret_cast<const MITCH::Message::UnitHeader *>(buffer);

            //TEST DROPPING SECTION
            //SIMULATES SIMULTANEOUS PACKET DROPS
            /*
            if (!(mPacketSequence % 10000)) {
              SPDLOG_WARN("Caused packet drop at {} server, {}",
                          (mRealtimeSessionType == RealtimeType::PRIMARY ? "PRIMARY" : "SECONDARY"),
                          mPacketSequence);
                          continue;
            }
            */

            //SIMULATES UNCORRELATED PACKET DROPS ON PRIMARY AND SECONDARY
            /*
            int r = rand() & 0b111111111111;
            if (!r) {
              SPDLOG_WARN("Caused packet drop at {} server, {}",
                          (mRealtimeSessionType == RealtimeType::PRIMARY ? "PRIMARY" : "SECONDARY"),
                          mPacketSequence);
              continue;
            }
            */

            //END TEST DROPPING SECTION

            if (unitHeader->mSequenceNumber != mPacketSequence ) {
              mPacketDropCallback(unitHeader->mSequenceNumber, mPacketSequence);
            };
            mPacketSequence = unitHeader->mSequenceNumber;
            SPDLOG_TRACE("Packet with {} messages received from {} realtime server!", unitHeader->mMessageCount,
                         (mRealtimeSessionType == RealtimeType::PRIMARY ? "PRIMARY" : "SECONDARY"));
            buffer += sizeof(MITCH::Message::UnitHeader);
            //bytesRemaining -= sizeof(MITCH::Message::UnitHeader);
            if (unitHeader->mMessageCount == 0) {
              SPDLOG_DEBUG("Received heartbeat from server.");
            }
            for (uint8_t messageIndex = 0; messageIndex < unitHeader->mMessageCount; messageIndex++) {
              uint16_t len;
              memcpy(&len, buffer, sizeof(len));
              if (bytesRemaining < len) {
                SPDLOG_WARN("Too little bytes for UDP message!");
                return;
              }
              mMessageCallback(buffer, len, mPacketSequence);

              ++mPacketSequence;
              buffer += len;
              bytesRemaining -= len;
            }
          }
        }
      });
    }

  }
}
