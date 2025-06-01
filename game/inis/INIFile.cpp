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

  while (!stream.eof() && !token.empty() && token[0] == ';') {
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
    if (peek == ' ' || peek == '\n' || peek == '\r') {
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
    if (peek == ' ' || peek == '\n' || peek == '\r' || peek == ';') {
      break;
    } else {
      auto c = stream.get();
      readBuffer.push_back(c);
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
  if (token == "yes" || token == "Yes") {
    return true;
  } else if (token == "no" || token == "No") {
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

  return stof(token);
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

std::optional<uint16_t> INIFile::parseShort(bool following) {
  if (!following) {
    advanceStream();
    auto token = getTokenInLine();
    if (token != "=") {
      return {};
    }
  }

  advanceStream();
  auto token = getTokenInLine();

  auto value = parseInteger(token);
  if (value > std::numeric_limits<uint16_t>::max()) {
    return {};
  }

  return value;
}

std::optional<int16_t> INIFile::parseSignedShort(bool following) {
  if (!following) {
    advanceStream();
    auto token = getTokenInLine();
    if (token != "=") {
      return {};
    }
  }

  advanceStream();
  auto token = getTokenInLine();

  auto value = parseSignedInteger(token);
  if (value < std::numeric_limits<int16_t>::min() && value > std::numeric_limits<int16_t>::max()) {
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

std::optional<uint32_t> INIFile::parseInteger(const std::string& s) {
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

std::optional<int32_t> INIFile::parseSignedInteger(const std::string& s) {
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
  auto valueOpt = parseSignedShort();
  if (!valueOpt) {
    return {};
  }
  pair.first = *valueOpt;

  valueOpt = parseSignedShort(true);
  if (!valueOpt) {
    return {};
  }
  pair.second = *valueOpt;

  return std::make_optional(std::move(pair));
}

std::optional<std::pair<uint16_t, uint16_t>> INIFile::parseShortPair() {
  std::pair<uint16_t, uint16_t> pair;
  auto valueOpt = parseShort();
  if (!valueOpt) {
    return {};
  }
  pair.first = *valueOpt;

  valueOpt = parseShort(true);
  if (!valueOpt) {
    return {};
  }
  pair.second = *valueOpt;

  return std::make_optional(std::move(pair));
}

std::optional<uint8_t> INIFile::parsePercent() {
  auto value = parseShort();
  if (!value || value > 100) {
    WARN_ZH("INIFile", "Invalid percent value.");
    return {};
  }

  return value;
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

