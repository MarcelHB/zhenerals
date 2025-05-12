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
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();
  if (token == "yes") {
    return true;
  } else if (token == "no") {
    return false;
  }

  WARN_ZH("INIFile", "Unknown boolean {}, returning false.", token);
  return false;
}

std::optional<float> INIFile::parseFloat() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();

  return stof(token);
}

std::optional<uint16_t> INIFile::parseInteger() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();

  return parseInteger(token);
}

std::optional<uint16_t> INIFile::parseInteger(const std::string& s) {
  try {
    return {std::stoul(s.c_str(), nullptr)};
  } catch (std::invalid_argument) {
    return {};
  } catch (std::out_of_range) {
    return {};
  }
}

std::optional<int16_t> INIFile::parseSignedInteger() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();

  return parseSignedInteger(token);
}

std::optional<int16_t> INIFile::parseSignedInteger(const std::string& s) {
  try {
    return {std::stol(s.c_str(), nullptr)};
  } catch (std::invalid_argument) {
    return {};
  } catch (std::out_of_range) {
    return {};
  }
}

std::optional<std::pair<int16_t, int16_t>> INIFile::parseSignedIntegerPair() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();
  std::pair<int16_t, int16_t> pair;
  auto valueOpt = parseSignedInteger(token);
  if (!valueOpt) {
    return {};
  }
  pair.first = *valueOpt;

  advanceStream();
  token = getToken();
  valueOpt = parseSignedInteger(token);
  if (!valueOpt) {
    return {};
  }
  pair.second = *valueOpt;

  return std::make_optional(std::move(pair));
}

std::optional<std::pair<uint16_t, uint16_t>> INIFile::parseIntegerPair() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();
  std::pair<uint16_t, uint16_t> pair;
  auto valueOpt = parseInteger(token);
  if (!valueOpt) {
    return {};
  }
  pair.first = *valueOpt;

  advanceStream();
  token = getToken();
  valueOpt = parseInteger(token);
  if (!valueOpt) {
    return {};
  }
  pair.second = *valueOpt;

  return std::make_optional(std::move(pair));
}

std::optional<uint8_t> INIFile::parsePercent() {
  advanceStream();
  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getToken();

  auto value = parseInteger(token);
  if (value && *value > 100) {
    value = {100};
  }

  return value;
}

std::string INIFile::parseString() {
  advanceStream();

  auto token = getToken();
  if (token != "=") {
    return {};
  }

  advanceStream();
  return getTokenInLine();
}

std::vector<std::string> INIFile::parseStringList() {
  advanceStream();
  auto token = getToken();
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

}

