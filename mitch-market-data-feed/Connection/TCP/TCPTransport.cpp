#include <TCPTransport.hpp>
#include <MDSessionConfig.hpp>
#include <SubscriptionHandler.hpp>
/*
 remove perror and ad error log -- later 
*/
namespace AAX {
  namespace TCP {

    MDSession::MDSession(const char *ip, int port) {
      SPDLOG_INFO("Instantiating TCP Session ");
      memset(&_address, 0, sizeof(_address));
      _address.sin_family = AF_INET;
      _address.sin_port = htons(port);
      _address.sin_addr.s_addr = inet_addr(ip);
      if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        SPDLOG_WARN("Could not create TCP client socket! Errno: {} {}", errno, strerror(errno));
      }
    }

    bool MDSession::connect() {
      startReceiver();
      if (::connect(tcpsock, (struct sockaddr *) &_address, sizeof(_address)) < 0) {
        SPDLOG_WARN("TCP connection failed! Errno: {} {}", errno, strerror(errno));
        return false;
      }
      SPDLOG_INFO("TCP connected!");
      sendLoginPacket();
      return true;
    }

    bool MDSession::disconnect() {
      //implement this later
      return true;
    }

    bool MDSession::send(const char *msg, size_t len) {
      if (::send(tcpsock, msg, len, 0) < 0) {
        SPDLOG_WARN("Sending TCP message failed! Errno: {} {}", errno, strerror(errno));
        return false;
      }
      return true;
    }

    MDSession::~MDSession() {
      if (worker_thread.joinable()) {
        worker_thread.join();
      }
      close(tcpsock);
    }

    void MDSession::startReceiver() {
      worker_thread = std::thread([this]() {
        uint16_t offset = 0; //for dealing with incomplete messages.
        while (!mStop.load()) {
          char msg[Config::MAX_PACKET_LENGTH];
          int receivedBytes;
          if ((receivedBytes = recv(tcpsock, &msg[offset], Config::MAX_PACKET_LENGTH - offset, 0)) < 0) {
            SPDLOG_WARN("Receiving TCP message error! Errno: {} {}", errno, strerror(errno));
            continue;
          }
          //SPDLOG_INFO("received {}, offset {}", receivedBytes, offset);
          if (receivedBytes > 0) {
            char *bufptr = &msg[0];
            uint16_t bytesLeft = receivedBytes + offset;
            while (bytesLeft >= sizeof(MITCH::Message::UnitHeader::mLength)) {
              uint16_t packetLength;
              //SPDLOG_INFO("RB:{} BL:{} PL:{} OFFSET:{}", receivedBytes, bytesLeft, packetLength, offset);
              memcpy(&packetLength, bufptr, sizeof(packetLength));

              if (bytesLeft < packetLength) {
                SPDLOG_DEBUG("Too little bytes to form a packet!");
                break;
              }

              const MITCH::Message::UnitHeader *unitHeader = reinterpret_cast<const MITCH::Message::UnitHeader *>(bufptr);
              mPacketSequence = unitHeader->mSequenceNumber;

              SPDLOG_DEBUG("Packet with {} messages received from md tcp connection !", unitHeader->mMessageCount);

              bufptr += sizeof(MITCH::Message::UnitHeader);

              for (uint8_t messageIndex = 0; messageIndex < unitHeader->mMessageCount; messageIndex++) {
                uint16_t len;
                memcpy(&len, bufptr, sizeof(len));
                mMessageCallback(bufptr, len);
                bufptr += len;
              }
              bytesLeft -= packetLength;
            }
            memcpy(&msg[0], bufptr, bytesLeft);
            offset = bytesLeft;
          }
        }
      });
    }

    void MDSession::sendLoginPacket() {
      MITCH::Message::UnitHeader loginUnitHeader;
      memset(&loginUnitHeader, 0, sizeof(MITCH::Message::UnitHeader));
      loginUnitHeader.mLength = sizeof(MITCH::Message::UnitHeader) + sizeof(MITCH::Message::LoginRequest);
      loginUnitHeader.mMessageCount = 1;
      loginUnitHeader.mMarketDataGroup = 0;
      loginUnitHeader.mSequenceNumber = 0;

      MITCH::Message::LoginRequest loginRequest;
      memset(&loginRequest, 0, sizeof(MITCH::Message::LoginRequest));
      loginRequest.mLength = sizeof(MITCH::Message::LoginRequest);
      loginRequest.mMsgType = MITCH::Message::kLoginRequest;
      memcpy(loginRequest.mUserName, AAX::Config::COMP_ID,
             sizeof(AAX::Config::COMP_ID) - 1); // excluding the tailing null
      memcpy(loginRequest.mPassword, AAX::Config::PASSWD, sizeof(AAX::Config::PASSWD) - 1);

      SPDLOG_INFO("Sending login packet!");
      sendMITCHPacket(&loginUnitHeader, &loginRequest);
    }

    void MDSession::sendRequestInstrumentsPacket() {
      AAX::MITCH::Message::UnitHeader unitHeader;
      memset(&unitHeader, 0, sizeof(MITCH::Message::UnitHeader));
      unitHeader.mLength = sizeof(MITCH::Message::UnitHeader) + sizeof(MITCH::Message::SnapshotRequest);
      unitHeader.mMessageCount = 1;
      unitHeader.mMarketDataGroup = 0;
      unitHeader.mSequenceNumber = 0;

      AAX::MITCH::Message::SnapshotRequest request;
      memset(&request, 0, sizeof(MITCH::Message::SnapshotRequest));
      request.mLength = sizeof(MITCH::Message::SnapshotRequest);
      request.mMsgType = MITCH::Message::kSnapshotRequest;
      memset(&request.mSegment, ' ', sizeof(MITCH::Message::SnapshotRequest::mSegment));
      memset(&request.mSymbol, ' ', sizeof(MITCH::Message::SnapshotRequest::mSymbol));
      request.mSubBook = 0;
      request.mSnapshotType = 2;
      memset(&request.mFromDate, 0, sizeof(MITCH::Message::SnapshotRequest::mSymbol));
      memset(&request.mFromTime, 0, sizeof(MITCH::Message::SnapshotRequest::mSymbol));
      request.mRequestId = 0;

      sendMITCHPacket(&unitHeader, &request);
      return;
    }

    void MDSession::sendOrderBookRequestPacket() {
      AAX::MITCH::Message::UnitHeader unitHeader;
      memset(&unitHeader, 0, sizeof(MITCH::Message::UnitHeader));
      unitHeader.mLength = sizeof(MITCH::Message::UnitHeader) + sizeof(MITCH::Message::SnapshotRequest);
      unitHeader.mMessageCount = 1;
      unitHeader.mMarketDataGroup = 0;
      unitHeader.mSequenceNumber = 0;

      AAX::MITCH::Message::SnapshotRequest request;
      memset(&request, 0, sizeof(MITCH::Message::SnapshotRequest));
      request.mLength = sizeof(MITCH::Message::SnapshotRequest);
      request.mMsgType = MITCH::Message::kSnapshotRequest;
      memset(&request.mSegment, ' ', sizeof(MITCH::Message::SnapshotRequest::mSegment));
      memcpy(&request.mSegment, "seg1  ", sizeof(MITCH::Message::SnapshotRequest::mSegment));
      memset(&request.mSymbol, ' ', sizeof(MITCH::Message::SnapshotRequest::mSymbol));
      request.mSubBook = 0;
      request.mSnapshotType = 0;
      auto &handler = AAX::SubscriptionHandler::instance();
      request.mSequenceNo = handler.mSnapshotRequestSequenceNo.load();
      memset(&request.mFromDate, 0, sizeof(MITCH::Message::SnapshotRequest::mFromDate));
      memset(&request.mFromTime, 0, sizeof(MITCH::Message::SnapshotRequest::mFromTime));
      request.mRequestId = 0;

      SPDLOG_INFO("Requesting orderbook snapshot for seg1!");
      sendMITCHPacket(&unitHeader, &request);
      return;
    }
  }
}
