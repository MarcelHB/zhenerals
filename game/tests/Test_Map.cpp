#include <gtest/gtest.h>

#include "Map.h"

namespace ZH {

void fillBuilder(MapBuilder&);

TEST(Map, getVertexSlice) {
  MapBuilder builder;
  builder.size = {4, 4};

  fillBuilder(builder);
  builder.heightMap = {{
      0, 1, 2, 3
    , 1, 1, 2, 3
    , 2, 2, 2, 3
    , 3, 3, 3, 3
  }};

  Map unit {builder};
  auto result =
    unit.getVertexSlice(
      IntFlatBox {
          .position = {1, 1}
        , .size = {2, 2}
      }
    );
  ASSERT_EQ(24, result.first.size());
  ASSERT_EQ(16, result.second.size());

  EXPECT_EQ(
      std::vector<uint32_t>(
          unit.getVertexIndices().cbegin()
        , unit.getVertexIndices().cbegin() + 24
      )
    , result.first
  );

  auto& data = unit.getVertexData();
  EXPECT_EQ(data[20].position.y, result.second[0].position.y);
  EXPECT_EQ(data[43].position.y, result.second[15].position.y);
}

void fillBuilder(MapBuilder& builder) {
  size_t size = builder.size.x * builder.size.y;

  builder.heightMap.resize(size, 0);
  builder.tileIndices.resize(size, 0);
  builder.blendTileIndices.resize(size, 0);

  builder.textureClasses.resize(1);
  auto& tc = builder.textureClasses[0];
  tc.width = 1;
}

}
