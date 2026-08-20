#include "Geometry.h"

namespace ZH {

float interpolateVertexTriangle(
    const glm::vec3& v1
  , const glm::vec3& v2
  , const glm::vec3& v3
  , const glm::vec2& pos
) {
  glm::vec3 u {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
  glm::vec3 v {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

  auto c = glm::cross(u, v);
  if (c.y == 0.0f) {
    return v1.y;
  }

  return v1.y - (c.x * (pos.x - v1.x) + c.z * (pos.y - v1.z)) / c.y;
}

}
