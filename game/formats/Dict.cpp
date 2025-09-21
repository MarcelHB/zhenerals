#include <vector>

#include "Dict.h"

namespace ZH {

size_t Dict::parse(
    const std::unordered_map<uint32_t, std::string>& chunkIndex
  , InflatingStream& stream
) {
  uint16_t numEntries = 0;

  size_t readSize = stream.read(reinterpret_cast<char*>(&numEntries), 2);
  size_t totalBytes = readSize;

  if (readSize != 2) {
    return totalBytes;
  }

  for (uint16_t i = 0; i < numEntries; ++i) {
    if (stream.eof()) {
      break;
    }

    int32_t keyType = 0;
    readSize = stream.read(reinterpret_cast<char*>(&keyType), 4);
    totalBytes += readSize;

    if (readSize != 4) {
      return totalBytes;
    }

    uint8_t t = keyType & 0xFF;
    if (t >= static_cast<std::underlying_type_t<DictType>>(DictType::COUNT)) {
      return totalBytes;
    }

    std::string keyName;
    auto nameLookup = chunkIndex.find(keyType >> 8);
    if (nameLookup != chunkIndex.cend()) {
      keyName = nameLookup->second;
    }

    switch (static_cast<DictType>(t)) {
      case DictType::BOOL:
        totalBytes += readBool(stream, keyName);
        break;
      case DictType::INT:
        totalBytes += readInt(stream, keyName);
        break;
      case DictType::FLOAT:
        totalBytes += readFloat(stream, keyName);
        break;
      case DictType::STRING:
        totalBytes += readString(stream, keyName);
        break;
      case DictType::U16STRING:
        totalBytes += readU16String(stream, keyName);
        break;
      default: break; // COUNT, handled above
    }
  }

  return totalBytes;
}

size_t Dict::size() const {
  return entries.size();
}

std::optional<bool> Dict::getBool(const std::string& key) const {
  return getT<bool>(key);
}

std::optional<int32_t> Dict::getInt(const std::string& key) const {
  return getT<int32_t>(key);
}

std::optional<float> Dict::getFloat(const std::string& key) const {
  return getT<float>(key);
}

OptionalCRef<std::string> Dict::getString(const std::string& key) const {
  return getCRefT<std::string>(key);
}

OptionalCRef<std::u16string> Dict::getU16String(const std::string& key) const {
  return getCRefT<std::u16string>(key);
}

size_t Dict::readBool(InflatingStream& stream, const std::string& key) {
  uint8_t rawValue = 0;
  if (stream.read(reinterpret_cast<char*>(&rawValue), 1) != 1) {
    return 0;
  }

  if (!key.empty()) {
    DictValue value {rawValue > 0};
    entries.emplace(key, std::move(value));
  }

  return 1;
}

size_t Dict::readInt(InflatingStream& stream, const std::string& key) {
  int32_t rawValue = 0;
  auto bytesRead = stream.read(reinterpret_cast<char*>(&rawValue), 4);
  if (bytesRead != 4) {
    return bytesRead;
  }

  if (!key.empty()) {
    DictValue value {rawValue};
    entries.emplace(key, std::move(value));
  }

  return bytesRead;
}

size_t Dict::readFloat(InflatingStream& stream, const std::string& key) {
  float rawValue = 0;
  auto bytesRead = stream.read(reinterpret_cast<char*>(&rawValue), 4);
  if (bytesRead != 4) {
    return bytesRead;
  }

  if (!key.empty()) {
    DictValue value {rawValue};
    entries.emplace(key, std::move(value));
  }

  return bytesRead;
}

size_t Dict::readString(InflatingStream& stream, const std::string& key) {
  uint16_t len = 0;
  auto bytesRead = stream.read(reinterpret_cast<char*>(&len), 2);
  if (bytesRead != 2) {
    return bytesRead;
  }

  std::vector<char> buffer;
  buffer.resize(len);
  bytesRead = stream.read(buffer.data(), len);
  if (bytesRead != len) {
    return bytesRead + 2;
  }

  if (!key.empty()) {
    DictValue value {std::string{buffer.data(), len}};
    entries.emplace(key, std::move(value));
  }

  return bytesRead + 2;
}

size_t Dict::readU16String(InflatingStream& stream, const std::string& key) {
  uint16_t len = 0;
  auto bytesRead = stream.read(reinterpret_cast<char*>(&len), 2);
  if (bytesRead != 2) {
    return bytesRead;
  }

  std::vector<char16_t> buffer;
  buffer.resize(len);
  bytesRead = stream.read(reinterpret_cast<char*>(buffer.data()), len * 2);
  if (bytesRead != len * 2) {
    return bytesRead + 2;
  }

  if (!key.empty()) {
    DictValue value {std::u16string{buffer.data(), len}};
    entries.emplace(key, std::move(value));
  }

  return bytesRead + 2;
}

}
