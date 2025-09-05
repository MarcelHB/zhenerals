#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "Battlefield.h"

namespace ZH {

Battlefield::Battlefield(
    std::shared_ptr<Map> map
  , MapBuilder& mapBuilder
  , Objects::InstanceFactory& instanceFactory
) : map(map)
  , instanceFactory(instanceFactory)
{
  TRACY(ZoneScoped);

  loadInstances(mapBuilder);

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

  updateCameraMatrix();
}

const glm::mat4& Battlefield::getCameraMatrix() const {
  return cameraMatrix;
}

glm::mat4 Battlefield::getObjectToWorldMatrix(const glm::vec3& pos, float radAngle) const {
  glm::mat4 axisFlip {1.0f};
  axisFlip[1][1] = 0.0f;
  axisFlip[1][2] = 1.0f;
  axisFlip[2][1] = 1.0f;
  axisFlip[2][2] = 0.0f;

  auto gridPos =
    glm::vec3{map->getWorldToGridMatrix() * glm::vec4{pos, 1.0f}};

  auto height = map->getCenterHeight(gridPos.x, gridPos.y) / (0.0625f / 1.0f);

  return
    glm::rotate(
        glm::translate(
            axisFlip
            * map->getWorldToGridMatrix()
          , {pos.x, pos.y, height}
        )
      , radAngle
      , {0.0f, 0.0f, 1.0f}
    );
}

Daytime Battlefield::getDaytime() const {
  return Daytime::AFTERNOON;
}

std::shared_ptr<Map> Battlefield::getMap() const {
  return map;
}

std::list<std::shared_ptr<Objects::Instance>>& Battlefield::getObjectInstances() {
  return instances;
}

void Battlefield::loadInstances(MapBuilder& mapBuilder) {
  for (auto& mapObject : mapBuilder.objects) {
    auto instance = instanceFactory.getInstance(mapObject);
    if (!instance) {
      continue;
    }

    instances.emplace_back(std::move(instance));
  }
}

void Battlefield::moveCameraAxially(float x, float y) {
  auto screenXAxis =
    glm::cross(
        glm::normalize(cameraTarget - cameraPos)
      , {0.0f, -1.0f, 0.0f}
    );

  screenXAxis *= x;
  cameraPos.x += screenXAxis.x;
  cameraPos.y += y;
  cameraPos.z += screenXAxis.z;

  cameraTarget.x += screenXAxis.x;
  cameraTarget.y += y;
  cameraTarget.z += screenXAxis.z;

  updateCameraMatrix();
}

void Battlefield::moveCameraDirectionally(float x, float y) {
  auto direction = glm::normalize(cameraTarget - cameraPos);

  auto screenXAxis =
    glm::cross(
        direction
      , {0.0f, -1.0f, 0.0f}
    );

  auto rotXMatrix =
    glm::rotate(
        glm::mat4{1.0f}
      , glm::radians(y)
      , screenXAxis
    );
  auto rotYMatrix =
    glm::rotate(
        glm::mat4{1.0f}
      , glm::radians(x)
      , {0.0f, -1.0f, 0.0f}
    );
  auto rotMatrix = rotYMatrix * rotXMatrix;

  cameraTarget = cameraPos + glm::vec3{rotMatrix * glm::vec4{direction, 1.0f}};

  updateCameraMatrix();
}

void Battlefield::updateCameraMatrix() {
  cameraMatrix =
    glm::lookAt(
        cameraPos
      , cameraTarget
      , glm::vec3 {0.0f, -1.0f, 0.0f}
    );
}

void Battlefield::zoomCamera(float in) {
  auto direction = glm::normalize(cameraPos - cameraTarget);
  cameraPos += direction * in;
  cameraTarget += direction * in;

  updateCameraMatrix();
}

}
