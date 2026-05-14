#include <fstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../MurmurHash.h"
#include "../inis/TerrainINI.h"

namespace ZH {

TEST(TerrainINI, parsing) {
  std::ifstream stream {"tests/resources/TerrainINI/terrain.ini", std::ios::binary};
  TerrainINI unit {stream};
  auto terrains = unit.parse();

  ASSERT_EQ(3, terrains.size());

  MurmurHash3_32 hasher1;
  hasher1.feed("BallPit");

  auto lookup = terrains.find(hasher1.getHash().value);
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("colors.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::SAND, lookup->second.type);

  MurmurHash3_32 hasher2;
  hasher2.feed("Slime");

  lookup = terrains.find(hasher2.getHash().value);
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("goo.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::GRASS, lookup->second.type);

  MurmurHash3_32 hasher3;
  hasher3.feed("FloorIsLava");

  lookup = terrains.find(hasher3.getHash().value);
  ASSERT_NE(lookup, terrains.cend());

  EXPECT_EQ("lava.tga", lookup->second.textureName);
  EXPECT_EQ(ZH::TerrainType::EASTERN_EUROPE_3, lookup->second.type);
}

}
