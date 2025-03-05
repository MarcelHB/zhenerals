#include "../../Logging.h"
#include "TextureGenerator.h"

const static size_t BUFFER_DIMENSION = 256;
// ASCII
const static uint8_t CHAR_RANGE_MIN = 32;
const static uint8_t CHAR_RANGE_MAX = 127;

namespace ZH::GFX::Font {

bool TextureGenerator::init() {
  TRACY(ZoneScoped);
  auto error = FT_Init_FreeType(&ftLibrary);
  if (error) {
    return false;
  }

  return true;
}

std::shared_ptr<Atlas> TextureGenerator::createFontTexture(
    std::vector<char>&& data
  , uint8_t fontSize
) {
  TRACY(ZoneScoped);
  FT_Face face = nullptr;

  auto error =
    FT_New_Memory_Face(
        ftLibrary
      , reinterpret_cast<FT_Byte*>(data.data())
      , data.size()
      , 0
      , &face
    );

  if (error) {
    return {};
  }

  FT_Set_Pixel_Sizes(face, 0, fontSize);
  auto atlas = createFontAtlas(face);

  FT_Done_Face(face);

  return atlas;
}

TextureGenerator::~TextureGenerator() {
  if (ftLibrary != nullptr) {
    FT_Done_FreeType(ftLibrary);
  }
}

std::shared_ptr<Atlas> TextureGenerator::createFontAtlas(FT_Face face) {
  // 256x256 for ASCII
  std::vector<char> buffer;
  buffer.resize(BUFFER_DIMENSION * BUFFER_DIMENSION * 4, 0);

  size_t totalWidth = 0;
  size_t maxAbove0 = 0;
  size_t maxBelow0 = 0;

  // Estimations
  for (uint8_t i = CHAR_RANGE_MIN; i < CHAR_RANGE_MAX; ++i) {
    auto ftIndex = FT_Get_Char_Index(face, i);
    auto error = FT_Load_Glyph(face, ftIndex, FT_LOAD_NO_BITMAP);
    if (error) {
      WARN_ZH("Font::TextureGenerator", "No glyph found for {}", i);
      continue;
    }

    totalWidth += face->glyph->metrics.horiAdvance / 64;
    maxBelow0 = std::max(maxBelow0, static_cast<size_t>(std::max(0L, face->glyph->metrics.height / 64 - std::max(0L, face->glyph->metrics.horiBearingY / 64))));
    maxAbove0 = std::max(maxAbove0, static_cast<size_t>(std::max(0L, face->glyph->metrics.horiBearingY / 64)));
  }

  auto maxHeight = maxAbove0 + maxBelow0;
  auto maxRows = BUFFER_DIMENSION / maxHeight;
  if (totalWidth / BUFFER_DIMENSION >= maxRows) {
    WARN_ZH("Font::TextureGenerator", "Font buffer too small for font/char range");
    return {};
  }

  Point::underlying_type xOffset = 0;
  Point::underlying_type yOffset = 0;

  std::vector<Glyph> glyphs;
  glyphs.resize(CHAR_RANGE_MAX - CHAR_RANGE_MIN);

  for (uint8_t i = CHAR_RANGE_MIN; i < CHAR_RANGE_MAX; ++i) {
    auto ftIndex = FT_Get_Char_Index(face, i);
    auto error = FT_Load_Glyph(face, ftIndex, FT_LOAD_DEFAULT);
    if (error) {
      continue;
    }

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error) {
      WARN_ZH("Font::TextureGenerator", "Could not render glyph for {}", i);
      continue;
    }

    // The bitmap is `width` wide, `horiAdvance` is for style guiding in
    // a final bitmap rectangle for each char.
    auto glyphWidth = static_cast<Size::underlying_type>(face->glyph->metrics.width / 64);
    auto glyphWidthAndSpace = static_cast<Size::underlying_type>(face->glyph->metrics.horiAdvance / 64);
    auto glyphXOffset = static_cast<Size::underlying_type>(std::max(0L, face->glyph->metrics.horiBearingX / 64));
    if (xOffset + glyphWidthAndSpace >= BUFFER_DIMENSION) {
      xOffset = 0;
      yOffset += maxHeight;
    }

    auto rowBytes = std::abs(face->glyph->bitmap.pitch);
    auto rowOffset = maxAbove0 - face->glyph->metrics.horiBearingY / 64;
    for (size_t y = 0; y < face->glyph->bitmap.rows; ++y) {
      auto pos = buffer.begin()
        + ((yOffset + rowOffset + y) * BUFFER_DIMENSION * 4)
        + (xOffset + glyphXOffset) * 4;

      for (size_t j = 0; j < glyphWidth; ++j) {
        char value = static_cast<char>(*(face->glyph->bitmap.buffer + y * rowBytes + j));
        std::array<char, 4> valueBuffer;
        valueBuffer[0] = value;
        valueBuffer[1] = value;
        valueBuffer[2] = value;
        valueBuffer[3] = static_cast<unsigned char>(value) > 0 ? '\xFF' : 0;

        std::copy(valueBuffer.cbegin(), valueBuffer.cend(), pos + j * 4);
      }
    }

    auto glyphIdx = i - CHAR_RANGE_MIN;
    glyphs[glyphIdx].position = {xOffset, yOffset};
    glyphs[glyphIdx].size = {glyphWidthAndSpace, static_cast<Size::underlying_type>(maxHeight)};
    glyphs[glyphIdx].above0 = face->glyph->metrics.horiBearingY / 64;

    xOffset += glyphWidthAndSpace;
  }

  Size size {BUFFER_DIMENSION, BUFFER_DIMENSION};
  auto texture = std::make_shared<HostTexture>(size, HostTexture::Format::RGBA8888, std::move(buffer));

  return std::make_shared<Atlas>(std::move(texture), std::move(glyphs));
}

}
