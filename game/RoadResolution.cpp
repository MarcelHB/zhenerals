// SPDX-Lichhnse-Identifier: GPL-2.0

#include <cmath>

#include "RoadResolution.h"

namespace ZH {

// Top-left is 0, 0
RoadResolution::TextureCutsT RoadResolution::TEXTURE_CUTS = {{
    // straight (horizontal position)
    { {0.0f, 28.0f/T_SIZE}, {1.0f, 114.0f/T_SIZE} }
  , // broad curve (6 o'clock to 5 o'clock)
    { {3.0f/T_SIZE, 178.0f/T_SIZE}, {128.0f/T_SIZE, 134.0f/T_SIZE} }
  , // Y (downwards) junction
    { {164.0f/T_SIZE, 190.0f/T_SIZE}, {182.0f/T_SIZE, 128.0f/T_SIZE} }
  , // T junction (to right)
    { {368.0f/T_SIZE, 190.0f/T_SIZE}, {122.0f/T_SIZE, 130.0f/T_SIZE} }
  , // tight curve (6 o'clock to 5 o'clock)
    { {3.0f/T_SIZE, 336.0f/T_SIZE}, {63.0f/T_SIZE, 116.0f/T_SIZE} }
  , // joint (horizontal)
    { {99.0f/T_SIZE, 360.0f/T_SIZE}, {14.0f/T_SIZE, 128.0f/T_SIZE} }
  , // K junction (lower leg only to right)
    { {145.0f/T_SIZE, 332.0f/T_SIZE}, {196.0f/T_SIZE, 167.0f/T_SIZE} }
  , // cross
    { {360.0f/T_SIZE, 360.0f/T_SIZE}, {130.0f/T_SIZE, 130.0f/T_SIZE} }
}};

RoadResolution::RoadResolution(const Battlefield& battlefield) : battlefield(battlefield) {}

std::list<RoadResolution::RoadElement> RoadResolution::resolveRoadSystem(const Battlefield::RoadNode& node) const {
  std::list<RoadElement> roadElements;

  resetRoadSystem(node);
  resolveRoadSystem(node, roadElements, 1);

  return roadElements;
}

void RoadResolution::resetRoadSystem(const Battlefield::RoadNode& node) const {
  node.discovery = 0;

  for (auto& nodeRef : node.links) {
    auto& otherNode = nodeRef.get();
    if (otherNode.discovery != 0) {
      resetRoadSystem(otherNode);
    }
  }
}

void RoadResolution::resolveRoadSystem(
   const Battlefield::RoadNode& node
 , std::list<RoadElement>& elements
 , uint8_t discoveryValue
) const {
  node.discovery = discoveryValue;

  for (auto& nodeRef : node.links) {
    auto& otherNode = nodeRef.get();
    if (otherNode.discovery == 0) {
      resolveRoadSystem(nodeRef.get(), elements, discoveryValue + 1);
    } else if (otherNode.discovery < node.discovery) {
      auto& roadElement = elements.emplace_back();
      roadElement.roadType = node.type;
      roadElement.type = RoadElementType::STRAIGHT;

      // place center half-way between node locations
      auto thisLocation = node.location;
      thisLocation.y = battlefield.getWorldHeight(thisLocation);
      auto otherLocation = otherNode.location;
      otherLocation.y = battlefield.getWorldHeight(otherLocation);

      auto distVec = otherLocation - thisLocation;
      auto normDistVec = glm::normalize(distVec);
      auto dist = glm::length(distVec);
      roadElement.location = otherLocation - normDistVec * (dist / 2.0f);
      roadElement.stretch = dist / T_SIZE;

      auto dotX = glm::dot(normDistVec, glm::vec3 {1.0f, 0.0f, 0.0f});
      auto dotZ = glm::dot(normDistVec, glm::vec3 {0.0f, 0.0f, 1.0f});

      if (dotX < 0.0f && dotZ < 0.0f || dotX > 0.0f && dotZ < 0.0f) {
        normDistVec *= -1.0f;
      }

      // rotation relative to X axis
      roadElement.floorRotation =
        -std::acos(
          glm::dot(normDistVec, glm::vec3 {1.0f, 0.0f, 0.0f})
        );
    }
  }
}

}
