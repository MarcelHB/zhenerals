#ifndef H_INI_FILE
#define H_INI_FILE

#include <cstdint>
#include <functional>
#include <istream>
#include <optional>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include "../common.h"

namespace ZH {

class INIFile {
  protected:
    std::istream& stream;

    INIFile(std::istream&);

    void advanceStream();
    void advanceStreamInLine();
    std::string consumeComment();
    std::string getToken();
    std::string getTokenInLine();

    std::unordered_map<std::string, std::string> parseAttributes();
    bool parseBool();
    std::optional<float> parseFloat();
    std::optional<uint16_t> parseInteger();
    std::optional<uint16_t> parseInteger(const std::string&);
    std::optional<int16_t> parseSignedInteger();
    std::optional<int16_t> parseSignedInteger(const std::string&);

    std::optional<std::pair<int16_t, int16_t>> parseSignedIntegerPair();
    std::optional<std::pair<uint16_t, uint16_t>> parseIntegerPair();

    std::optional<uint8_t> parsePercent();
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
  private:
    std::vector<char> readBuffer;
};

}

#endif
