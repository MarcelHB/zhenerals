#include <gtest/gtest.h>

#include "MurmurHash.h"
#include "inis/RoadsBridgesINI.h"

namespace ZH {

TEST(RoadsBridgesINI, parsing) {
  std::ifstream stream {"tests/resources/RoadsBridgesINI/roads_bridges.ini", std::ios::binary};

  RoadsBridgesINI unit {stream};
  auto roadsBridges = unit.parse();

  // Roads
  auto& roads = roadsBridges.roads;
  ASSERT_EQ(2, roads.size());

  MurmurHash3_32 hasher;
  hasher.feed("Autobahn");
  auto lookup = roads.find(hasher.getHash().value);
  ASSERT_TRUE(lookup != roads.cend());

  auto& road1 = lookup->second;
  EXPECT_EQ("concrete.tga", road1.texture);
  EXPECT_FLOAT_EQ(123.7f, road1.width);
  EXPECT_FLOAT_EQ(100.0f, road1.widthInTexture);

  MurmurHash3_32 hasher2;
  hasher2.feed("Interstate");
  lookup = roads.find(hasher2.getHash().value);
  ASSERT_TRUE(lookup != roads.cend());

  auto& road2 = lookup->second;
  EXPECT_EQ("cardboard.tga", road2.texture);
  EXPECT_FLOAT_EQ(1200.0f, road2.width);
  EXPECT_FLOAT_EQ(0.4f, road2.widthInTexture);

  // Bridges
  auto& bridges = roadsBridges.bridges;
  ASSERT_EQ(1, bridges.size());

  MurmurHash3_32 hasher3;
  hasher3.feed("PontNeuf");
  auto bridgeLookup = bridges.find(hasher3.getHash().value);
  ASSERT_TRUE(bridgeLookup != bridges.cend());

  auto& bridge = bridgeLookup->second;
  EXPECT_EQ(2, bridge.transitionOCLs.size());
  EXPECT_EQ(2, bridge.transitionEffects.size());

  EXPECT_TRUE(
    std::none_of(
        bridge.models.cbegin()
      , bridge.models.cend()
      , [](const std::string& c) { return c.empty(); }
    )
  );
  EXPECT_TRUE(
    std::none_of(
        bridge.textures.cbegin()
      , bridge.textures.cend()
      , [](const std::string& c) { return c.empty(); }
    )
  );
  EXPECT_TRUE(
    std::none_of(
        bridge.towerObjects.cbegin()
      , bridge.towerObjects.cend()
      , [](const std::string& c) { return c.empty(); }
    )
  );
}

}
