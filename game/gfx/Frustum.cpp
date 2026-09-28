// SPDX-License-Identifier: GPL-2.0

#include <algorithm>

#include "Frustum.h"

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

bool Frustum::isPatchPlaneInside(const PatchPlane& plane) const {
  std::array<glm::vec3, 4> points;
  points[0] = plane.position;
  points[1] = plane.position + plane.width;
  points[2] = plane.position + plane.height;
  points[3] = plane.position + plane.width + plane.height;

  for (size_t i = 0; i < 6; ++i) {
    auto& plane = planes[i];

    auto allOutside =
      std::all_of(points.cbegin(), points.cend(), [&plane](const glm::vec3& p) {
        return glm::dot(plane.normal, p) + plane.distance < 0;
      });
    if (allOutside) {
      return false;
    }
  }

  return true;
}

bool Frustum::isSphereInside(const Sphere& sphere) const {
  for (size_t i = 0; i < 6; ++i) {
    if (glm::dot(planes[i].normal, sphere.position) + planes[i].distance < -sphere.radius) {
      return false;
    }
  }

  return true;
}

}
