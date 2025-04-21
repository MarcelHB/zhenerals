#include <fstream>

#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../formats/MAPFile.h"

namespace ZH {

// Steam version v1.05, EN

void dumpHeightMap(const Map&);

TEST(MapTest, parsing) {
  Config config;
  ResourceLoader rl {{"MapsZH.big"}, config.baseDir};

  auto streamOpt = rl.getFileStream("maps\\shellmapmd\\shellmapmd.map");
  ASSERT_TRUE(streamOpt);

  auto stream = streamOpt->getStream();
  InflatingStream inflatingStream {stream};
  MAPFile unit {inflatingStream};
  auto map = unit.parseMap();

  auto size = map->getSize();
  EXPECT_EQ(315, size.x);
  EXPECT_EQ(315, size.y);

  //dumpHeightMap(*map);
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
