// SPDX-License-Identifier: GPL-2.0

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "Battlefield.h"
#include "MurmurHash.h"

namespace ZH {

uint64_t Battlefield::ScorchData::nextID = 0;

Battlefield::ScorchData::ScorchData() : id(nextID++) {}

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
  loadRoads(mapBuilder);
  loadScorches(mapBuilder);

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

  auto lightTarget =
    glm::vec3 {
        mapGameSize.x / 2.0f
      , 0
      , mapGameSize.y / 2.0f
    };

  auto lightPos =
    glm::vec3 {
        mapGameSize.x * 0.25f
      , -1500.0f
      , mapGameSize.y * 0.75f
    };

  sunlightNormal = glm::normalize(lightTarget - lightPos);
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

void Battlefield::loadRoads(MapBuilder& mapBuilder) {
  TRACY(ZoneScoped);

  std::unordered_map<uint32_t, std::reference_wrapper<RoadNode>> lookupMap;

  auto getCacheKey = [](const MapObject& road) -> uint32_t {
    union FtoU {
      float f;
      uint32_t u;
    };
    FtoU ftoU;

    MurmurHash3_32 hasher;
    for (uint8_t i = 0; i < 3; ++i) {
      ftoU.f = road.location[i];
      hasher.feed(ftoU.u);
    }

    return hasher.getHash();
  };

  auto& mapRoads = mapBuilder.roads;
  for (auto it = mapRoads.cbegin(); it != mapRoads.cend(); ++it) {
    auto& roadPt1 = *it;
    ++it;

    if (it == mapRoads.cend()) {
      break;
    }

    auto& roadPt2 = *it;

    auto key1 = getCacheKey(roadPt1);
    auto key2 = getCacheKey(roadPt2);

    OptionalRef<RoadNode> node1Ref;
    OptionalRef<RoadNode> node2Ref;

    auto nodeLookup = lookupMap.find(key1);
    if (nodeLookup == lookupMap.end()) {
      node1Ref = std::ref(roads.emplace_back());
      node1Ref->get().location = roadPt1.location;
      node1Ref->get().type = roadPt1.name;
      node1Ref->get().flags = roadPt1.flags;

      lookupMap.emplace(key1, *node1Ref);
    } else {
      node1Ref = std::ref(nodeLookup->second);
    }

    nodeLookup = lookupMap.find(key2);
    if (nodeLookup == lookupMap.end()) {
      node2Ref = std::ref(roads.emplace_back());
      node2Ref->get().location = roadPt2.location;
      node2Ref->get().type = roadPt2.name;
      node2Ref->get().flags = roadPt2.flags;

      lookupMap.emplace(key2, *node2Ref);
    } else {
      node2Ref = std::ref(nodeLookup->second);
    }

    node1Ref->get().links.emplace_back(node2Ref->get());
    node2Ref->get().links.emplace_back(node1Ref->get());
  }

  uint32_t i = 1;
  for (auto& roadNode : roads) {
    if (roadNode.roadSystem == 0) {
      floodRoad(roadNode, i++);
    }
  }
}

void Battlefield::floodRoad(RoadNode& node, uint32_t i) {
  node.roadSystem = i;

  for (auto& link : node.links) {
    if (link.get().roadSystem != i) {
      floodRoad(link.get(), i);
    }
  }
}

void Battlefield::loadScorches(MapBuilder& mapBuilder) {
  for (auto& scorch : mapBuilder.scorches) {
    auto& dict = scorch.properties;
    auto radiusOpt = dict.getFloat("objectRadius");
    if (!radiusOpt) {
      continue;
    }

    ScorchData data;
    data.location = scorch.location;
    data.radius = *radiusOpt;

    auto typeOpt = dict.getInt("scorchType");
    data.type = typeOpt.value_or(0);

    scorches.emplace_back(std::move(data));
  }
}

const glm::vec3 Battlefield::getSunlightNormal() const {
  return sunlightNormal;
}

const std::list<Battlefield::ScorchData>& Battlefield::getScorches() const {
  return scorches;
}

float Battlefield::getWorldHeight(const glm::vec3& pos) const {
  auto offPos = map->getWorldOffsetMatrix() * glm::vec4 {pos, 1.0f};
  return map->getHeight(glm::vec2 {offPos.x, offPos.z}) + pos.y * (1.0f/Map::TERRAIN_HEIGHT_SCALE);
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
