#include "../../Logging.h"
#include "FontManager.h"

namespace ZH::GFX::Font {

FontManager::FontManager() {
  if (!textureGenerator.init()) {
    broken = true;
    ERROR_ZH("FontManager", "TextureGenerator not initialized.");
  }
}

std::shared_ptr<Atlas> FontManager::getFont(uint8_t size, bool bold) {
  if (broken) {
    return {};
  }

  FontKey key {size, bold};
  auto lookup = fonts.find(key);
  if (lookup != fonts.cend()) {
    return lookup->second;
  }

  std::shared_ptr<Atlas> newFont = {};
  if (bold) {
    newFont = textureGenerator.createFontTexture(readFile("fonts/NotoSans_SemiCondensed-Bold.ttf"), size);
  } else {
    newFont = textureGenerator.createFontTexture(readFile("fonts/NotoSans_SemiCondensed-Regular.ttf"), size);
  }

  if (newFont) {
    fonts.emplace(key, newFont);
  }

  return newFont;
}

}
