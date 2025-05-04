#include <fstream>

#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../BattlefieldFactory.h"

namespace ZH {

// Steam version v1.05, EN

void dumpHeightMap(const Map&);

TEST(MapTest, parsing) {
  Config config;
  ResourceLoader mapsLoader {{"MapsZH.big"}, config.baseDir};

  BattlefieldFactory factory {mapsLoader};
  auto battlefield = factory.load("shellmapmd");
  ASSERT_TRUE(battlefield);

  auto map = battlefield->getMap();

  auto size = map->getSize();
  EXPECT_EQ(315, size.x);
  EXPECT_EQ(315, size.y);

  EXPECT_EQ(99225, map->getHeightMap().size());
  //dumpHeightMap(*map);
  EXPECT_EQ(16, map->getTexturesIndex().size());
  EXPECT_EQ(396900, map->getVertexData().size());
  EXPECT_EQ(595350, map->getVertexIndices().size());
}

void dumpHeightMap(const Map& map) {
  auto size = map.getSize();
  auto& data = map.getHeightMap();

  std::ofstream out {"heightmap.ppm"};
  out << "P2" << std::endl
    << size.x << " "  << size.y << std::endl
    << "255" << std::endl;

  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      auto v = *(data.data() + (y * size.x + x));
      out << std::to_string(v) << " ";
    }
    out << std::endl;
  }
}

}
