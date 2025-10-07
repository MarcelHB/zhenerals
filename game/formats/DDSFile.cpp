// SPDX-License-Identifier: GPL-2.0

#include <array>

#include "../common.h"
#include "../Logging.h"
#include "DDSFile.h"

namespace ZH {

DDSFile::DDSFile(std::istream& stream) : stream(stream) {}

#define read4() \
  stream.read(reinterpret_cast<char*>(&buffer4), 4); \
  if (stream.gcount() != 4) { \
    return {}; \
  }

std::shared_ptr<GFX::HostTexture> DDSFile::getTexture() {
  TRACY(ZoneScoped);

  uint32_t buffer4;

  read4()
  if (buffer4 != 0x20534444) { // 'DDS '
    return {};
  }

  read4()
  if (buffer4 != 0x7C) {
    return {};
  }

  read4()
  uint32_t flags = buffer4;

  read4()
  uint32_t height = buffer4;

  read4()
  uint32_t width = buffer4;

  read4()
  uint32_t linearSize = buffer4;

  read4()
  uint32_t depth = buffer4;
  if (depth > 1) {
    WARN_ZH("DDSFile", "Unsupported feature: depth");
    return {};
  }

  read4()
  uint32_t mipmaps = buffer4;

  stream.seekg(44, std::ios::cur);

  struct Format {
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t RGBBitCount;
    uint32_t rBitMask;
    uint32_t gBitMask;
    uint32_t bBitMask;
    uint32_t aBitMask;
  };

  Format format;
  stream.read(reinterpret_cast<char*>(&format), sizeof(Format));
  if (stream.gcount() != sizeof(Format)) {
    return {};
  }

  if (format.size != 0x20) {
    WARN_ZH("DDSFile", "Unsupported feature: format description type");
    return {};
  }

  if (format.flags != 0x4) {
    WARN_ZH("DDSFile", "Unsupported feature: format type {}", format.flags);
    return {};
  }

  enum class Encoding {
      UNKNOWN
    , DXT1
    , DXT5
  };

  Encoding encoding = Encoding::UNKNOWN;
  if (format.fourCC == 0x31545844) { // 'DXT1'
    encoding = Encoding::DXT1;
  } else if (format.fourCC == 0x35545844) { // 'DXT5'
    encoding = Encoding::DXT5;
  } else {
    WARN_ZH("DDSFile", "Unsupported feature: DXT compression type {}", format.fourCC);
    return {};
  }

  read4()
  uint32_t caps1 = buffer4;

  read4()
  uint32_t caps2 = buffer4;

  stream.seekg(12, std::ios::cur);

  Size size {width, height};

  std::vector<char> data;
  if (encoding == Encoding::DXT1) {
    data = decodeDXT1(size);
  } else if (encoding == Encoding::DXT5) {
    data = decodeDXT5(size);
  }

  return std::make_shared<GFX::HostTexture>(
      Size {size.x, size.y}
    , ZH::GFX::HostTexture::Format::BGRA8888
    , std::move(data)
  );
}

std::array<uint8_t, 6> extractColors(const std::array<uint16_t, 2>& borderColors) {
  std::array<uint8_t, 6> colors;
  colors[0] = (borderColors[0] & 0x1F) * 8; // B
  colors[1] = ((borderColors[0] >> 5) & 0x3F) * 4; // G
  colors[2] = ((borderColors[0] >> 11) & 0x1F) * 8; // R
  colors[3] = (borderColors[1] & 0x1F) * 8;
  colors[4] = ((borderColors[1] >> 5) & 0x3F) * 4;
  colors[5] = ((borderColors[1] >> 11) & 0x1F) * 8;

  return colors;
}

std::vector<char> DDSFile::decodeDXT1(Size size) {
  std::vector<char> data;
  data.resize(size.x * size.y * 4, 0);

  std::array<uint16_t, 2> borderColors;
  std::array<uint8_t, 4> block;
  std::array<uint8_t, 6> colors;

  for (decltype(size.y) y = 0; y < std::max(size.y / 4, 1u); ++y) {
    for (decltype(size.x) x = 0; x < std::max(size.x / 4, 1u); ++x) {
      stream.read(reinterpret_cast<char*>(borderColors.data()), 4);
      if (stream.gcount() != 4) {
        return {};
      }

      stream.read(reinterpret_cast<char*>(block.data()), block.size());
      if (stream.gcount() != block.size()) {
        return {};
      }

      colors = extractColors(borderColors);
      bool col1IsGreater = borderColors[0] > borderColors[1];

      for (uint8_t i = 0; i < 16; ++i) {
        uint32_t color = 0xFF000000;
        uint8_t byte = i / 4;
        uint8_t offset = (i % 4) * 2;
        uint8_t value = (block[byte] & (0x3 << offset)) >> offset;

        if (value == 0) {
          color |= (colors[2] << 16) | (colors[1] << 8) | colors[0];
        } else if (value == 1) {
          color |= (colors[5] << 16) | (colors[4] << 8) | colors[3];
        } else if (value == 2) {
          if (col1IsGreater) {
            color |= ((colors[2] * 2 + colors[5]) / 3) << 16;
            color |= ((colors[1] * 2 + colors[4]) / 3) << 8;
            color |= ((colors[0] * 2 + colors[3]) / 3);
          } else {
            color |= ((colors[2] + colors[5]) / 2) << 16;
            color |= ((colors[1] + colors[4]) / 2) << 8;
            color |= ((colors[0] + colors[3]) / 2);
          }
        } else {
          if (col1IsGreater) {
            color |= ((colors[2] + colors[5] * 2) / 3) << 16;
            color |= ((colors[1] + colors[4] * 2) / 3) << 8;
            color |= ((colors[0] + colors[3] * 2) / 3);
          } else {
            color = 0;
          }
        }

        uint8_t row = byte;
        uint8_t column = i % 4;
        size_t destination = size.x * (y * 4 + row) + (x * 4 + column);
        *(reinterpret_cast<uint32_t*>(data.data()) + destination) = color;
      }
    }
  }

  return data;
}

std::vector<char> DDSFile::decodeDXT5(Size size) {
  std::vector<char> data;
  data.resize(size.x * size.y * 4, 0);

  std::array<uint8_t, 8> alpha;
  std::array<uint16_t, 2> borderColors;
  std::array<uint8_t, 6> colors;
  std::array<uint8_t, 4> block;
  std::array<uint8_t, 6> alphaBlock;

  for (decltype(size.y) y = 0; y < std::max(size.y / 4, 1u); ++y) {
    for (decltype(size.x) x = 0; x < std::max(size.x / 4, 1u); ++x) {
      stream.read(reinterpret_cast<char*>(alpha.data()), 2);
      if (stream.gcount() != 2) {
        return {};
      }

      stream.read(reinterpret_cast<char*>(alphaBlock.data()), 6);
      if (stream.gcount() != 6) {
        return {};
      }

      stream.read(reinterpret_cast<char*>(borderColors.data()), 4);
      if (stream.gcount() != 4) {
        return {};
      }

      stream.read(reinterpret_cast<char*>(block.data()), block.size());
      if (stream.gcount() != block.size()) {
        return {};
      }

      colors = extractColors(borderColors);
      if (alpha[0] > alpha[1]) {
        alpha[2] = (6 * alpha[0] + alpha[1]) / 7;
        alpha[3] = (5 * alpha[0] + 2 * alpha[1]) / 7;
        alpha[4] = (4 * alpha[0] + 3 * alpha[1]) / 7;
        alpha[5] = (3 * alpha[0] + 4 * alpha[1]) / 7;
        alpha[6] = (2 * alpha[0] + 5 * alpha[1]) / 7;
        alpha[7] = (alpha[0] + 6 * alpha[1]) / 7;
      } else {
        alpha[2] = (4 * alpha[0] + alpha[1]) / 5;
        alpha[3] = (3 * alpha[0] + 2 * alpha[1]) / 5;
        alpha[4] = (2 * alpha[0] + 3 * alpha[1]) / 5;
        alpha[5] = (alpha[0] + 4 * alpha[1]) / 5;
        alpha[6] = 0;
        alpha[7] = 255;
      }

      uint64_t alphaBytes = *reinterpret_cast<uint64_t*>(alphaBlock.data());
      for (uint8_t i = 0; i < 16; ++i) {
        uint32_t color = 0x00000000;
        uint8_t byte = i / 4;
        uint8_t offset = (i % 4) * 2;
        uint8_t value = (block[byte] & (0x3 << offset)) >> offset;

        if (value == 0) {
          color |= (colors[2] << 16) | (colors[1] << 8) | colors[0];
        } else if (value == 1) {
          color |= (colors[5] << 16) | (colors[4] << 8) | colors[3];
        } else if (value == 2) {
          color |= ((colors[2] * 2 + colors[5]) / 3) << 16;
          color |= ((colors[1] * 2 + colors[4]) / 3) << 8;
          color |= ((colors[0] * 2 + colors[3]) / 3);
        } else {
          color |= ((colors[2] + colors[5] * 2) / 3) << 16;
          color |= ((colors[1] + colors[4] * 2) / 3) << 8;
          color |= ((colors[0] + colors[3] * 2) / 3);
        }

        color |= alpha[((alphaBytes & (7u << (3 * i))) >> (3 * i))] << 24;

        uint8_t row = byte;
        uint8_t column = i % 4;
        size_t destination = size.x * (y * 4 + row) + (x * 4 + column);
        *(reinterpret_cast<uint32_t*>(data.data()) + destination) = color;
      }
    }
  }

  return data;
}

}
