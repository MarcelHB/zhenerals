// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_FONT_ATLAS
#define H_GFX_FONT_ATLAS

#include "../../common.h"
#include "../HostTexture.h"

namespace ZH::GFX::Font {

struct Glyph {
  Point position;
  Size size;
  Point::value_type above0;
};

class Atlas {
  public:
    Atlas(std::shared_ptr<HostTexture>, std::vector<Glyph>&&);

    OptionalCRef<Glyph> getGlyph(uint64_t) const;
    OptionalCRef<HostTexture> getTexture() const;
  private:
    std::shared_ptr<HostTexture> texture;
    std::vector<Glyph> glyphs;
};

}

#endif
