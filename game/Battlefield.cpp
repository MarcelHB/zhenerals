// SPDX-License-Identifier: GPL-2.0

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

glm::mat4 Battlefield::getObjectToGridMatrix(const glm::vec3& pos, float radAngle) const {
  auto gridPos =
    glm::vec3 {map->getWorldToGridMatrix() * glm::vec4 {pos.x, 0, pos.y, 1.0f}};

  auto height = map->getCenterHeight(gridPos.x, gridPos.z);

  auto rotation =
    glm::rotate(
        glm::mat4 {1.0f}
      , -radAngle
      , glm::vec3 {0.0f, 1.0f, 0.0f}
    );

  auto worldTranslation =
    glm::translate(
        glm::mat4 {1.0f}
      , glm::vec3 {pos.x, pos.z, pos.y}
    );

  auto gridTranslation =
    glm::translate(
        glm::mat4 {1.0f}
      , glm::vec3 {0, height, 0}
    );

  return
      gridTranslation
      * map->getWorldToGridMatrix()
      * worldTranslation
      * rotation;
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
  GFX::Camera::Settings settings;
  settings.near = near;
  settings.far = far;
  settings.fovDeg = fovDeg;
  settings.width = width;
  settings.height = height;

  camera.setPerspectiveProjection(settings);
  newMatrices = true;
}

void Battlefield::zoomCamera(float in) {
  camera.zoom(in);
  newMatrices = true;
}

}
