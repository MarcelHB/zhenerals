#include <algorithm>
#include <cstdint>
#include <vector>

#include "common.h"
#include "CSFFile.h"

namespace ZH {

CSFFile::CSFFile(std::istream& stream) : stream(stream) {}

#define read4() \
  stream.read(reinterpret_cast<char*>(&buffer4), 4); \
  if (stream.gcount() != 4) { \
    return map; \
  }

CSFFile::StringMap CSFFile::getStrings() {
  TRACY(ZoneScoped);

  StringMap map;

  std::vector<char> readBuffer;
  readBuffer.reserve(1024);
  uint32_t buffer4;

  read4()
  if (buffer4 != 0x43534620) {
    return map;
  }

  stream.seekg(20, std::ios::cur);

  StringEntry entry;

  read4()
  while (buffer4 == 0x4C424C20) { // 'LBL '
    read4()
    uint32_t numStrings = buffer4;
    read4()
    uint32_t labelLength = buffer4;

    readBuffer.resize(labelLength);
    stream.read(readBuffer.data(), labelLength);
    if (stream.gcount() != labelLength) {
      return map;
    }

    readBuffer[labelLength] = 0;
    std::string label{readBuffer.data()};

    for (uint32_t i = 0; i < numStrings; ++i) {
      read4()

      bool hasSoundFile = buffer4 == 0x53545257; // 'STRW'
      if (!hasSoundFile && buffer4 != 0x53545220) { // 'STR '
        return map;
      }

      read4()
      uint32_t strLength = buffer4 * 2;
      readBuffer.resize(strLength);
      stream.read(readBuffer.data(), strLength);
      if (stream.gcount() != strLength) {
        return map;
      }

      if (i == 0) {
        std::transform(readBuffer.begin(), readBuffer.begin() + strLength, readBuffer.begin(), [](char c) {
          return ~c;
        });
        std::u16string string{reinterpret_cast<char16_t*>(readBuffer.data()), buffer4};
        entry.string = std::move(string);
        // EVAL: stripping spaces?
      }

      if (hasSoundFile) {
        read4()
        readBuffer.resize(buffer4);
        stream.read(readBuffer.data(), buffer4);
        if (stream.gcount() != buffer4) {
          return map;
        }

        if (i == 0) {
          entry.soundFile = {readBuffer.data(), readBuffer.size()};
        }
      }

      if (i == 0) {
        map.emplace(std::move(label), std::move(entry));
      }
    }

    read4()
  }

  return map;
}

}
