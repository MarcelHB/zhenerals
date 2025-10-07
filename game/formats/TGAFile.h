// SPDX-License-Identifier: GPL-2.0

#ifndef H_TGA_FILE
#define H_TGA_FILE

#include <istream>
#include <memory>
#include <optional>

#include "../common.h"
#include "../gfx/HostTexture.h"

namespace ZH {

class TGAFile {
  public:
    TGAFile(std::istream&);

    std::optional<Size> getSize();
    std::shared_ptr<GFX::HostTexture> getTexture();
  private:
    std::istream& stream;
    bool headerParsed = false;
    Size size;
    uint8_t bitsPerPixel = 0;

    bool parseHeader();
};

}

#endif
