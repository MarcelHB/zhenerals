// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_FRUSTRUM
#define H_GFX_FRUSTRUM

#include <array>

#include "Camera.h"

namespace ZH::GFX {

class Frustum {
  public:
    Frustum(const Camera& camera);

    bool isSphereInside(const glm::vec3&, float) const;
  private:
    struct Plane {
      glm::vec3 normal;
      float distance;
    };

    std::array<Plane, 6> planes;
};

}

#endif
