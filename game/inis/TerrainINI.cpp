#include "TerrainINI.h"

namespace ZH {

TerrainINI::TerrainINI(std::istream& instream) : INIFile(instream) {
}

TerrainINI::Terrains TerrainINI::parse() {
  Terrains terrains;

  while (!stream.eof()) {
    parseTerrain(terrains);
  }

  return terrains;
}

void TerrainINI::parseTerrain(Terrains& terrains) {
  auto token = consumeComment();

  if (token != "Terrain") {
    return;
  }

  advanceStream();
  auto key = getTokenInLine();
  Terrain terrain;
  token = consumeComment();

  while (token != "End" && !stream.eof()) {
    if (token == "Class") {
      auto typeOpt = parseType();
      if (!typeOpt) {
        return;
      }

      terrain.type = *typeOpt;
    } else if (token == "Texture") {
      advanceStream();
      token = getTokenInLine();
      if (token != "=") {
        return;
      }

      advanceStream();
      token = getTokenInLine();
      terrain.textureName = std::move(token);
    }

    token = consumeComment();
  }

  terrains.emplace(std::move(key), std::move(terrain));
}

std::optional<TerrainType> TerrainINI::parseType() {
  advanceStream();
  auto token = getTokenInLine();
  if (token != "=") {
    return {};
  }

  advanceStream();
  token = getTokenInLine();

  if (token == "NONE") {
    return TerrainType::NONE;
  } else if (token == "DESERT_1") {
    return TerrainType::DESERT_1;
  } else if (token == "DESERT_2") {
    return TerrainType::DESERT_2;
  } else if (token == "DESERT_3") {
    return TerrainType::DESERT_3;
  } else if (token == "EASTERN_EUROPE_1") {
    return TerrainType::EASTERN_EUROPE_1;
  } else if (token == "EASTERN_EUROPE_2") {
    return TerrainType::EASTERN_EUROPE_2;
  } else if (token == "EASTERN_EUROPE_3") {
    return TerrainType::EASTERN_EUROPE_3;
  } else if (token == "SWISS_1") {
    return TerrainType::SWISS_1;
  } else if (token == "SWISS_2") {
    return TerrainType::SWISS_2;
  } else if (token == "SWISS_3") {
    return TerrainType::SWISS_3;
  } else if (token == "SNOW_1") {
    return TerrainType::SNOW_1;
  } else if (token == "SNOW_2") {
    return TerrainType::SNOW_2;
  } else if (token == "SNOW_3") {
    return TerrainType::SNOW_3;
  } else if (token == "DIRT") {
    return TerrainType::DIRT;
  } else if (token == "GRASS") {
    return TerrainType::GRASS;
  } else if (token == "TRANSITION") {
    return TerrainType::TRANSITION;
  } else if (token == "ROCK") {
    return TerrainType::ROCK;
  } else if (token == "SAND") {
    return TerrainType::SAND;
  } else if (token == "CLIFF") {
    return TerrainType::CLIFF;
  } else if (token == "WOOD") {
    return TerrainType::WOOD;
  } else if (token == "BLEND_EDGE") {
    return TerrainType::BLEND_EDGE;
  } else if (token == "DESERT_LIVE") {
    return TerrainType::DESERT_LIVE;
  } else if (token == "DESERT_DRY") {
    return TerrainType::DESERT_DRY;
  } else if (token == "SAND_ACCENT") {
    return TerrainType::SAND_ACCENT;
  } else if (token == "BEACH_TROPICAL") {
    return TerrainType::BEACH_TROPICAL;
  } else if (token == "BEACH_PARK") {
    return TerrainType::BEACH_PARK;
  } else if (token == "MOUNTAIN_RUGGED") {
    return TerrainType::MOUNTAIN_RUGGED;
  } else if (token == "GRASS_COBBLESTONE") {
    return TerrainType::GRASS_COBBLESTONE;
  } else if (token == "GRASS_ACCENT") {
    return TerrainType::GRASS_ACCENT;
  } else if (token == "RESIDENTIAL") {
    return TerrainType::RESIDENTIAL;
  } else if (token == "SNOW_RUGGED") {
    return TerrainType::SNOW_RUGGED;
  } else if (token == "SNOW_FLAT") {
    return TerrainType::SNOW_FLAT;
  } else if (token == "FIELD") {
    return TerrainType::FIELD;
  } else if (token == "ASPHALT") {
    return TerrainType::ASPHALT;
  } else if (token == "CONCRETE") {
    return TerrainType::CONCRETE;
  } else if (token == "CHINA") {
    return TerrainType::CHINA;
  } else if (token == "ROCK_ACCENT") {
    return TerrainType::ROCK_ACCENT;
  } else if (token == "URBAN") {
    return TerrainType::URBAN;
  } else {
    return {};
  }
}

}
