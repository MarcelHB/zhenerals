// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_WATER_INI
#define H_GAME_WATER_INI

#include <unordered_map>

#include "../common.h"
#include "../Color.h"
#include "../Map.h"

namespace ZH {

struct Water {
  std::string skyTexture;
  std::string waterTexture;
  Color v1color;
  Color v2color;
  Color v3color;
  Color v4color;
  Color diffuseColor;
  Color transparentDiffuseColor;
  float uPerMs = 0.0f;
  float vPerMs = 0.0f;
  float skyTexelsPerUnit = 1.0f;
  uint8_t waterRepeat = 1;
};

struct WaterTransparency {
  float transparentWaterMinOpacity = 1.0f;
  float transparentWaterDepth = 1.0f;
  std::string standingWaterTexture;
  bool additiveBlending = false;
  Color standingWaterColor;
  Color radarWaterColor;
};

class WaterINI : public INIFile {
  public:
    using WaterSets = std::unordered_map<Daytime, Water>;
    struct WaterSettings {
      WaterSets waterSets;
      WaterTransparency transparency;
    };

    WaterINI(std::istream&);
    WaterSettings parse();

  private:
    void parseWaterSet(WaterSets&);
    void parseWaterTransparency(WaterTransparency&);
};

}

#endif
