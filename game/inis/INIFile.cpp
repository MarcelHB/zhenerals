#include "../Logging.h"
#include "INIFile.h"

namespace ZH {

INIFile::INIFile(std::istream& instream) : stream(instream) {
  readBuffer.reserve(128);
}

void INIFile::advanceStream() {
  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r') {
      stream.get();
    } else {
      break;
    }
  } while (!stream.eof());
}

void INIFile::advanceStreamInLine() {
  do {
    auto peek = stream.peek();
    if (peek == ' ') {
      stream.get();
    } else {
      break;
    }
  } while (!stream.eof());
}

std::string INIFile::consumeComment() {
  advanceStream();
  auto token = getToken();

  while (!stream.eof() && !token.empty() && (token[0] == ';' || token[0] == '/')) {
    while (!stream.eof()) {
      auto c = stream.get();
      if (c == '\n') {
        break;
      }
    }
    advanceStream();
    token = getToken();
  }

  return token;
}

std::string INIFile::getToken() {
  readBuffer.clear();

  // get first whatever it is
  auto c = stream.get();
  readBuffer.push_back(c);

  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r' || peek == ';' || peek == '/') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
    }
  } while (!stream.eof());

  return {readBuffer.cbegin(), readBuffer.cend()};
}

std::string INIFile::getTokenInLine() {
  readBuffer.clear();

  do {
    auto peek = stream.peek();
    if (peek == ' ' || peek == '\n' || peek == '\r' || peek == ';' || peek == '/') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
      // stuff being glued, like '=55'
      if (c == '=') {
        break;
      }
    }
  } while (!stream.eof());

  return {readBuffer.cbegin(), readBuffer.cend()};
}

std::unordered_map<std::string, std::string> INIFile::parseAttributes() {
  std::unordered_map<std::string, std::string> attributes;

  advanceStreamInLine();
  auto token = getTokenInLine();
  if (token != "=") {
    return attributes;
  }

  advanceStreamInLine();
  token = getTokenInLine();
  while (!token.empty()) {
    auto splitPos = token.find(":");
    if (splitPos == std::string::npos) {
      return attributes;
    }

    auto key = token.substr(0, splitPos);
    auto value = token.substr(splitPos + 1);

    // whitespace after colon
    while (!key.empty() && value.empty()) {
      advanceStreamInLine();
      value = getTokenInLine();
    }

    attributes.emplace(std::move(key), std::move(value));

    advanceStreamInLine();
    token = getTokenInLine();
  }

  return attributes;
}

bool INIFile::parseBool() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseBool(token);
}

bool INIFile::parseBool(const std::string& token) const {
  if (token == "yes" || token == "Yes" || token == "YES") {
    return true;
  } else if (token == "no" || token == "No" || token == "NO") {
    return false;
  }

  WARN_ZH("INIFile", "Unknown boolean {}, returning false.", token);
  return false;
}

std::optional<float> INIFile::parseFloat() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseFloat(token);
}

std::optional<float> INIFile::parseFloat(const std::string& value) const {
  try {
    return {stof(value)};
  } catch (std::invalid_argument) {
    return {};
  } catch (std::out_of_range) {
    return {};
  }
}

std::optional<uint8_t> INIFile::parseByte() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  auto value = parseInteger(token);
  if (value > std::numeric_limits<uint8_t>::max()) {
    return {};
  }

  return value;
}

std::optional<int8_t> INIFile::parseSignedByte() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  auto value = parseInteger(token);
  if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max()) {
    return {};
  }

  return value;
}

std::array<float, 3> INIFile::parseCoord3D() {
  std::array<float, 3> coords;

  auto values = parseAttributes();
  if (values.contains("X")) {
    coords[0] = parseFloat(values["X"]).value_or(0.0f);
  }
  if (values.contains("Y")) {
    coords[1] = parseFloat(values["Y"]).value_or(0.0f);
  }
  if (values.contains("Z")) {
    coords[2] = parseFloat(values["Z"]).value_or(0.0f);
  }

  return coords;
}

std::optional<uint16_t> INIFile::parseShort() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseShort(token);
}

std::optional<uint16_t> INIFile::parseShort(const std::string& token) const {
  auto value = parseInteger(token);
  if (!value || *value > std::numeric_limits<uint16_t>::max()) {
    return {};
  }

  return value;
}

std::optional<int16_t> INIFile::parseSignedShort() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseSignedShort(token);
}

std::optional<int16_t> INIFile::parseSignedShort(const std::string& token) const {
  auto value = parseSignedInteger(token);
  if (!value || (value < std::numeric_limits<int16_t>::min() && value > std::numeric_limits<int16_t>::max())) {
    return {};
  }

  return value;
}

std::optional<uint32_t> INIFile::parseInteger() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseInteger(token);
}

std::optional<uint32_t> INIFile::parseInteger(const std::string& s) const {
  try {
    return {std::stoul(s.c_str(), nullptr)};
  } catch (std::invalid_argument) {
    return {};
  } catch (std::out_of_range) {
    return {};
  }
}

std::optional<int32_t> INIFile::parseSignedInteger() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  return parseSignedInteger(token);
}

std::optional<int32_t> INIFile::parseSignedInteger(const std::string& s) const {
  try {
    return {std::stol(s.c_str(), nullptr)};
  } catch (std::invalid_argument) {
    return {};
  } catch (std::out_of_range) {
    return {};
  }
}

std::optional<std::pair<int16_t, int16_t>> INIFile::parseSignedShortPair() {
  std::pair<int16_t, int16_t> pair;
  auto values = parseStringList();
  if (values.size() != 2) {
    return {};
  }

  auto opt = parseSignedShort(values[0]);
  if (!opt) {
    return {};
  }
  pair.first = *opt;

  opt = parseSignedShort(values[1]);
  if (!opt) {
    return {};
  }
  pair.second = *opt;

  return std::make_optional(std::move(pair));
}

std::optional<std::pair<uint16_t, uint16_t>> INIFile::parseShortPair() {
  std::pair<uint16_t, uint16_t> pair;
  auto values = parseStringList();
  if (values.size() != 2) {
    return {};
  }

  auto opt = parseShort(values[0]);
  if (!opt) {
    return {};
  }
  pair.first = *opt;

  opt = parseShort(values[1]);
  if (!opt) {
    return {};
  }
  pair.second = *opt;

  return std::make_optional(std::move(pair));
}

std::optional<int8_t> INIFile::parsePercent() {
  auto value = parseSignedShort();
  if (!value) {
    WARN_ZH("INIFile", "Invalid percent value: {}", value);
    return {};
  } else if (value > 100) {
    return {100};
  } else if (value < -100) {
    return {-100};
  }

  return value;
}

Color INIFile::parseRGB() {
  auto color = parseRGBA();
  color.a = 0xFF;

  return color;
}

Color INIFile::parseRGBA() {
  Color color;

  auto values = parseAttributes();
  if (values.contains("R")) {
    color.r = std::min(parseInteger(values["R"]).value_or(0), 255u);
  }
  if (values.contains("G")) {
    color.g = std::min(parseInteger(values["G"]).value_or(0), 255u);
  }
  if (values.contains("B")) {
    color.b = std::min(parseInteger(values["B"]).value_or(0), 255u);
  }
  if (values.contains("A")) {
    color.a = std::min(parseInteger(values["A"]).value_or(255), 255u);
  }

  return color;
}

std::string INIFile::parseString() {
  advanceStream();

  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  return getTokenInLine();
}

// If we encounter missing `=` between key and value
std::string INIFile::parseLooseValue() {
  advanceStream();

  auto token = getTokenInLine();
  if (token != "=") {
    return token;
  }

  advanceStream();
  return getTokenInLine();
}

std::vector<std::string> INIFile::parseStringList() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  std::vector<std::string> values;

  advanceStreamInLine();
  token = getTokenInLine();
  while (!token.empty()) {
    values.emplace_back(std::move(token));
    advanceStreamInLine();
    token = getTokenInLine();
  }

  return values;
}

bool INIFile::parseEmptyAttributeBlock() {
  advanceStream();
  auto token = consumeComment();

  if (token == "End") {
    return true;
  } else {
    WARN_ZH("INIFile", "Empty block actually not empty.");
    return false;
  }
}

}

