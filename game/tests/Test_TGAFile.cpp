#include <fstream>

#include <gtest/gtest.h>

#include "../formats/TGAFile.h"

namespace ZH {

TEST(TGAFileTest, parsing32bit) {
  std::ifstream stream {"tests/resources/TGAFile/pixels32.tga", std::ios::binary};
  TGAFile unit {stream};

  auto tga = unit.getTexture();
  ASSERT_TRUE(tga);

  auto size = tga->getSize();
  EXPECT_EQ(2, size.x);
  EXPECT_EQ(4, size.y);

  EXPECT_EQ(GFX::HostTexture::Format::BGRA8888, tga->getFormat());

  auto& data = tga->getData();
  EXPECT_EQ(0x55, data[0]);
  EXPECT_EQ(0x66, data[1]);
  EXPECT_EQ(0x77, data[2]);
  EXPECT_EQ('\xFF', data[3]);

  EXPECT_EQ(0x22, data[16]);
  EXPECT_EQ(0x33, data[17]);
  EXPECT_EQ(0x44, data[18]);
  EXPECT_EQ('\xFF', data[19]);
}

TEST(TGAFileTest, parsing24bit) {
  std::ifstream stream {"tests/resources/TGAFile/pixels24.tga", std::ios::binary};
  TGAFile unit {stream};

  auto tga = unit.getTexture();
  ASSERT_TRUE(tga);

  auto size = tga->getSize();
  EXPECT_EQ(2, size.x);
  EXPECT_EQ(4, size.y);

  EXPECT_EQ(GFX::HostTexture::Format::BGRA8888, tga->getFormat());

  auto& data = tga->getData();
  EXPECT_EQ(0x55, data[0]);
  EXPECT_EQ(0x66, data[1]);
  EXPECT_EQ(0x77, data[2]);
  EXPECT_EQ('\xFF', data[3]);

  EXPECT_EQ(0x22, data[16]);
  EXPECT_EQ(0x33, data[17]);
  EXPECT_EQ(0x44, data[18]);
  EXPECT_EQ('\xFF', data[19]);
}

TEST(TGAFileTest, parsingHeader) {
  std::ifstream stream {"tests/resources/TGAFile/pixels24.tga", std::ios::binary};
  TGAFile unit {stream};

  auto size = unit.getSize();
  ASSERT_TRUE(size);

  EXPECT_EQ(2, size->x);
  EXPECT_EQ(4, size->y);
}

}
