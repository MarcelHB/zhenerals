#include <glm/gtc/matrix_transform.hpp>

#include "Battlefield.h"

namespace ZH {

Battlefield::Battlefield(std::shared_ptr<Map>&& _map)
  : map(std::move(_map))
{
  auto size = map->getSize();
  cameraTarget =
    glm::vec3 {
        size.x / 2.0f
      , map->getCenterHeight(size.x / 2, size.y / 2)
      , size.y / 2.0f
    };

  cameraPos =
    glm::vec3 {
        size.x * 0.45f
      , 255 * 0.15f
      , size.y * 0.45f
    };
}

glm::mat4 Battlefield::getCameraMatrix() {
  return glm::lookAt(
      cameraPos
    , cameraTarget
    , glm::vec3 {0.0f, -1.0f, 0.0f}
  );
}

std::shared_ptr<Map> Battlefield::getMap() const {
  return map;
}

}
