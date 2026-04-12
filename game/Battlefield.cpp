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
  , mapGameSize {
        map->getSize().x * Map::GRID_TO_GAME_SCALE
      , map->getSize().y * Map::GRID_TO_GAME_SCALE
    }
  , instanceFactory(instanceFactory)
{
  TRACY(ZoneScoped);

  loadInstances(mapBuilder);

  camera.reposition(
      glm::vec3 {
          mapGameSize.x * 0.45f
        , 500.0f
        , mapGameSize.y * 0.45f
      }
    , glm::vec3 {
          mapGameSize.x / 2.0f
        , getWorldHeight(glm::vec3 {mapGameSize.x / 2.0f, 0.0f, mapGameSize.y / 2.0f})
        , mapGameSize.y / 2.0f
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

glm::mat4 Battlefield::getWorldMatrix(const glm::vec3& pos, float radAngle) const {
  auto height = getWorldHeight(pos) * Map::TERRAIN_HEIGHT_SCALE;

  auto rotation =
    glm::rotate(
        glm::mat4 {1.0f}
      , -radAngle
      , glm::vec3 {0.0f, 1.0f, 0.0f}
    );

  auto translation =
    glm::translate(
        glm::mat4 {1.0f}
      , glm::vec3 {pos.x, height, pos.z}
    );

  return
      map->getWorldOffsetMatrix()
      * translation
      * rotation;
}

Daytime Battlefield::getDaytime() const {
  return Daytime::AFTERNOON;
}

std::shared_ptr<Map> Battlefield::getMap() const {
  return map;
}

const glm::vec2& Battlefield::getMapGameSize() const {
  return mapGameSize;
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

float Battlefield::getWorldHeight(const glm::vec3& pos) const {
  auto offPos = map->getWorldOffsetMatrix() * glm::vec4 {pos, 1.0f};
  return map->getCenterHeight(offPos.x / 10, offPos.z / 10) + pos.y;
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
