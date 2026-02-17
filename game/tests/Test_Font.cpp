#include <fstream>

#include <gtest/gtest.h>

#include "../common.h"
#include "../gfx/font/FontManager.h"

namespace ZH {

void dumpAtlas(const GFX::Font::Atlas&);
void dumpFTGlyphs();
void dumpTexture(const GFX::Font::Atlas&);

TEST(FontTest, getFont) {
  GFX::Font::FontManager unit;
  auto font1 = unit.getFont(12, false);
  auto font2 = unit.getFont(14, false);
  auto font3 = unit.getFont(14, true);

  ASSERT_TRUE(font1);
  ASSERT_TRUE(font2);
  ASSERT_TRUE(font3);

  auto t1 = font1->getTexture()->get().getData();
  auto t2 = font2->getTexture()->get().getData();
  auto t3 = font3->getTexture()->get().getData();

  EXPECT_NE(t1, t2);
  EXPECT_NE(t2, t3);
  EXPECT_NE(t1, t3);
}

TEST(FontTest, glyphs) {
  GFX::Font::FontManager unit;
  auto font = unit.getFont(16, false);
  ASSERT_TRUE(font);
  /*
  dumpTexture(*font);
  dumpAtlas(*font);
  dumpFTGlyphs();
  */

  EXPECT_FALSE(font->getGlyph(17));

  int32_t lastX = 0;
  int32_t lastY = 0;

  for (uint8_t i = 32; i < 127; ++i) {
    auto glyph = font->getGlyph(i);
    ASSERT_TRUE(glyph);

    if (glyph->get().position.x != lastX) {
      EXPECT_EQ(0, glyph->get().position.x);
      lastX = glyph->get().size.x;
      EXPECT_EQ(lastY + glyph->get().size.y, glyph->get().position.y);
      lastY += glyph->get().size.y;
    } else {
      EXPECT_EQ(lastX, glyph->get().position.x);
      EXPECT_EQ(lastY, glyph->get().position.y);
      lastX += glyph->get().size.x;
    }
  }
}

void dumpTexture(const GFX::Font::Atlas& font) {
  auto& texture = font.getTexture()->get();
  auto size = texture.getSize();
  auto& data = texture.getData();

  std::ofstream out {"texture.ppm"};
  out << "P3" << std::endl
    << size.x << " " << size.y << std::endl
    << "255" << std::endl;

  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      for (uint8_t i = 0; i < 3; ++i) {
        uint8_t v = static_cast<uint8_t>(*(data.begin() + (y * size.x * 4) + (x * 4) + i));
        out << std::to_string(v) << " ";
      }
    }
    out << std::endl;
  }
}

void dumpAtlas(const GFX::Font::Atlas& font) {
  std::ofstream out {"glyphs.csv"};
  out << "i,c,x,y,w,h" << std::endl;

  for (uint8_t i = 32; i < 127; ++i) {
    auto glyphOpt = font.getGlyph(i);
    if (!glyphOpt) {
      continue;
    }

    auto& glyph = glyphOpt->get();
    out << std::to_string(i);

    if (i == 34) {
      out << ",dbl-quote";
    } else {
      out << ",\"" << static_cast<char>(i) << "\"";
    }
    out << "," << glyph.position.x
      << "," << glyph.position.y
      << "," << glyph.size.x
      << "," << glyph.size.y
      << std::endl;
  }
}

void dumpFTGlyphs() {
  auto fontData = readFile("fonts/NotoSans_SemiCondensed-Regular.ttf");

  FT_Library ftLibrary = nullptr;
  FT_Init_FreeType(&ftLibrary);

  FT_Face face = nullptr;
  FT_New_Memory_Face(
      ftLibrary
    , reinterpret_cast<FT_Byte*>(fontData.data())
    , fontData.size()
    , 0
    , &face
  );

  FT_Set_Pixel_Sizes(face, 0, 16);

  std::ofstream out {"ft_glyphs.csv"};
  out << "i,c,g.w,g.h,g.hbX,g.hbY,g.ha,g.va,bm.l,bm.t" << std::endl;

  for (uint8_t i = 32; i < 127; ++i) {
    auto ftIndex = FT_Get_Char_Index(face, i);
    FT_Load_Glyph(face, ftIndex, FT_LOAD_DEFAULT);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

    out << std::to_string(i);
    if (i == 34) {
      out << ",dbl-quote";
    } else {
      out << ",\"" << static_cast<char>(i) << "\"";
    }

    auto& metrics = face->glyph->metrics;
    out << "," << metrics.width / 64
      << "," << metrics.height / 64
      << "," << metrics.horiBearingX / 64
      << "," << metrics.horiBearingY / 64
      << "," << metrics.horiAdvance / 64
      << "," << metrics.vertAdvance / 64
      << "," << face->glyph->bitmap_left
      << "," << face->glyph->bitmap_top
      << std::endl;
  }

  FT_Done_Face(face);

  FT_Done_FreeType(ftLibrary);
}

}
