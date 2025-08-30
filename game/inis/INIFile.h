#ifndef H_INI_FILE
#define H_INI_FILE

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include "../Color.h"
#include "../common.h"
#include "../Logging.h"

namespace ZH {

class INIFile;

template<typename T>
using INIApplier = std::function<bool(T&, INIFile&)>;

template<typename T>
using INIApplierMap = std::map<std::string, INIApplier<T>>;

class INIFile {
  protected:
    std::istream& stream;

    INIFile(std::istream&);
    void advanceStream();
    void advanceStreamInLine();
    std::string consumeComment();
    std::string getToken();
    std::string getTokenInLine();

  public:
    std::unordered_map<std::string, std::string> parseAttributes();
    bool parseBool();
    bool parseBool(const std::string&) const;
    std::optional<float> parseFloat();
    std::optional<float> parseFloat(const std::string&) const;

    std::optional<uint8_t> parseByte();
    std::optional<int8_t> parseSignedByte();

    std::array<float, 3> parseCoord3D();

    // EVAL parse generic list
    std::optional<uint16_t> parseShort();
    std::optional<uint16_t> parseShort(const std::string&) const;
    std::optional<int16_t> parseSignedShort();
    std::optional<int16_t> parseSignedShort(const std::string&) const;

    std::optional<uint32_t> parseInteger();
    std::optional<uint32_t> parseInteger(const std::string&) const;
    std::optional<int32_t> parseSignedInteger();
    std::optional<int32_t> parseSignedInteger(const std::string&) const;

    std::optional<std::pair<int16_t, int16_t>> parseSignedShortPair();
    std::optional<std::pair<uint16_t, uint16_t>> parseShortPair();

    std::optional<int8_t> parsePercent();
    Color parseRGB();
    Color parseRGBA();
    std::string parseString();
    std::string parseLooseValue();
    std::vector<std::string> parseStringList();

    template<typename T>
    BitField<T> parseEnumBitField(std::function<T(const std::string&)> getter) {
      auto values = parseStringList();
      if (values.empty()) {
        return {};
      }

      BitField<T> field;
      for (auto& value : values) {
        auto tValue = getter(value);
        field |= tValue;
      }

      return field;
    }

    template<typename T>
    std::optional<T> parseEnum(std::function<std::optional<T>(const std::string_view&)> getter) {
      advanceStream();
      auto token = getTokenInLine();
      if (token != "=") {
        return {};
      }

      advanceStream();
      token = getTokenInLine();

      std::transform(token.cbegin(), token.cend(), token.begin(), [](char c) { return std::toupper(c); });
      return getter(token);
    }

    template<typename T>
    bool parseEnumSet(std::set<T>& set, std::function<std::optional<T>(const std::string_view&)> getter) {
      auto values = parseStringList();
      if (values.empty()) {
        return false;
      }

      auto firstValue = values[0];
      if (firstValue == "ALL") {
        for (size_t i = 1; i < static_cast<std::underlying_type<T>::type>(T::ALL); ++i) {
          set.insert(static_cast<T>(i));
        }
      }

      if (values.size() == 1) {
        return true;
      }

      size_t startIndex = (firstValue == "ALL" || firstValue == "NONE") ? 1 : 0;

      for (size_t i = startIndex; i < values.size(); ++i) {
        auto& value = values[i];
        std::string lookupToken;

        if (value[0] == '+' || value[0] == '-') {
          lookupToken = value.substr(1);
        } else {
          lookupToken = value;
        }

        std::transform(
            lookupToken.cbegin()
          , lookupToken.cend()
          , lookupToken.begin()
          , [](char c) { return std::toupper(c); }
        );

        auto valueOpt = getter(lookupToken);
        if (valueOpt) {
          if (value[0] == '-') {
            auto lookup = set.find(*valueOpt);
            if (lookup != set.cend()) {
              set.erase(lookup);
            }
          } else {
            set.insert(*valueOpt);
          }
        } else {
          WARN_ZH("INIFile", "Unsupported enum value: {}", value);
        }
      }

      return true;
    };

    template <typename T>
    bool applyValueByKey(const INIApplierMap<T>& map, T& obj, const std::string& key) {
      auto it = map.find(key);
      if (it == map.cend()) {
        it = map.find("*");
        if (it != map.cend()) {
          return it->second(obj, *this);
        } else {
          WARN_ZH("INIFile", "Unsupported field: {}", key);
          return false;
        }
      }

      return it->second(obj, *this);
    }

    template <typename T>
    bool applyValueByKeyOfMaps(T& obj, const std::string& key) {
      return false;
    }

    template <typename T, typename Map, typename ... Maps>
    bool applyValueByKeyOfMaps(T& obj, const std::string& key, const Map& map, Maps ... maps) {
      auto it = map.find(key);

      if (it != map.cend()) {
        return it->second(obj, *this);
      } else {
        if (applyValueByKeyOfMaps(obj, key, maps...)) {
          return true;
        }
      }

      it = map.find("*");
      if (it != map.cend()) {
        return it->second(obj, *this);
      }

      return false;
    }

    template<typename B>
    bool parseAttributes(B& b, const INIApplierMap<B>& map) {
      advanceStream();
      auto token = consumeComment();

      while (token != "End" && !stream.eof()) {
        if (!applyValueByKey(map, b, token)) {
          WARN_ZH("INIFile", "Error while parsing: {}", token);
          return false;
        }

        token = consumeComment();
      }

      return true;
    }

    template<typename B, typename ... Maps>
    bool parseAttributesForMaps(B& b, Maps... maps) {
      advanceStream();
      auto token = consumeComment();

      while (token != "End" && !stream.eof()) {
        if (!applyValueByKeyOfMaps(b, token, maps...)) {
          WARN_ZH("INIFile", "Error while parsing: {}", token);
          return false;
        }

        token = consumeComment();
      }

      return true;
    }

    template<typename T>
    bool parseAttributeBlock(T& builder, const INIApplierMap<T>& map) {
      return parseAttributes(builder, map);
    }

    template<typename T, typename B>
    bool parseSubtypedAttributeBlock(std::shared_ptr<B>&& pointer, const INIApplierMap<T>& map) {
      pointer = std::make_shared<T>();
      auto data = static_pointer_cast<T>(pointer);
      return parseAttributeBlock(*data, map);
    }

    template<typename T, typename B, typename ... Maps>
    bool parseSubtypedAttributeBlocks(std::shared_ptr<B>&& pointer, Maps... maps) {
      pointer = std::make_shared<T>();
      auto data = static_pointer_cast<T>(pointer);
      return parseAttributesForMaps(*data, maps...);
    }

    bool parseEmptyAttributeBlock();
  private:
    std::vector<char> readBuffer;
};

}

#endif
