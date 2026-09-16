// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_HOST_TEXTURE
#define H_GFX_HOST_TEXTURE

#include <vector>

#include "common.h"
#include "Dimensions.h"

namespace ZH::GFX {

class HostTexture {
  public:
    enum class Format {
        BGRA8888
      , RGBA8888
      , DXT1
      , DXT5
    };

    HostTexture (
        Size
      , Format
      , std::vector<unsigned char>&&
      , uint32_t numMipMaps = 1
    );

    const std::vector<unsigned char>& getData() const;
    Format getFormat() const;
    uint32_t getNumMipMaps() const;
    Size getSize() const;
  private:
    Size size;
    Format format;
    std::vector<unsigned char> data;
    uint32_t numMipMaps;
};

}

#endif
