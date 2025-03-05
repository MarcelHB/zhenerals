#ifndef H_GFX_FONT_TEXTURE_GENERATOR
#define H_GFX_FONT_TEXTURE_GENERATOR

#include <memory>
#include <vector>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "Atlas.h"

namespace ZH::GFX::Font {

class TextureGenerator {
  public:
    TextureGenerator() = default;
    TextureGenerator(const TextureGenerator&) = delete;
    ~TextureGenerator();

    bool init();
    std::shared_ptr<Atlas> createFontTexture(std::vector<char>&&, uint8_t);
  private:
    FT_Library ftLibrary = nullptr;

    std::shared_ptr<Atlas> createFontAtlas(FT_Face);
};

}

#endif
