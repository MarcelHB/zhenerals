// SPDX-License-Identifier: GPL-2.0

#include <vector>

#include "../Logging.h"
#include "MAPFile.h"

namespace ZH {

MAPFile::MAPFile(InflatingStream& instream) : stream(instream)
{}

std::shared_ptr<MapBuilder> MAPFile::parseMap() {
  TRACY(ZoneScoped);

  MapBuilder mapBuilder;

  uint32_t buffer4 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return {};
  }

  if (buffer4 != 0x704D6B43) { // 'CkMp'
    return {};
  }

  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return {};
  }

  std::vector<char> strBuffer;
  strBuffer.resize(255);

  auto numToCEntries = buffer4;
  for (uint32_t i = 0; i < numToCEntries; ++i) {
    uint8_t strLength = 0;

    if (stream.read(reinterpret_cast<char*>(&strLength), 1) != 1) {
      return {};
    }

    if (stream.read(strBuffer.data(), strLength) != strLength) {
      return {};
    }

    std::string entry {strBuffer.data(), strLength};
    if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
      return {};
    }

    mapBuilder.chunkLabels.emplace(buffer4, std::move(entry));
  }

  while (!stream.eof()) {
    parseNextChunk(mapBuilder);
  }

  return std::make_shared<MapBuilder>(std::move(mapBuilder));
}

size_t MAPFile::parseNextChunk(MapBuilder& mapBuilder) {
  auto metaDataOpt = getChunkMetaData();
  if (!metaDataOpt) {
    return 0;
  }

  auto typeLookup = mapBuilder.chunkLabels.find(metaDataOpt->id);
  if (typeLookup == mapBuilder.chunkLabels.cend()) {
    stream.seekg(metaDataOpt->payloadSize, std::ios::cur);
    return metaDataOpt->payloadSize + 10;
  }

  size_t bytesRead = parseChunk(mapBuilder, typeLookup->second, *metaDataOpt);
  if (bytesRead > metaDataOpt->payloadSize) {
    WARN_ZH("MAPFile", "Chunk {} exceeding size: {} vs. {}", typeLookup->second, bytesRead, metaDataOpt->payloadSize);
  } else {
    stream.seekg(metaDataOpt->payloadSize - bytesRead, std::ios::cur);
  }

  return metaDataOpt->payloadSize + 10;
}

std::optional<MAPFile::ChunkMetaData> MAPFile::getChunkMetaData() {
  ChunkMetaData md;

  uint32_t buffer4 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return {};
  }
  md.id = buffer4;

  uint32_t buffer2 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer2), 2) != 2) {
    return {};
  }
  md.version = buffer2;

  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return {};
  }
  md.payloadSize = buffer4;

  return {md};
}

size_t MAPFile::parseChunk(
    MapBuilder& mapBuilder
  , const std::string& chunkType
  , const ChunkMetaData& metaData
) {
  if (chunkType == "BlendTileData") {
    return parseBlendTiles(mapBuilder, metaData);
  } else if (chunkType == "Condition") {
    return parseScriptCondition(mapBuilder, metaData);
  } else if (chunkType == "GlobalLighting") {
    return parseGlobalLighting(mapBuilder, metaData);
  } else if (chunkType == "HeightMapData") {
    return parseHeightMap(mapBuilder, metaData);
  } else if (chunkType == "Object") {
    return parseObject(mapBuilder, metaData);
  } else if (chunkType == "ObjectsList") {
    return parseObjectsList(mapBuilder, metaData);
  } else if (chunkType == "OrCondition") {
    return parseScriptOrCondition(mapBuilder, metaData);
  } else if (chunkType == "PlayerScriptsList") {
    return parsePlayerScriptsList(mapBuilder, metaData);
  } else if (chunkType == "PolygonTriggers") {
    return parsePolygonTriggers(mapBuilder, metaData);
  } else if (chunkType == "Script") {
    return parseScript(mapBuilder, metaData);
  } else if (chunkType == "ScriptAction") {
    return parseScriptAction(mapBuilder, metaData);
  } else if (chunkType == "ScriptActionFalse") {
    return parseScriptActionFalse(mapBuilder, metaData);
  } else if (chunkType == "ScriptGroup") {
    return parseScriptGroup(mapBuilder, metaData);
  } else if (chunkType == "ScriptList") {
    return parseScriptList(mapBuilder, metaData);
  } else if (chunkType == "SidesList") {
    return parseSidesList(mapBuilder, metaData);
  } else if (chunkType == "WorldInfo") {
    return parseWorldInfo(mapBuilder);
  } else {
    return 0;
  }
}

#define read1() \
  bytesRead = stream.read(reinterpret_cast<char*>(&buffer1), 1); \
  totalBytes += bytesRead; \
  if (bytesRead != 1) { \
    return totalBytes; \
  }

#define read4() \
  bytesRead = stream.read(reinterpret_cast<char*>(&buffer4), 4); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return totalBytes; \
  }

#define readf() \
  bytesRead = stream.read(reinterpret_cast<char*>(&bufferf), 4); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return totalBytes; \
  }

#define readString() \
  stringOpt = parseString(); \
  totalBytes += stringOpt.first; \
  if (!stringOpt.second) { \
    return totalBytes; \
  }

using StringOpt = std::pair<size_t, std::optional<std::string>>;

size_t MAPFile::parseBlendTiles(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint8_t buffer1 = 0;
  uint32_t buffer4 = 0;
  float bufferf;

  read4()
  uint32_t dataLength = buffer4;
  if (dataLength != mapBuilder.heightMap.size()) {
    WARN_ZH("MAPFile", "Blend tiles appear to be parsed before height map. Inspect.");
  }

  mapBuilder.tileIndices.resize(dataLength);
  mapBuilder.cliffInfoIndices.resize(dataLength);
  mapBuilder.blendTileIndices.resize(dataLength);
  mapBuilder.extraBlendTileIndices.resize(dataLength);

  auto statesWidthBytes = (mapBuilder.size.x + 7) / 8;
  auto statesLength = statesWidthBytes * mapBuilder.size.y;
  mapBuilder.flipStates.resize(statesLength);
  mapBuilder.cliffStates.resize(statesLength);

  bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.tileIndices.data()), dataLength * 2);
  totalBytes += bytesRead;
  if (bytesRead != dataLength * 2) {
    return totalBytes;
  }

  bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.blendTileIndices.data()), dataLength * 2);
  totalBytes += bytesRead;
  if (bytesRead != dataLength * 2) {
    return totalBytes;
  }

  if (metaData.version >= 6) {
    bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.extraBlendTileIndices.data()), dataLength * 2);
    totalBytes += bytesRead;
    if (bytesRead != dataLength * 2) {
      return totalBytes;
    }
  }

  if (metaData.version >= 5) {
    bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.cliffInfoIndices.data()), dataLength * 2);
    totalBytes += bytesRead;
    if (bytesRead != dataLength * 2) {
      return totalBytes;
    }
  }

  if (metaData.version >= 7) {
    if (metaData.version == 7) {
      auto widthBytes = (mapBuilder.size.x + 1) / 8;
      auto cliffDataLength = widthBytes * mapBuilder.size.y;
      std::vector<uint8_t> buffer;
      buffer.resize(cliffDataLength);

      bytesRead = stream.read(reinterpret_cast<char*>(buffer.data()), cliffDataLength);
      totalBytes += bytesRead;
      if (bytesRead != cliffDataLength) {
        return totalBytes;
      }

      for (size_t j = 0; j < mapBuilder.size.y; ++j) {
        for (size_t i = 0; i < widthBytes; ++j) {
          mapBuilder.cliffStates[j * statesWidthBytes + i] = buffer[j * widthBytes + i];
        }
      }
    } else {
      bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.cliffStates.data()), mapBuilder.size.y * statesWidthBytes);
      totalBytes += bytesRead;
      if (bytesRead != mapBuilder.size.y * statesWidthBytes) {
        return totalBytes;
      }
    }
  } else {
    auto getHeight = [&mapBuilder](size_t x, size_t y) {
      auto index = y * mapBuilder.size.x + x;
      if (index < mapBuilder.heightMap.size()) {
        return mapBuilder.heightMap[index] * (10.0f / 16.0f);
      } else {
        return 0.0f;
      }
    };

    auto setCliffState = [&mapBuilder, statesWidthBytes](size_t x, size_t y, bool isCliff) {
      auto index = y * statesWidthBytes + (x >> 3);
      if (index >= mapBuilder.cliffStates.size()) {
        return;
      }

      auto value = mapBuilder.cliffStates[index];
      auto mask =  1 << (x & 0x7);
      if (isCliff) {
        value |= mask;
      } else {
        value &= (~mask);
      }
      mapBuilder.cliffStates[index] = value;
    };

    for (size_t x = 0; x < mapBuilder.size.x - 1; ++x) {
      for (size_t y = 0; y < mapBuilder.size.y - 1; ++y) {
        float h1 = getHeight(x, y);
        float h2 = getHeight(x + 1, y);
        float h3 = getHeight(x, y + 1);
        float h4 = getHeight(x + 1, y + 1);

        float minZ = h1;
        if (minZ > h2) minZ = h2;
        if (minZ > h3) minZ = h3;
        if (minZ > h4) minZ = h4;

        float maxZ = h1;
        if (maxZ < h2) maxZ = h2;
        if (maxZ < h3) maxZ = h3;
        if (maxZ < h4) maxZ = h4;

        bool isCliff = (maxZ - minZ > 9.8f);
        setCliffState(x, y, isCliff);
      }
    }
  }

  read4()
  auto numBitmapTiles = buffer4;

  read4()
  auto numBlendedTiles = buffer4;
  mapBuilder.blendTileInfo.resize(numBlendedTiles);

  uint32_t numCliffInfo = 1;
  if (metaData.version >= 5) {
    read4()
    numCliffInfo = buffer4;
  }
  mapBuilder.cliffInfo.resize(numCliffInfo);

  read4()
  auto numTextureClasses = std::min(buffer4, 256u);
  mapBuilder.textureClasses.resize(numTextureClasses);

  StringOpt stringOpt;

  for (uint32_t i = 0; i < numTextureClasses; ++i) {
    auto& tc = mapBuilder.textureClasses[i];
    read4()
    tc.firstTile = buffer4;

    read4()
    tc.numTiles = buffer4;

    read4()
    tc.width = buffer4;

    read4() // isGDF

    readString()
    tc.name = *stringOpt.second;
  }

  if (metaData.version >= 4) {
    read4() // numEdgeTiles

    read4()
    uint32_t numEdgeTextureClasses = buffer4;
    mapBuilder.edgeTextureClasses.resize(numEdgeTextureClasses);

    for (uint32_t i = 0; i < numEdgeTextureClasses; ++i) {
      auto& tc = mapBuilder.edgeTextureClasses[i];

      read4()
      tc.firstTile = buffer4;

      read4()
      tc.numTiles = buffer4;

      read4()
      tc.width = buffer4;

      readString()
      tc.name = *stringOpt.second;
    }
  }

  // EVAL see where 0 ends up
  for (uint32_t i = 1; i < numBlendedTiles; ++i) {
    auto& bti = mapBuilder.blendTileInfo[i];

    read4()
    bti.blendIdx = buffer4;

    read1()
    bti.horizontal = buffer1 > 0;

    read1()
    bti.vertical = buffer1 > 0;

    read1()
    bti.rightDiagonal = buffer1 > 0;

    read1()
    bti.leftDiagonal = buffer1 > 0;

    read1()
    bti.inverted = buffer1;

    if (metaData.version >= 3) {
      read1()
      bti.longDiagonal = buffer1 > 0;
    }

    if (metaData.version >= 4) {
      read4()
      if (buffer4 != static_cast<uint32_t>(-1)) {
        bti.customBlendEdgeClass = buffer4;
      }
    }

    read4() // something flag
  }

  if (metaData.version >= 5) {
    for (uint32_t i = 1; i < numCliffInfo; ++i) {
      auto& ci = mapBuilder.cliffInfo[i];

      read4()
      ci.tileIndex = buffer4;

      for (uint8_t j = 0; j < 4; ++j) {
        readf()
        ci.u[j] = bufferf;

        readf()
        ci.v[j] = bufferf;
      }

      read1()
      ci.flip = buffer1 > 0;

      read1()
      ci.mutant = buffer1 > 0;
    }
  }

  // Ignoring v1 handling found in original for now
  if (metaData.version == 1) {
    WARN_ZH("MAPFile", "Missing v1 resize handling.");
  }

  return totalBytes;
}

size_t MAPFile::parseGlobalLighting(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint32_t buffer4 = 0;
  float bufferf = 0.0f;

  for (uint8_t i = 0; i < 4; ++i) {
    std::array<Light*, 2> lights {
        &mapBuilder.lights[i][0]
      , &mapBuilder.objectLights[i][0]
    };

    for (auto& light : lights) {
      for (uint8_t j = 0; j < 3; ++j) {
        readf()
        light->ambient[j] = bufferf;
      }
      for (uint8_t j = 0; j < 3; ++j) {
        readf()
        light->diffuse[j] = bufferf;
      }
      for (uint8_t j = 0; j < 3; ++j) {
        readf()
        light->position[j] = bufferf;
      }
    }

    if (metaData.version >= 2) {
      for (uint8_t j = 1; j < 3; ++j) {
        auto& light = mapBuilder.lights[i][j];

        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.ambient[k] = bufferf;
        }
        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.diffuse[k] = bufferf;
        }
        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.position[k] = bufferf;
        }
      }
    }

    if (metaData.version >= 3) {
      for (uint8_t j = 1; j < 3; ++j) {
        auto& light = mapBuilder.objectLights[i][j];

        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.ambient[k] = bufferf;
        }
        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.diffuse[k] = bufferf;
        }
        for (uint8_t k = 0; k < 3; ++k) {
          readf()
          light.position[k] = bufferf;
        }
      }
    }
  }

  return totalBytes;
}

size_t MAPFile::parseHeightMap(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint32_t buffer4 = 0;

  read4()
  mapBuilder.size.x = buffer4;

  read4()
  mapBuilder.size.y = buffer4;

  if (metaData.version >= 3) {
    read4()
    mapBuilder.borderSize = buffer4;
  }

  if (metaData.version >= 4) {
    read4()
    auto numBorders = buffer4;
    mapBuilder.boundaries.resize(numBorders);

    for (uint32_t i = 0; i < numBorders; ++i) {
      read4()
      auto x = buffer4;
      read4()
      auto y = buffer4;
      mapBuilder.boundaries[i] = {
          static_cast<Point::value_type>(x)
        , static_cast<Point::value_type>(y)
      };
    }
  } else {
    mapBuilder.boundaries.resize(1);
    mapBuilder.boundaries[0] =
      {
          mapBuilder.size.x - 2 * mapBuilder.borderSize
        , mapBuilder.size.y - 2 * mapBuilder.borderSize
      };
  }

  read4()
  auto dataSize = buffer4;
  mapBuilder.heightMap.resize(dataSize);

  bytesRead = stream.read(reinterpret_cast<char*>(mapBuilder.heightMap.data()), dataSize);
  totalBytes += bytesRead;
  if (bytesRead != dataSize) {
    return totalBytes;
  }

  // Ignoring v1 case
  if (metaData.version == 1) {
    WARN_ZH("MAPFile", "HeightMap v1")
  }

  return totalBytes;
}

size_t MAPFile::parseObject(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  float bufferf = 0.0f;

  MapObject object;
  for (uint8_t i = 0; i < 3; ++i) {
    readf()
    object.location[i] = bufferf;
  }

  if (metaData.version <= 2) {
    object.location[2] = 0.0f;
  }

  bytesRead = stream.read(reinterpret_cast<char*>(&object.angle), 4);
  totalBytes += bytesRead;
  if (bytesRead != 4) {
    return totalBytes;
  }

  uint32_t flags = 0;
  bytesRead = stream.read(reinterpret_cast<char*>(&flags), 4);
  totalBytes += bytesRead;
  if (bytesRead != 4) {
    return totalBytes;
  }

  StringOpt stringOpt;
  readString()
  object.name = *stringOpt.second;

  if (metaData.version >= 2) {
    totalBytes += object.properties.parse(mapBuilder.chunkLabels, stream);
  }

  auto waypointIDOpt = object.properties.getInt("waypointID");
  if (waypointIDOpt) {
    object.waypoint = true;
  }

  auto lightOpt = object.properties.getFloat("lightHeightAboveTerrain");
  if (lightOpt) {
    object.light = true;
  }

  auto scorchOpt = object.properties.getInt("scorchType");
  if (scorchOpt) {
    object.scorch = true;
  }

  mapBuilder.objects.emplace_back(std::move(object));

  return totalBytes;
}

size_t MAPFile::parseObjectsList(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;

  while (totalBytes < metaData.payloadSize) {
    // "Object" each
    totalBytes += parseNextChunk(mapBuilder);
  }

  if (totalBytes > metaData.payloadSize) {
    WARN_ZH("MAPFile", "Objects exceed objects list");
  }

  return totalBytes;
}

size_t MAPFile::parsePolygonTriggers(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint32_t buffer4 = 0;
  uint8_t buffer1 = 0;
  float bufferf = 0.0f;
  StringOpt stringOpt;

  read4()
  auto numTriggers = buffer4;
  mapBuilder.polygonTriggers.resize(numTriggers);

  for (uint32_t i = 0; i < numTriggers; ++i) {
    auto& trigger = mapBuilder.polygonTriggers[i];

    readString()
    trigger.name = *stringOpt.second;

    if (metaData.version >= 4) {
      readString()
      trigger.layerName = *stringOpt.second;
    }

    read4()
    trigger.id = buffer4;

    if (metaData.version >= 2) {
      read1()
      trigger.water = buffer1 > 0;
    }

    if (metaData.version >= 3) {
      read1()
      trigger.river = buffer1 > 0;

      read4()
      trigger.riverStart = buffer4;
    }

    read4()
    auto numPoints = buffer4;
    trigger.points.resize(numPoints);

    for (auto& pt : trigger.points) {
      for (uint8_t j = 0; j < 3; ++j) {
        read4()
        pt[j] = buffer4;
      }
    }

    // Ignoring v1 handling
    if (metaData.version == 1) {
      WARN_ZH("MAPFile", "Polygon triggers v1");
    }
  }

  return totalBytes;
}

size_t MAPFile::parsePlayerScriptsList(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  auto totalBytes = parseNextChunk(mapBuilder);

  if (totalBytes >= metaData.payloadSize) {
    WARN_ZH("MAPFile", "ScriptList exceeds limits");
  }

  return totalBytes;
}

size_t MAPFile::parseScript(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  MapScript script;
  uint8_t buffer1 = 0;
  uint32_t buffer4 = 0;
  size_t totalBytes = 0;
  size_t bytesRead = 0;

  StringOpt stringOpt;
  readString()
  script.name = *stringOpt.second;

  readString()
  script.comment = *stringOpt.second;

  readString()
  script.condition = *stringOpt.second;

  readString()
  script.action = *stringOpt.second;

  read1()
  script.action = buffer1 > 0;

  read1()
  script.once = buffer1 > 0;

  read1()
  script.easy = buffer1 > 0;

  read1()
  script.normal = buffer1 > 0;

  read1()
  script.hard = buffer1 > 0;

  read1()
  script.subroutine = buffer1 > 0;

  if (metaData.version >= 2) {
    read4()
    script.delaySec = buffer4 > 0;
  }

  mapBuilder.scripts.emplace_front(std::move(script));

  while (totalBytes < metaData.payloadSize) {
    // "OrCondition", "ScriptAction", "ScriptActionFalse"
    totalBytes += parseNextChunk(mapBuilder);
  }

  if (totalBytes > metaData.payloadSize) {
    WARN_ZH("MAPFile", "Script exceeded");
  }

  return totalBytes;
}

size_t MAPFile::parseScriptAction(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint32_t buffer4 = 0;

  if (mapBuilder.scripts.empty()) {
    WARN_ZH("MAPFile", "No script for action");
    return 0;
  }

  MapScriptAction scriptAction;

  read4()
  auto typeOpt = Script::getScriptActionTypeByValue(buffer4);
  if (!typeOpt) {
    return totalBytes;
  }
  scriptAction.type = *typeOpt;

  if (metaData.version >= 2) {
    read4()
    scriptAction.keyType = buffer4;
  }

  read4()
  auto numParams = buffer4;
  scriptAction.params.resize(numParams);
  totalBytes += parseParams(scriptAction.params);

  mapBuilder.scripts.front().actions.emplace_back(std::move(scriptAction));

  return totalBytes;
}

size_t MAPFile::parseScriptActionFalse(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint32_t buffer4 = 0;

  if (mapBuilder.scripts.empty()) {
    WARN_ZH("MAPFile", "No script for action");
    return 0;
  }

  MapScriptAction scriptAction;

  read4()
  auto typeOpt = Script::getScriptActionTypeByValue(buffer4);
  if (!typeOpt) {
    return totalBytes;
  }
  scriptAction.type = *typeOpt;

  if (metaData.version >= 2) {
    read4()
    scriptAction.keyType = buffer4;
  }

  read4()
  auto numParams = buffer4;
  scriptAction.params.resize(numParams);
  totalBytes += parseParams(scriptAction.params);

  mapBuilder.scripts.front().actionsOnFalse.emplace_back(std::move(scriptAction));

  return totalBytes;
}

size_t MAPFile::parseScriptGroup(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint8_t buffer1 = 0;
  StringOpt stringOpt;

  MapScriptGroup scriptGroup;

  readString()
  scriptGroup.name = *stringOpt.second;

  read1()
  scriptGroup.active = buffer1 > 0;

  if (metaData.version >= 2) {
    read1()
    scriptGroup.active = buffer1 > 0;
  }

  mapBuilder.scriptGroups.emplace_back(std::move(scriptGroup));

  while (totalBytes < metaData.payloadSize) {
    // "Script"
    totalBytes += parseNextChunk(mapBuilder);
  }

  return totalBytes;
}

size_t MAPFile::parseScriptCondition(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  if (mapBuilder.scripts.empty() || mapBuilder.scripts.front().orConditions.empty()) {
    WARN_ZH("MAPFile", "No scripts/or-conditions for condition");
    return 0;
  }

  StringOpt stringOpt;
  float bufferf = 0.0;
  int32_t buffer4 = 0;
  size_t totalBytes = 0;
  size_t bytesRead = 0;

  MapScriptCondition condition;

  read4()
  auto conditionTypeOpt = Script::getScriptConditionTypeByValue(buffer4);
  if (!conditionTypeOpt) {
    return totalBytes;
  }
  condition.type = *conditionTypeOpt;

  if (metaData.version >= 4) {
    read4()
    condition.keyType = buffer4;
  }

  read4()
  auto numParams = buffer4;
  condition.params.resize(numParams);
  totalBytes += parseParams(condition.params);

  condition.version = metaData.version;

  auto& script = mapBuilder.scripts.front();
  auto& orCondition = script.orConditions.front();

  orCondition.conditions.emplace_back(std::move(condition));

  return totalBytes;
}

size_t MAPFile::parseScriptList(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  size_t totalBytes = 0;

  while (totalBytes < metaData.payloadSize) {
    // "Script", "ScriptGroup"
    totalBytes += parseNextChunk(mapBuilder);
  }

  if (totalBytes > metaData.payloadSize) {
    WARN_ZH("MAPFile", "Scripts exceed ScriptList");
  }

  return totalBytes;
}

size_t MAPFile::parseScriptOrCondition(MapBuilder& mapBuilder, const ChunkMetaData& /*metaData*/) {
  if (mapBuilder.scripts.empty()) {
    WARN_ZH("MAPFile", "No script for OrCondition");
    return 0;
  }

  auto& script = mapBuilder.scripts.front();
  script.orConditions.emplace_front();

  // "Condition"
  return parseNextChunk(mapBuilder);
}

size_t MAPFile::parseSidesList(MapBuilder& mapBuilder, const ChunkMetaData& metaData) {
  float bufferf = 0.0f;
  uint8_t buffer1 = 0;
  uint32_t buffer4 = 0;
  size_t totalBytes = 0;
  size_t bytesRead = 0;

  read4()
  auto numPlayers = buffer4;
  mapBuilder.sides.resize(numPlayers);

  StringOpt stringOpt;

  for (auto& side : mapBuilder.sides) {
    totalBytes += side.properties.parse(mapBuilder.chunkLabels, stream);

    read4()
    auto numBuildListElements = buffer4;
    side.buildInfo.resize(numBuildListElements);

    for (auto& bi : side.buildInfo) {
      readString()
      bi.buildingName = *stringOpt.second;

      readString()
      bi.templateName = *stringOpt.second;

      for (uint8_t k = 0; k < 3; ++k) {
        readf()
        bi.position[k] = bufferf;
      }
      bi.position[2] = 0.0f;

      readf()
      bi.angle = bufferf;

      read1()
      bi.prebuilt = buffer1 > 0;

      read4()
      bi.numRebuilt = buffer4;

      if (metaData.version >= 3) {
        readString()
        bi.script = *stringOpt.second;

        read4()
        bi.health = buffer4;

        read1()
        bi.whiner = buffer1 > 0;

        read1()
        bi.unsellable = buffer1 > 0;

        read1()
        bi.repairable = buffer1 > 0;
      }
    }
  }

  if (metaData.version >= 2) {
    read4()
    auto numTeams = buffer4;

    mapBuilder.teams.resize(numTeams);
    for (auto& team : mapBuilder.teams) {
      totalBytes += team.properties.parse(mapBuilder.chunkLabels, stream);
    }
  }

  // "PlayerScriptsList"
  totalBytes += parseNextChunk(mapBuilder);
  if (totalBytes > metaData.payloadSize) {
    WARN_ZH("MAPFile", "PlayerScriptsList exceeds limits");
  }

  return totalBytes;
}

size_t MAPFile::parseWorldInfo(MapBuilder& mapBuilder) {
  return mapBuilder.worldDict.parse(mapBuilder.chunkLabels, stream);
}

size_t MAPFile::parseParams(std::vector<MapScriptParam>& params) {
  float bufferf = 0.0f;
  int32_t buffer4 = 0;
  size_t bytesRead = 0;
  size_t totalBytes = 0;
  StringOpt stringOpt;

  for (auto& param : params) {
    read4()
    auto paramTypeOpt = Script::getScriptParameterTypeByValue(buffer4);
    if (!paramTypeOpt) {
      return totalBytes;
    }
    param.type = *paramTypeOpt;

    if (param.type == Script::ParameterType::COORD3D) {
      for (uint8_t j = 0; j < 3; ++j) {
        readf()
        param.positionParam[j] = bufferf;
      }
    } else {
      read4()
      param.intParam = buffer4;

      readf()
      param.floatParam = bufferf;

      readString()
      param.stringParam = *stringOpt.second;
    }
  }

  return totalBytes;
}

std::pair<size_t, std::optional<std::string>> MAPFile::parseString() {
  uint16_t strLen = 0;
  size_t totalBytes = 0;

  auto bytesRead = stream.read(reinterpret_cast<char*>(&strLen), 2);
  totalBytes += bytesRead;
  if (bytesRead != 2) {
    return std::make_pair<size_t, std::optional<std::string>>(std::move(totalBytes), {});
  }

  std::vector<char> strBuffer;
  strBuffer.resize(strLen);

  bytesRead = stream.read(strBuffer.data(), strLen);
  totalBytes += bytesRead;
  if (bytesRead != strLen) {
    return std::make_pair<size_t, std::optional<std::string>>(std::move(totalBytes), {});
  }

  return std::make_pair<size_t, std::optional<std::string>>(
      std::move(totalBytes)
    , std::string {strBuffer.data(), strLen}
  );
}

}
