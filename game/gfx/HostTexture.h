#ifndef H_GFX_HOST_TEXTURE
#define H_GFX_HOST_TEXTURE

#include <vector>

#include "../common.h"
#include "../Dimensions.h"

namespace ZH::GFX {

class HostTexture {
  public:
    enum class Format {
        BGRA8888
      , RGBA8888
    };

    HostTexture (Size, Format, std::vector<char>&&);

    Size getSize() const;
    Format getFormat() const;
    const std::vector<char>& getData() const;
  private:
    Size size;
    Format format;
    std::vector<char> data;
};

}

#endif
