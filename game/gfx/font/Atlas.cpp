// SPDX-License-Identifier: GPL-2.0

#include "Atlas.h"

namespace ZH::GFX::Font {

Atlas::Atlas(
    std::shared_ptr<HostTexture> texture
  , std::vector<Glyph>&& glyphs
) : texture(std::move(texture))
  , glyphs(glyphs)
{}

OptionalCRef<Glyph> Atlas::getGlyph(uint64_t codepoint) const {
  if (codepoint < 32 || codepoint >= 127) {
    return {};
  }

  return std::make_optional(std::cref(glyphs[codepoint - 32]));
}

OptionalCRef<HostTexture> Atlas::getTexture() const {
  return ptrToCOptional(texture);
}

}
