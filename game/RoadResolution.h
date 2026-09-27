// SPDX-License-Identifier: GPL-2.0

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "Battlefield.h"

namespace ZH {

class RoadResolution {
  public:
    using TextureCutsT =
      const std::vector<
        // position, size, as UV coord
        std::pair<const glm::vec2, const glm::vec2>
      >;

    enum class RoadElementType : uint8_t {
        STRAIGHT = 0
      , BROAD_CURVE
      , Y_JUNCTION
      , T_JUNCTION
      , TIGHT_CURVE
      , JOINT
      , K_JUNCTION
      , CROSS
    };

    struct RoadElement {
      RoadElementType type;
      glm::vec3 location;
      // the INI ref
      std::string roadType;

      float floorRotation = 0.0f;
      glm::vec3 slopeRotationAxis;
      float slopeRotation = 0.0f;

      // for straight street in length direction, 1.0f = full texture length
      float stretch = 1.0f;
      // where to start UV-wise in length direction
      float offset = 0.0f;

      // corners: what vertices need to be prolonged by the texture width
      uint8_t toothBits = 0;
    };

    static constexpr float T_SIZE = 512.0f;
    static const TextureCutsT TEXTURE_CUTS;

    RoadResolution(const Battlefield&);

    std::list<RoadElement> resolveRoadSystem(const Battlefield::RoadNode&) const;
  private:
    const Battlefield& battlefield;

    void resolveRoadSystem(
        const Battlefield::RoadNode&
      , std::list<RoadElement>&
      , uint8_t
    ) const;
};

}
