#include <cstdint>

#include "InflatingStream.h"

static constexpr size_t REFPACK_CHUNK_SIZE = 65536;

namespace ZH {

InflatingStream::InflatingStream(std::istream& stream) : stream(stream) {
  uint32_t buffer4 = 0;
  stream.read(reinterpret_cast<char*>(&buffer4), 4);
  if (stream.gcount() != 4) {
    broken = true;
    return;
  }

  if (buffer4 == 0x524145) { // EAR
    compressionType = CompressionType::REFPACK;
  }

  stream.read(reinterpret_cast<char*>(&buffer4), 4);
  if (stream.gcount() != 4) {
    broken = true;
    return;
  }

  inflatedSize = buffer4;
  advanceByCompressionHeader();
}

bool InflatingStream::eof() const {
  if (compressionType == CompressionType::REFPACK) {
    return readPos == inflationBuffer.size();
  } else {
    return stream.eof();
  }
}

uint32_t InflatingStream::getInflatedSize() const {
  return inflatedSize;
}

void InflatingStream::seekg(size_t pos, std::ios_base::seekdir direction) {
  if (direction == std::ios::cur) {
    size_t result = 0;
    if (__builtin_add_overflow(readPos, pos, &result)) {
      readPos = inflationBuffer.size();
    } else {
      readPos = std::min(inflationBuffer.size(), result);
    }
  } else {
    readPos = std::min(inflationBuffer.size(), pos);
  }
}

void InflatingStream::advanceByCompressionHeader() {
  if (compressionType == CompressionType::REFPACK) {
    uint16_t buffer2 = 0;
    stream.read(reinterpret_cast<char*>(&buffer2), 2);
    if (stream.gcount() != 2) {
      broken = true;
      return;
    }

    // Big endian
    buffer2 = (buffer2 & 0xFF) << 8 | (buffer2 & 0xFF00) >> 8;

    // Skipping (un)compressed size info fields
    uint8_t seek = 3;
    if (buffer2 & 0x8000) {
      seek = 4;
    }
    if (buffer2 & 0x100) {
      seek *= 2;
    }

    stream.seekg(seek, std::ios::cur);

    if (stream.eof()) {
      broken = true;
    }
  }
}

uint64_t InflatingStream::read(char* buffer, uint64_t numBytes) {
  if (broken) {
    return 0;
  }

  if (compressionType == CompressionType::REFPACK) {
    return readRefPack(buffer, numBytes);
  } else {
    stream.read(buffer, numBytes);

    return stream.gcount();
  }
}

uint64_t InflatingStream::readRefPack(char* buffer, uint64_t numBytes) {
  if (inflationBuffer.empty()) {
    inflationBuffer.resize(inflatedSize);
    decodeRefPack();
  }

  if (readPos == inflationBuffer.size()) {
    return 0;
  }

  auto bytesToRead = std::min(numBytes, inflationBuffer.size() - readPos);
  std::copy(
      inflationBuffer.begin() + readPos
    , inflationBuffer.begin() + readPos + bytesToRead
    , buffer
  );
  readPos += bytesToRead;

  return bytesToRead;
}

#define read1() \
  stream.read(reinterpret_cast<char*>(&buffer1), 1); \
  if (stream.gcount() != 1) { \
    broken = true; \
    return fillSize; \
  }

// Maybe it can be streamed in some way, but it there are big blocks,
// asking for at least 128kib lookback, so let's keep this for now
uint32_t InflatingStream::decodeRefPack() {
  bool stop = false;
  size_t fillSize = 0;
  uint8_t buffer1 = 0, one = 0, two = 0, three = 0, four = 0;

  while (true) {
    read1()
    one = buffer1;

    uint32_t readSize = 0, copySize = 0, copyOffset = 0;

    if (!(one & 0x80)) {
      read1()
      two = buffer1;

      readSize = one & 3;
      copySize = ((one & 0x1C) >> 2) + 3;
      copyOffset = (((one & 0x60) << 3) | two) + 1;
    } else if (!(one & 0x40)) {
      read1()
      two = buffer1;
      read1()
      three = buffer1;

      readSize = two >> 6;
      copySize = (one & 0x3F) + 4;
      copyOffset = (((two & 0x3F) << 8) | three) + 1;
    } else if (!(one & 0x20)) {
      read1()
      two = buffer1;
      read1()
      three = buffer1;
      read1()
      four = buffer1;

      readSize = one & 3;
      copySize = (((one & 0xC) << 6) + four) + 5;
      copyOffset = (((((one & 0x10) << 4) | two) << 8) | three) + 1;
    } else {
      readSize = ((one & 0x1F) + 1) * 4;
      if (readSize > 112) {
        readSize = one & 0x3;
        stop = true;
      }
    }

    stream.read(reinterpret_cast<char*>(inflationBuffer.data() + fillSize), readSize);
    if (stream.gcount() != readSize) {
      return fillSize;
    }
    fillSize += readSize;

    if (copySize > 0
        && copyOffset <= fillSize
        && (fillSize - copyOffset + copySize) <= fillSize + copySize
    ) {
      std::copy(
          inflationBuffer.begin() + fillSize - copyOffset
        , inflationBuffer.begin() + fillSize - copyOffset + copySize
        , inflationBuffer.begin() + fillSize
      );
      fillSize += copySize;
    }

    if (stop) {
      return fillSize;
    }
  }
}

}
