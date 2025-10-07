// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_TERRAIN_INI
#define H_GAME_TERRAIN_INI

#include "../common.h"
#include "INIFile.h"

namespace ZH {

enum class TerrainType {
    NONE
  , DESERT_1
  , DESERT_2
  , DESERT_3
  , EASTERN_EUROPE_1
  , EASTERN_EUROPE_2
  , EASTERN_EUROPE_3
  , SWISS_1
  , SWISS_2
  , SWISS_3
  , SNOW_1
  , SNOW_2
  , SNOW_3
  , DIRT
  , GRASS
  , TRANSITION
  , ROCK
  , SAND
  , CLIFF
  , WOOD
  , BLEND_EDGE
  , DESERT_LIVE
  , DESERT_DRY
  , SAND_ACCENT
  , BEACH_TROPICAL
  , BEACH_PARK
  , MOUNTAIN_RUGGED
  , GRASS_COBBLESTONE
  , GRASS_ACCENT
  , RESIDENTIAL
  , SNOW_RUGGED
  , SNOW_FLAT
  , FIELD
  , ASPHALT
  , CONCRETE
  , CHINA
  , ROCK_ACCENT
  , URBAN
};

struct Terrain {
  TerrainType type = TerrainType::NONE;
  std::string textureName;
};

class TerrainINI : public INIFile {
  public:
    using Terrains = std::unordered_map<std::string, Terrain>;

    TerrainINI(std::istream& instream);
    Terrains parse();
  private:
    void parseTerrain(Terrains&);
    std::optional<TerrainType> parseType();
};

}

#endif
