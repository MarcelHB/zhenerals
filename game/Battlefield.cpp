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
  camera.reposition(
      glm::vec3 {
          size.x * 0.45f
        , 255 * 0.15f
        , size.y * 0.45f
      }
    , glm::vec3 {
          size.x / 2.0f
        , map->getCenterHeight(size.x / 2, size.y / 2)
        , size.y / 2.0f
      }
    , glm::vec3 { 0.0f, -1.0f, 0.0f }
  );
}

bool Battlefield::cameraHasMoved() const {
  return newMatrices;
}

void Battlefield::frameDoneTick() {
  newMatrices = false;
}

const GFX::Camera& Battlefield::getCamera() const {
  return camera;
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
  camera.moveAxially(x, y);
  newMatrices = true;
}

void Battlefield::moveCameraDirectionally(float x, float y) {
  camera.moveDirectionally(x, y);
  newMatrices = true;
}

void Battlefield::setPerspectiveProjection(
    float near
  , float far
  , float fovDeg
  , float width
  , float height
) {
  camera.setPerspectiveProjection(near, far, fovDeg, width, height);
  newMatrices = true;
}

void Battlefield::zoomCamera(float in) {
  camera.zoom(in);
  newMatrices = true;
}

}
