// SPDX-License-Identifier: GPL-2.0

#ifndef H_DICT
#define H_DICT

#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>

#include "../common.h"
#include "../InflatingStream.h"

namespace ZH {

class Dict {
  public:
    enum class DictType : uint8_t {
        BOOL
      , INT
      , FLOAT
      , STRING
      , U16STRING
      , COUNT
    };
  private:
    using DictValue = std::variant<bool, int32_t, float, std::string, std::u16string>;

    using IndexT =
      std::unordered_map<
          std::string
        , std::pair<DictType, DictValue>
      >;

    IndexT entries;
  public:
    class Iterator {
      friend Dict;

      private:
        Iterator(IndexT::const_iterator it) : it(it) {}

      public:
        Iterator& operator++();
        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator&) const;
        const std::string& key() const;
        DictType type() const;

      private:
        IndexT::const_iterator it;
    };

    size_t parse(const std::unordered_map<uint32_t, std::string>& chunkIndex, InflatingStream& stream);
    size_t size() const;

    Iterator cbegin() const;
    Iterator cend() const;
    std::optional<bool> getBool(const std::string&) const;
    std::optional<int32_t> getInt(const std::string&) const;
    std::optional<float> getFloat(const std::string&) const;
    OptionalCRef<std::string> getString(const std::string&) const;
    OptionalCRef<std::u16string> getU16String(const std::string&) const;
  private:

    size_t readBool(InflatingStream&, const std::string&);
    size_t readInt(InflatingStream&, const std::string&);
    size_t readFloat(InflatingStream&, const std::string&);
    size_t readString(InflatingStream&, const std::string&);
    size_t readU16String(InflatingStream&, const std::string&);

    template<typename T>
    std::optional<T> getT(const std::string& key) const {
      auto lookup = entries.find(key);
      if (lookup != entries.cend() && std::holds_alternative<T>(lookup->second.second)) {
        return {std::get<T>(lookup->second.second)};
      } else {
        return {};
      }
    }

    template<typename T>
    OptionalCRef<T> getCRefT(const std::string& key) const {
      auto lookup = entries.find(key);
      if (lookup != entries.cend() && std::holds_alternative<T>(lookup->second.second)) {
        return {std::cref(std::get<T>(lookup->second.second))};
      } else {
        return {};
      }
    }
};

}

#endif
