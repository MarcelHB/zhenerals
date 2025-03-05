#ifndef H_TGA_FILE
#define H_TGA_FILE

#include <istream>
#include <memory>

#include "gfx/HostTexture.h"

namespace ZH {

class TGAFile {
  public:
    TGAFile(std::istream&);

    std::shared_ptr<GFX::HostTexture> getTexture();
  private:
    std::istream& stream;
};

}

#endif
