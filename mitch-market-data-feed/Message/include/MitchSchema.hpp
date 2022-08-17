#include <limits>

#pragma once

namespace AAX {
  enum DecodeStatus {
    DUPLICATE_PACKET,
    PACKET_GAP,
    EXPECTED_PACKET
  };
  namespace MITCH {
    namespace Message {
      enum MsgType : unsigned char {
        kLoginRequest = 0x01,
        kLoginResponse = 0x02,
        kLogoutRequest = 0x05,
        kReplayRequest = 0x03,
        kReplayResponse = 0x04,
        kSnapshotRequest = 0x81,
        kSnapshotResponse = 0x82,
        kSnapshotComplete = 0x83,
        kTime = 0x54,
        kSystemEvent = 0x53,
        kSymbolDirectory = 0x52,
        kSymbolStatus = 0x48,
        kAddOrder = 0x41,
        kAddAttributedOrder = 0x46,
        kOrderDeleted = 0x44,
        kOrderModified = 0x55,
        kOrderBookClear = 0x79,
        kOrderExecuted = 0x45,
        kOrderExecutedWithPriceSize = 0x43,
        kTrade = 0x50,
        kAuctionTrade = 0x51,
        kOffBookTrade = 0x78,
        kRecoveryTrade = 0x76,
        kAuctionInfo = 0x49,
        kStatistics = 0x77,
        kExtendedStatistics = 0x80,
        kNews = 0x75,
        kUpdatePricePoint = 0x73,
        kDeletePricePoint = 0x72,
        kTopOfBook = 0x71,
        kIndicativeQuoteInfo = 0x69
      };

      //sorry.
      constexpr const char *kMsgTypeHexToString[0xFF + 1]{
              "InvalidMsgType",
              "kLoginRequest",
              "kLoginResponse",
              "kReplayRequest",
              "kReplayResponse",
              "kLogoutRequest",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "kAddOrder",
              "InvalidMsgType",
              "kOrderExecutedWithPriceSize",
              "kOrderDeleted",
              "kOrderExecuted",
              "kAddAttributedOrder",
              "InvalidMsgType",
              "kSymbolStatus",
              "kAuctionInfo",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "kTrade",
              "kAuctionTrade",
              "kSymbolDirectory",
              "kSystemEvent",
              "kTime",
              "kOrderModified",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "kIndicativeQuoteInfo",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "kTopOfBook",
              "kDeletePricePoint",
              "kUpdatePricePoint",
              "InvalidMsgType",
              "kNews",
              "kRecoveryTrade",
              "kStatistics",
              "kOffBookTrade",
              "kOrderBookClear",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "kExtendedStatistics",
              "kSnapshotRequest",
              "kSnapshotResponse",
              "kSnapshotComplete",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType",
              "InvalidMsgType"
      };

#pragma pack(push, 1)
      struct UnitHeader {
        uint16_t mLength;
        uint8_t mMessageCount;
        unsigned char mMarketDataGroup;
        uint64_t mSequenceNumber;
      };

      struct MessageHeader {
        uint16_t mLength;
        MsgType mMsgType;
      };

      struct LoginRequest {
        uint16_t mLength;
        MsgType mMsgType;
        char mUserName[6]; //mCompId
        char mPassword[10];
      };

      struct SnapshotRequest {
        uint16_t mLength;
        MsgType mMsgType;
        uint64_t mSequenceNo;
        char mSegment[6];
        char mSymbol[13];
        uint8_t mSubBook;
        uint8_t mSnapshotType;
        char mFromDate[8];
        char mFromTime[8];
        uint32_t mRequestId;
      };

      struct LogoutRequest {
        uint16_t mLength;
        MsgType mMsgType;
      };

      struct Heartbeat {
        uint16_t mLength;
        uint8_t mMsgCount;
        unsigned char mMarketDataGroup;
        uint64_t mSequenceNo;
        char *mPayload;
      };

      struct LoginResponse {
        uint16_t mLength;
        MsgType mMsgType;
        char mStatus;
      };

      struct SnapshotResponse {
        uint16_t mLength;
        MsgType mMsgType;
        uint64_t mSequenceNo;
        uint32_t mOrderCount;
        char mStatus;
        uint8_t mSnapshotType;
        uint32_t mRequestId;
      };

      struct SnapshotComplete {
        uint16_t mLength;
        MsgType mMsgType;
        uint64_t mSequenceNo;
        char mSegment[6];
        char mSymbol[13];
        int8_t mSubBook;
        char mTradingStatus;
        uint8_t mSnapshotType;
        uint32_t mRequestId;
      };
      struct SystemEvent {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        char mEventCode;
      };
      struct SymbolDirectory {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        char mSymbol[13];
        char mSymbolStatus;
        char mIdentificationNo[12];
        char mSegment[6];
        char mExpirationDate[8];
        char mUnderlying[13];
        int32_t mStikePrice;
        char mOptionType;
        char mIssuer[6];
        char mIssueDate[8];
        int32_t mCoupon;
        char mFlag;
        int8_t mSubBook;
        char mCorporateAction[5];
        char mPartionId;
        char mExerciseStyle;
      };

      struct SymbolStatus {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        char mSymbol[13];
        char mTradingStatus;
        int8_t mFlag;
        char mHaltReason[4];
        uint8_t mSessionChangeReason;
        char mNewEndTime[12];
        int8_t mSubBook;
      };
      struct OrderBookClear {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        char mSymbol[13];
        uint8_t mSubBook;
        char mBookType;
      };

      struct DeleteOrder {
        //
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
      };

      struct AddOrder {
        //
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
        char mSide;
        int64_t mQuantity;
        //test
        char mSymbol[13];
        int64_t mPrice;
        uint8_t mFlags;
        char mRFQID[10];
      };
      struct AddAttributedOrder {
        //
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
        char mSide;
        int64_t mQuantity;
        //test
        char mSymbol[13];
        int64_t mPrice;
        char mAttribution[6];
        uint8_t mFlags;
      };
      struct ModifyOrder {
        //
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
        int64_t mQuantity;
        int64_t mPrice;
        uint8_t mFlags;
      };

      struct ExecuteOrder {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
        int64_t mQuantity;
        uint64_t mTradeId;
        int64_t mLastOptPx;
        int64_t mVolatility;
        int64_t mUnderlyingRef;
        char mTradingDateTime[27];
        char mIIC[12];
        uint8_t mIICType;
        int64_t mPrice;
        char mCurrency[3];
        char mVenue[4];
        uint8_t mPriceNotation;
        int64_t mNotionalAmount;
        char mNotionalCurrency[3];
        char mPublicationDateTime[27];
      };
      struct ExecuteOrderPriceSize {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        uint64_t mOrderID;
        int64_t mQuantity;
        int64_t mDisplayQuantity;
        uint64_t mTradeId;
        char mPrintable;
        int64_t mPrice;
        int64_t mLastOptPx;
        int64_t mVolatility;
        int64_t mUnderlyingRef;
        char mTradingDateTime[27];
        uint8_t mIICType;
        char mIIC[12];
        char mCurrency[3];
        char mVenue[4];
        uint8_t mPriceNotation;
        int64_t mNotionalAmount;
        char mNotionalCurrency[3];
        char mPublicationDateTime[27];
      };
      struct ClearBook {
        //
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanosecond;
        char mSymbol[13];
        uint8_t mSubook;
        char mBooktype;
      };
      struct Trade {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        int64_t mExecutedQty;
        char mSymbol[13];
        int64_t mPrice;
        uint64_t mTradeId;
        int8_t mSubBook;
        int64_t mLastOptPx;
        int64_t mVolatility;
        int64_t mUnderlyingRefPrice;
        int8_t mFlag;
        char mTradingDateTime[27];
        uint8_t mISINCodeType;
        char mISINCode[12];
        char mCurrency[3];
        char mExecutionVenue[4];
        uint8_t mPriceNotation;
        int64_t mNotionalAmount;
        char mNotionalCurrency[3];
        char mPubDateTime[27];
        char PTCancellationFlag[4];
        char PTAmendmentFlag[4];
      };
/*
strcut AuctionTrade
{

};
struct OffBookTrade
{

};
struct RecoverTrade
{

};
struct ExtendedStatistics
{

};

**/
      struct PriceDepthL1 {
        uint16_t mLength;
        MsgType mMsgType;
        uint32_t mNanos;
        char mSymbol[6];
        int8_t mSubBook;
        char mAction;
        char mSide;
        int64_t mPrice;
        int64_t mQty;
        int64_t mMktOrderQty;
        char mReserved[2];

      };

#pragma pack(pop)


    }
  }
}
