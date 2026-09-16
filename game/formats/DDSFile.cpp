// SPDX-License-Identifier: GPL-2.0

#include <array>

#include "common.h"
#include "Logging.h"
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
  uint32_t numMipMaps = buffer4;

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

  auto encoding = Encoding::UNKNOWN;
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
  std::vector<unsigned char> data;
  size_t rawSize = calculateTotalSize(size, numMipMaps, encoding);

  data.resize(rawSize);
  stream.read(reinterpret_cast<char*>(data.data()), data.size());

  return std::make_shared<GFX::HostTexture>(
      size
    , (encoding == Encoding::DXT5)
        ? ZH::GFX::HostTexture::Format::DXT5
        : ZH::GFX::HostTexture::Format::DXT1
    , std::move(data)
    , numMipMaps
  );
}

size_t DDSFile::calculateTotalSize(const Size& size, uint32_t numMipMaps, Encoding encoding) {
  size_t totalSize = 0;

  for (uint32_t i = 0; i < numMipMaps; ++i) {
    auto width = std::max(1u, size.x >> i);
    auto height = std::max(1u, size.y >> i);

    auto numBlocks = ((width + 3) / 4) * ((height + 3) / 4);

    totalSize += numBlocks * (encoding == Encoding::DXT5 ? 16 : 8);
  }

  return totalSize;
}

}
