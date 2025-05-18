#include <gtest/gtest.h>

#include "../Geometry.h"

#include <glm/glm.hpp>

namespace ZH {

TEST(Geometry, getPointsInPolygon) {

  std::vector<glm::ivec2> polygons {{
      {0, 0}
    , {0, 1}
    , {2, 3}
    , {2, 0}
  }};

  Size size {4, 4};
  auto result = getPointsInPolygon(size, polygons, glm::mat4(1));

  EXPECT_LE(1, result[0]);
  EXPECT_LE(1, result[1]);
  EXPECT_LE(1, result[2]);
  EXPECT_EQ(0, result[3]);

  EXPECT_LE(1, result[4]);
  EXPECT_LE(1, result[5]);
  EXPECT_LE(1, result[6]);
  EXPECT_EQ(0, result[7]);

  EXPECT_LE(0, result[8]);
  EXPECT_LE(1, result[9]);
  EXPECT_LE(1, result[10]);
  EXPECT_EQ(0, result[11]);

  EXPECT_LE(0, result[12]);
  EXPECT_LE(0, result[13]);
  EXPECT_LE(1, result[14]);
  EXPECT_EQ(0, result[15]);
}

}
