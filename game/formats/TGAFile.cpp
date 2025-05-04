#include <array>

#include "../common.h"
#include "../Logging.h"
#include "TGAFile.h"

namespace ZH {

TGAFile::TGAFile(std::istream& stream) : stream(stream) {}

std::optional<Size> TGAFile::getSize() {
  TRACY(ZoneScoped);

  if (!headerParsed && !parseHeader()) {
    return {};
  }

  return {size};
}

std::shared_ptr<GFX::HostTexture> TGAFile::getTexture() {
  TRACY(ZoneScoped);

  if (!headerParsed && !parseHeader()) {
    return {};
  }

  uint8_t bytesPerPx = bitsPerPixel / 8;
  std::vector<char> data;
  data.resize(4 * size.x * size.y);

  if (bitsPerPixel == 32) {
    for (auto y = 0; y < size.y; ++y) {
      stream.read(data.data() + (size.y - 1 - y) * size.x * 4, size.x * 4);
      if (stream.gcount() != size.x * 4) {
        return {};
      }
    }
  } else {
    std::array<char, 3> buffer;
    for (auto y = 0; y < size.y; ++y) {
      for (auto x = 0; x < size.x; ++x) {
        stream.read(buffer.data(), 3);
        if (stream.gcount() != 3) {
          return {};
        }

        auto it = data.data() + ((size.y - 1 - y) * size.x * 4) + x * 4;
        *(it) = buffer[0];
        *(it + 1) = buffer[1];
        *(it + 2) = buffer[2];
        *(it + 3) = 0xFF;
      }
    }
  }

  return std::make_shared<GFX::HostTexture>(
      Size {size.x, size.y}
    , ZH::GFX::HostTexture::Format::BGRA8888
    , std::move(data)
  );
}

bool TGAFile::parseHeader() {
  std::array<char, 3> fields;
  stream.read(fields.data(), 3);
  if (stream.gcount() != 3) {
    return false;
  }

  // Support validation
  if (fields[0] != 0 || fields[1] != 0 || fields[2] != 2) {
    WARN_ZH("TGA", "Unsupported header field values.");
    return false;
  }

  stream.seekg(2, std::ios::cur);

  uint16_t colorMapSize = 0;
  stream.read(reinterpret_cast<char*>(&colorMapSize), 2);
  if (stream.gcount() != 2) {
    return false;
  }

  if (colorMapSize != 0) {
    WARN_ZH("TGA", "Unsupported color map value.");
    return false;
  }

  stream.seekg(5, std::ios::cur);

  size.x = 0;
  stream.read(reinterpret_cast<char*>(&size.x), 2);
  if (stream.gcount() != 2) {
    return false;
  }

  size.y = 0;
  stream.read(reinterpret_cast<char*>(&size.y), 2);
  if (stream.gcount() != 2) {
    return false;
  }

  stream.read(reinterpret_cast<char*>(&bitsPerPixel), 1);
  if (stream.gcount() != 1) {
    return false;
  }

  if (bitsPerPixel != 32 && bitsPerPixel != 24) {
    WARN_ZH("TGA", "Unsupported bitsPerPx value.");
    return false;
  }

  // Ignore whatever weird order bits until there is something
  stream.seekg(1, std::ios::cur);
  headerParsed = true;

  return true;
}

}
