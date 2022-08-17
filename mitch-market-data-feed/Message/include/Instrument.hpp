#pragma once

#include <cstdint>
#include <string>
#include <stdio.h>
#include <string.h>

namespace AAX {
  namespace Message {

    struct InstrumentId {
      using type = int64_t;
      type mId;

      InstrumentId() : mId(-1) {
      }

      InstrumentId(type id) : mId(id) {

      }

      InstrumentId &operator=(const InstrumentId &id) {
        mId = id.get();
        return *this;
      }

      InstrumentId &operator=(InstrumentId &&id) {
        mId = std::move(id.get());
        return *this;
      }

      InstrumentId(const InstrumentId &id) {
        mId = id.get();
      }

      InstrumentId(InstrumentId &&id) {
        mId = std::move(id.get());
      }

      bool operator==(const InstrumentId id) {
        return mId == id.get();
      }

      type &get() {
        return mId;
      }

      const type &get() const {
        return mId;
      }

    };

    struct InstrumentName {
      char mName[13];

      InstrumentName() {
        memset(mName, '\0', sizeof(mName));
      }

      InstrumentName(const std::string &name) {
        memcpy(mName, name.data(), std::min(name.length(), sizeof(mName)));
      }

      InstrumentName(const char *name) {
        memcpy(mName, name, std::min(strlen(name), sizeof(mName)));
      }

      InstrumentName(const InstrumentName &name) {
        memcpy(mName, name.data(), sizeof(mName));
      }

      InstrumentName &operator=(const InstrumentName &name) {
        if (this != &name) {
          memcpy(mName, name.data(), sizeof(mName));
        }
        return *this;
      }

      bool operator==(const InstrumentName &name) const {
        return memcpy(const_cast<char *>(mName), name.data(), sizeof(mName)) == 0;
      }

      const char *data() {
        return mName;
      }

      const char *data() const {
        return mName;
      }

      std::string toString() {
        return std::string(mName, sizeof(mName));
      }

    };

    inline bool operator==(const std::string &left, const InstrumentName &right) {
      return memcpy(const_cast<char *>(left.data()), right.data(), sizeof(right.mName)) == 0;
    }

    inline bool operator==(const InstrumentName &left, const std::string &right) {
      return memcpy(const_cast<char *>(left.data()), right.data(), sizeof(left.mName)) == 0;
    }

    struct InstrumentIdHash {
      std::size_t operator()(const InstrumentId &instrument) const noexcept {
        auto hashfn = std::hash<int>{};
        return hashfn(instrument.get());
      }

    };

    struct InstrumentNameHash {
      std::size_t operator()(const InstrumentName &instrument) const noexcept {
        auto hashfn = std::hash<std::string>{};
        return hashfn(std::string(instrument.mName, sizeof(instrument.mName)));
      }

    };

  }
}
