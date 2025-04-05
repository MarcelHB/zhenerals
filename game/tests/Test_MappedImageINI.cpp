#include <fstream>

#include <gtest/gtest.h>

#include "../inis/MappedImageINI.h"

namespace ZH {

TEST(MappedImageINITest, parsing) {
  std::ifstream stream {"tests/resources/MappedImageINI/textures.ini", std::ios::binary};
  MappedImageINI unit {stream};

  auto images = unit.parse();
  ASSERT_EQ(2, images.size());

  auto it = images.find("LogoBig");
  ASSERT_NE(images.cend(), it);
  EXPECT_EQ("logobig.tga", it->second.texture);
  EXPECT_EQ(1024, it->second.size.w);
  EXPECT_EQ(1022, it->second.size.h);
  EXPECT_EQ(0, it->second.topLeft.x);
  EXPECT_EQ(0, it->second.topLeft.y);
  EXPECT_EQ(1024, it->second.bottomRight.x);
  EXPECT_EQ(768, it->second.bottomRight.y);

  it = images.find("LogoSmall");
  ASSERT_NE(images.cend(), it);
  EXPECT_EQ("logosmall.tga", it->second.texture);
  EXPECT_EQ(32, it->second.size.w);
  EXPECT_EQ(24, it->second.size.h);
  EXPECT_EQ(2, it->second.topLeft.x);
  EXPECT_EQ(3, it->second.topLeft.y);
  EXPECT_EQ(31, it->second.bottomRight.x);
  EXPECT_EQ(31, it->second.bottomRight.y);
}

}
