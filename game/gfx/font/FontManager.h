// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_FONT_FONT_MANAGER
#define H_GFX_FONT_FONT_MANAGER

#include <unordered_map>

#include "../../common.h"
#include "TextureGenerator.h"

namespace ZH::GFX::Font {

struct FontKey {
  uint8_t size;
  bool bold;

  bool operator==(const FontKey& other) const {
    return size == other.size && bold == other.bold;
  }
};

}

template<>
struct std::hash<ZH::GFX::Font::FontKey> {
  size_t operator()(const ZH::GFX::Font::FontKey& key) const noexcept {
    return key.size | (key.bold << 9);
  };
};

namespace ZH::GFX::Font {

class FontManager {
  public:
    FontManager();

    std::shared_ptr<Atlas> getFont(uint8_t, bool bold = false);
  private:
    bool broken = false;
    TextureGenerator textureGenerator;
    std::unordered_map<FontKey, std::shared_ptr<Atlas>> fonts;
};

}

#endif
