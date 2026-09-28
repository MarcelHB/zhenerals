// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_FRUSTUM
#define H_GFX_FRUSTUM

#include <array>

#include "Camera.h"
#include "Geometry.h"

namespace ZH::GFX {

class Frustum {
  public:
    Frustum(const Camera& camera);

    bool isPatchPlaneInside(const PatchPlane&) const;
    bool isSphereInside(const Sphere&) const;
  private:
    struct Plane {
      glm::vec3 normal;
      float distance;
    };

    std::array<Plane, 6> planes;
};

}

#endif
