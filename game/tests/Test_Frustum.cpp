#include <gtest/gtest.h>

#include "../gfx/Frustum.h"

#include <glm/gtx/rotate_vector.hpp>

namespace ZH {

TEST(Frustum, isSphereInside0) {
  GFX::Camera cam;
  cam.reposition(
      glm::vec3 {0.0f, 0.0f, 0.0f}
    , glm::vec3 {1.0f, 0.0f, 0.0f}
    , glm::vec3 {0.0f, 1.0f, 0.0f}
  );
  cam.setPerspectiveProjection({
      .near = 0.1f
    , .far  = 10.0f
    , .fovDeg = 90.0f
    , .width = 1.0f
    , .height = 1.0f
  });

  GFX::Frustum unit {cam};

  // fully inside
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {2.0f, 0.0f, 0.0f}
    , 1.0f
  ));
  // inside but crossing
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {2.0f, 0.0f, 0.0f}
    , 20.0f
  ));
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {5.0f, 0.0f, 4.5f}
    , 1.0f
  ));
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {5.0f, 4.5f, 0.0f}
    , 1.0f
  ));
  // outside but crossing
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {-2.0f, 0.0f, 0.0f}
    , 20.0f
  ));
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {5.0f, 0.0f, -5.5f}
    , 1.0f
  ));
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {5.0f, -5.5f, 0.0f}
    , 1.0f
  ));
  // fully outside
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {-2.0f, 0.0f, 0.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {12.0f, 0.0f, 0.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {5.0f, 0.0f, 8.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {5.0f, 0.0f, -8.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {5.0f, 8.0f, 0.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {5.0f, -8.0f, 0.0f}
    , 1.0f
  ));
}

TEST(Frustum, isSphereInsideOffset) {
  GFX::Camera cam;
  cam.reposition(
      glm::vec3 {1.0f, 1.0f, 0.0f}
    , glm::vec3 {2.0f, 1.0f, 0.0f}
    , glm::vec3 {0.0f, 1.0f, 0.0f}
  );
  cam.setPerspectiveProjection({
      .near = 0.1f
    , .far  = 10.0f
    , .fovDeg = 90.0f
    , .width = 1.0f
    , .height = 1.0f
  });

  GFX::Frustum unit {cam};

  // fully inside
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {2.0f, 2.0f, 0.0f}
    , 1.0f
  ));
  // inside but crossing
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {2.0f, 2.0f, 0.0f}
    , 20.0f
  ));
  // outside but crossing
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {1.0f, 1.0f, 0.0f}
    , 20.0f
  ));
  // fully outside
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {0.0f, 0.0f, 0.0f}
    , 1.0f
  ));
  EXPECT_FALSE(unit.isSphereInside(
      glm::vec3 {13.0f, 0.0f, 0.0f}
    , 1.0f
  ));
}

TEST(Frustum, replayCase1) {
  GFX::Camera cam;
  glm::vec2 size {315, 315};

  cam.reposition(
      glm::vec3 { size.x * 0.45f, 255 * 0.15f, size.y * 0.45f }
    , glm::vec3 { size.x / 2.0f, 7.8125f, size.y / 2.0f }
    , glm::vec3 { 0.0f, -1.0f, 0.0f }
  );
  cam.setPerspectiveProjection({
      .near = 0.1f
    , .far  = 1000.0f
    , .fovDeg = 60.0f
    , .width = 1600.0f
    , .height = 900.0f
  });

  GFX::Frustum unit {cam};
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {155.956055f, 7.8125f, 166.464111f}
    , 11.8505402f
  ));
}

TEST(Frustum, replayCase2) {
  GFX::Camera cam;
  glm::vec2 size {315, 315};

  cam.reposition(
      glm::vec3 { size.x * 0.45f, 255 * 0.15f, size.y * 0.45f }
    , glm::vec3 { size.x / 2.4f, 7.8125f, size.y / 2.0f }
    , glm::vec3 { 0.0f, -1.0f, 0.0f }
  );
  cam.setPerspectiveProjection({
      .near = 0.1f
    , .far  = 1000.0f
    , .fovDeg = 60.0f
    , .width = 1600.0f
    , .height = 900.0f
  });

  GFX::Frustum unit {cam};
  EXPECT_TRUE(unit.isSphereInside(
      glm::vec3 {157.828125f, 7.83916855f, 173.15303f}
    , 4.76993608f
  ));
}

}
