// SPDX-License-Identifier: GPL-2.0

#include "../MurmurHash.h"
#include "RoadsBridgesINI.h"

namespace ZH {

RoadsBridgesINI::RoadsBridgesINI(std::istream& stream) : INIFile(stream) {}

RoadsBridgesINI::RoadsAndBridges RoadsBridgesINI::parse() {
  RoadsAndBridges roadsBridges;

  while (!stream.eof()) {
    auto token = consumeComment();
    if (token == "Road") {
      parseRoad(roadsBridges.roads);
    } else if (token == "Bridge") {
      parseBridge(roadsBridges.bridges);
    }
  }

  return roadsBridges;
}

void RoadsBridgesINI::parseBridge(std::unordered_map<uint32_t, Bridge>& bridges) {
  Bridge bridge;

  auto parseModel = [this, &bridge](uint8_t idx) {
    bridge.models[idx] = parseString();
  };
  auto parseTowerObject = [this, &bridge](uint8_t idx) {
    bridge.towerObjects[idx] = parseString();
  };
  auto parseTexture = [this, &bridge](uint8_t idx) {
    bridge.textures[idx] = parseString();
  };

  advanceStream();
  auto key = getTokenInLine();
  auto token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "BridgeModelName") {
      parseModel(0);
    } else if (token == "BridgeModelNameDamaged") {
      parseModel(1);
    } else if (token == "BridgeModelNameReallyDamaged") {
      parseModel(2);
    } else if (token == "BridgeModelNameBroken") {
      parseModel(3);
    } else if (token == "BridgeScale") {
      bridge.scale = parseFloat().value_or(bridge.scale);
    } else if (token == "DamagedToSound") {
      bridge.damagedSound = parseString();
    } else if (token == "NumFXPerType") {
      bridge.numEffectsPerType = parseInteger().value_or(bridge.numEffectsPerType);
    } else if (token == "RadarColor") {
      bridge.radarColor = parseRGB();
    } else if (token == "RepairedToSound") {
      bridge.repairedSound = parseString();
    } else if (token == "ScaffoldObjectName") {
      bridge.scaffoldObject = parseString();
    } else if (token == "ScaffoldSupportObjectName") {
      bridge.scaffoldSupportObject = parseString();
    } else if (token == "Texture") {
      parseTexture(0);
    } else if (token == "TextureDamaged") {
      parseTexture(1);
    } else if (token == "TextureReallyDamaged") {
      parseTexture(2);
    } else if (token == "TextureBroken") {
      parseTexture(3);
    } else if (token == "TowerObjectNameFromLeft") {
      parseTowerObject(0);
    } else if (token == "TowerObjectNameFromRight") {
      parseTowerObject(1);
    } else if (token == "TowerObjectNameToLeft") {
      parseTowerObject(2);
    } else if (token == "TowerObjectNameToRight") {
      parseTowerObject(3);
    } else if (token == "TransitionEffectsHeight") {
      bridge.transitionEffectsHeight = parseFloat().value_or(bridge.transitionEffectsHeight);
    } else if (token == "TransitionToFX") {
      parseTransition(bridge, false);
    } else if (token == "TransitionToOCL") {
      parseTransition(bridge, true);
    }

    token = consumeComment();
  }

  MurmurHash3_32 hasher;
  hasher.feed(key);
  bridges.emplace(hasher.getHash().value, std::move(bridge));
}

void RoadsBridgesINI::parseTransition(Bridge& bridge, bool isOCL) {
  auto values = parseStringList();
  if (values.size() != 4) {
    return;
  }

  Bridge::Transition transition;

  if (values[0] == "Transition:Damage") {
    transition.damage = true;
  } else if (values[0] == "Transition:Repair") {
    transition.damage = false;
  } else {
    return;
  }

  if (values[1] == "ToState:PRISTINE") {
    transition.toState = Bridge::State::PRISTINE;
  } else if (values[1] == "ToState:DAMAGED") {
    transition.toState = Bridge::State::DAMAGED;
  } else if (values[1] == "ToState:REALLYDAMAGED") {
    transition.toState = Bridge::State::REALLYDAMAGED;
  } else if (values[1] == "ToState:RUBBLE") {
    transition.toState = Bridge::State::RUBBLE;
  } else {
    return;
  }

  if (values[2].find("EffectNum:") != 0) {
    return;
  }
  transition.effectNum =
    parseInteger(values[2].substr(values[2].find(":") + 1))
      .value_or(transition.effectNum);

  size_t offset = 0;
  if (isOCL && values[3].find("OCL:") == 0) {
    offset = 4;
  } else if (!isOCL && values[3].find("FX:") == 0) {
    offset = 3;
  } else {
    return;
  }

  transition.name = values[3].substr(offset);

  if (isOCL) {
    bridge.transitionOCLs.emplace_back(std::move(transition));
  } else {
    bridge.transitionEffects.emplace_back(std::move(transition));
  }
}

void RoadsBridgesINI::parseRoad(std::unordered_map<uint32_t, Road>& roads) {
  Road road;

  advanceStream();
  auto key = getTokenInLine();
  auto token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "Texture") {
      road.texture = parseString();
      if (road.texture.empty()) {
        return;
      }
    } else if (token == "RoadWidth") {
      road.width = parseFloat().value_or(road.width);
    } else if (token == "RoadWidthInTexture") {
      road.widthInTexture = parseFloat().value_or(road.widthInTexture);
    }

    token = consumeComment();
  }

  MurmurHash3_32 hasher;
  hasher.feed(key);
  roads.emplace(hasher.getHash().value, std::move(road));
}

}
