#include <array>

#include "../common.h"
#include "../Logging.h"
#include "TGAFile.h"

namespace ZH {

TGAFile::TGAFile(std::istream& stream) : stream(stream) {}

std::shared_ptr<GFX::HostTexture> TGAFile::getTexture() {
  TRACY(ZoneScoped);

  std::array<char, 3> fields;
  stream.read(fields.data(), 3);
  if (stream.gcount() != 3) {
    return {};
  }

  // Support validation
  if (fields[0] != 0 || fields[1] != 0 || fields[2] != 2) {
    WARN_ZH("TGA", "Unsupported header field values.");
    return {};
  }

  stream.seekg(2, std::ios::cur);

  uint16_t colorMapSize = 0;
  stream.read(reinterpret_cast<char*>(&colorMapSize), 2);
  if (stream.gcount() != 2) {
    return {};
  }

  if (colorMapSize != 0) {
    WARN_ZH("TGA", "Unsupported color map value.");
    return {};
  }

  stream.seekg(5, std::ios::cur);

  uint32_t width = 0;
  stream.read(reinterpret_cast<char*>(&width), 2);
  if (stream.gcount() != 2) {
    return {};
  }

  uint32_t height = 0;
  stream.read(reinterpret_cast<char*>(&height), 2);
  if (stream.gcount() != 2) {
    return {};
  }

  uint8_t bitsPerPx = 0;
  stream.read(reinterpret_cast<char*>(&bitsPerPx), 1);
  if (stream.gcount() != 1) {
    return {};
  }

  if (bitsPerPx != 32 && bitsPerPx != 24) {
    WARN_ZH("TGA", "Unsupported bitsPerPx value.");
    return {};
  }

  // Ignore whatever weird order bits until there is something
  stream.seekg(1, std::ios::cur);

  uint8_t bytesPerPx = bitsPerPx / 8;
  std::vector<char> data;
  data.resize(4 * width * height);

  if (bitsPerPx == 32) {
    for (auto y = 0; y < height; ++y) {
      stream.read(data.data() + (height - 1 - y) * width * 4, width * 4);
      if (stream.gcount() != width * 4) {
        return {};
      }
    }
  } else {
    std::array<char, 3> buffer;
    for (auto y = 0; y < height; ++y) {
      for (auto x = 0; x < width; ++x) {
        stream.read(buffer.data(), 3);
        if (stream.gcount() != 3) {
          return {};
        }

        auto it = data.data() + ((height - 1 - y) * width * 4) + x * 4;
        *(it) = buffer[0];
        *(it + 1) = buffer[1];
        *(it + 2) = buffer[2];
        *(it + 3) = 0xFF;
      }
    }
  }

  return std::make_shared<GFX::HostTexture>(
      Size {width, height}
    , ZH::GFX::HostTexture::Format::BGRA8888
    , std::move(data)
  );
}

}
