#include <fstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../inis/TerrainINI.h"

namespace ZH {

TEST(TerrainINI, parsing) {
  std::ifstream stream {"tests/resources/TerrainINI/terrain.ini", std::ios::binary};
  TerrainINI unit {stream};
  auto terrains = unit.parse();

  ASSERT_EQ(3, terrains.size());

  auto lookup = terrains.find("BallPit");
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("colors.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::SAND, lookup->second.type);

  lookup = terrains.find("Slime");
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("goo.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::GRASS, lookup->second.type);

  lookup = terrains.find("FloorIsLava");
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("lava.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::EASTERN_EUROPE_3, lookup->second.type);
}

}
