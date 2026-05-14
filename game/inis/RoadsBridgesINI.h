// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_ROADS_BRIDGES_INI
#define H_GAME_ROADS_BRIDGES_INI

#include <array>
#include <list>
#include <unordered_map>

#include "../Color.h"
#include "INIFile.h"

namespace ZH {

struct Road {
  std::string texture;
  float width = 60.0f;
  float widthInTexture = 1.0f;
};

struct Bridge {
  enum class State {
      PRISTINE
    , DAMAGED
    , REALLYDAMAGED
    , RUBBLE
  };

  struct Transition {
    // not damage: repair
    bool damage = true;
    State toState;
    uint32_t effectNum = 0;
    std::string name;
  };

  float scale = 1.0f;
  std::string scaffoldObject;
  std::string scaffoldSupportObject;
  Color radarColor;
  float transitionEffectsHeight = 0.0f;
  uint32_t numEffectsPerType = 1;
  // normal, damaged, really damaged, broken
  std::array<std::string, 4> models;
  std::array<std::string, 4> textures;
  // from left, right, to left, right
  std::array<std::string, 4> towerObjects;
  std::string damagedSound;
  std::string repairedSound;

  std::list<Transition> transitionOCLs;
  std::list<Transition> transitionEffects;
};

class RoadsBridgesINI : public INIFile {
  public:
    using Bridges = std::unordered_map<uint32_t, Bridge>;
    using Roads = std::unordered_map<uint32_t, Road>;

    struct RoadsAndBridges {
      Roads roads;
      Bridges bridges;
    };

    RoadsBridgesINI(std::istream&);
    RoadsAndBridges parse();

  private:
    void parseBridge(std::unordered_map<uint32_t, Bridge>&);
    void parseRoad(std::unordered_map<uint32_t, Road>&);
    void parseTransition(Bridge&, bool isOCL);
};

}

#endif
