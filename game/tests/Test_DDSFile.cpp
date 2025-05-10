#include <fstream>

#include <gtest/gtest.h>

#include "../formats/DDSFile.h"

namespace ZH {

TEST(DDSFileTest, parsingDXT1) {
  std::ifstream stream {"tests/resources/DDSFile/dxt1.dds", std::ios::binary};
  DDSFile unit {stream};

  auto dds = unit.getTexture();
  ASSERT_TRUE(dds);

  auto size = dds->getSize();
  EXPECT_EQ(2, size.x);
  EXPECT_EQ(8, size.y);

  EXPECT_EQ(GFX::HostTexture::Format::BGRA8888, dds->getFormat());

  auto data = dds->getData();
  auto u32data = reinterpret_cast<uint32_t*>(data.data());
  EXPECT_EQ(0x00000000, u32data[0]);
  EXPECT_EQ(0xFFF8FCF8, u32data[4]);
  EXPECT_EQ(0xFFA80050, u32data[8]);
  EXPECT_EQ(0xFFF8FC00, u32data[12]);
}

}
