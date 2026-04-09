// SPDX-License-Identifier: GPL-2.0

#include "Frustum.h"
#include "../Logging.h"

namespace ZH::GFX {

Frustum::Frustum(const Camera& camera) {
  auto mat = camera.getProjectionMatrix() * camera.getCameraMatrix();

  for (size_t i = 0; i < 6; ++i) {
    auto& plane = planes[i];
    auto idx = i / 2;
    auto factor = (i % 2 == 0 ? 1 : -1);

    plane.distance = mat[3][3] + mat[3][idx] * factor;

    plane.normal[0] = mat[0][3] + mat[0][idx] * factor;
    plane.normal[1] = mat[1][3] + mat[1][idx] * factor;
    plane.normal[2] = mat[2][3] + mat[2][idx] * factor;

    auto length = glm::length(plane.normal);
    plane.normal /= length;
    plane.distance /= length;
  }
}

bool Frustum::isSphereInside(const glm::vec3& position, float radius) const {
  for (size_t i = 0; i < 6; ++i) {
    if (glm::dot(planes[i].normal, position) + planes[i].distance < -radius) {
      return false;
    }
  }

  return true;
}

}
