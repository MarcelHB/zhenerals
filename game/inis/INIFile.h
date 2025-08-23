#ifndef H_INI_FILE
#define H_INI_FILE

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
    std::optional<float> parseFloat();

    std::optional<uint8_t> parseByte();
    std::optional<int8_t> parseSignedByte();
    // EVAL parse generic list
    std::optional<uint16_t> parseShort(bool following = false);
    std::optional<int16_t> parseSignedShort(bool following = false);

    std::optional<uint32_t> parseInteger();
    std::optional<uint32_t> parseInteger(const std::string&);
    std::optional<int32_t> parseSignedInteger();
    std::optional<int32_t> parseSignedInteger(const std::string&);

    std::optional<std::pair<int16_t, int16_t>> parseSignedShortPair();
    std::optional<std::pair<uint16_t, uint16_t>> parseShortPair();

    std::optional<uint8_t> parsePercent();
    Color parseRGB();
    Color parseRGBA();
    std::string parseString();
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
        std::string_view valueView = value;

        if (value[0] == '+' || value[0] == '-') {
          valueView = std::string_view(value.begin() + 1, value.end());
        }

        auto valueOpt = getter(valueView);
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
        WARN_ZH("INIFile", "Unsupported field: {}", key);
        return false;
      }

      it->second(obj, *this);

      return true;
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

    bool parseEmptyAttributeBlock();
  private:
    std::vector<char> readBuffer;
};

}

#endif
