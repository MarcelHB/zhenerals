#ifndef H_DICT
#define H_DICT

#include <optional>
#include <unordered_map>
#include <variant>

#include "../common.h"
#include "../InflatingStream.h"

namespace ZH {

class Dict {
  public:
    size_t parse(const std::unordered_map<uint32_t, std::string>& chunkIndex, InflatingStream& stream);
    size_t size() const;

    std::optional<bool> getBool(const std::string&) const;
    std::optional<int32_t> getInt(const std::string&) const;
    std::optional<float> getFloat(const std::string&) const;
    OptionalCRef<std::string> getString(const std::string&) const;
    OptionalCRef<std::u16string> getU16String(const std::string&) const;
  private:
    using DictValue = std::variant<bool, int32_t, float, std::string, std::u16string>;
    enum class DictType : uint8_t {
        BOOL
      , INT
      , FLOAT
      , STRING
      , U16STRING
      , COUNT
    };

    std::unordered_map<std::string, DictValue> entries;

    size_t readBool(InflatingStream&, const std::string&);
    size_t readInt(InflatingStream&, const std::string&);
    size_t readFloat(InflatingStream&, const std::string&);
    size_t readString(InflatingStream&, const std::string&);
    size_t readU16String(InflatingStream&, const std::string&);

    template<typename T>
    std::optional<T> getT(const std::string& key) const {
      auto lookup = entries.find(key);
      if (lookup != entries.cend() && std::holds_alternative<T>(lookup->second)) {
        return {std::get<T>(lookup->second)};
      } else {
        return {};
      }
    }

    template<typename T>
    OptionalCRef<T> getCRefT(const std::string& key) const {
      auto lookup = entries.find(key);
      if (lookup != entries.cend() && std::holds_alternative<T>(lookup->second)) {
        return {std::cref(std::get<T>(lookup->second))};
      } else {
        return {};
      }
    }
};

}

#endif
