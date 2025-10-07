// SPDX-License-Identifier: GPL-2.0

#ifndef H_MAP_FILE
#define H_MAP_FILE

#include <istream>
#include <memory>
#include <optional>

#include "../common.h"
#include "../Map.h"
#include "../InflatingStream.h"

namespace ZH {

class MAPFile {
  public:
    MAPFile(InflatingStream&);

    std::shared_ptr<MapBuilder> parseMap();
  private:
    struct ChunkMetaData {
      uint32_t id;
      uint16_t version;
      uint32_t payloadSize;
    };

    InflatingStream& stream;

    std::optional<ChunkMetaData> getChunkMetaData();
    size_t parseChunk(MapBuilder&, const std::string&, const ChunkMetaData&);
    size_t parseNextChunk(MapBuilder&);

    size_t parseBlendTiles(MapBuilder&, const ChunkMetaData&);
    size_t parseGlobalLighting(MapBuilder&, const ChunkMetaData&);
    size_t parseHeightMap(MapBuilder&, const ChunkMetaData&);
    size_t parseObject(MapBuilder&, const ChunkMetaData&);
    size_t parseObjectsList(MapBuilder&, const ChunkMetaData&);
    size_t parsePolygonTriggers(MapBuilder&, const ChunkMetaData&);
    size_t parsePlayerScriptsList(MapBuilder&, const ChunkMetaData&);
    size_t parseScript(MapBuilder&, const ChunkMetaData&);
    size_t parseScriptAction(MapBuilder&, const ChunkMetaData&);
    size_t parseScriptActionFalse(MapBuilder&, const ChunkMetaData&);
    size_t parseScriptCondition(MapBuilder& mapBuilder, const ChunkMetaData&);
    size_t parseScriptGroup(MapBuilder&, const ChunkMetaData&);
    size_t parseScriptList(MapBuilder&, const ChunkMetaData&);
    size_t parseScriptOrCondition(MapBuilder& mapBuilder, const ChunkMetaData&);
    size_t parseSidesList(MapBuilder&, const ChunkMetaData&);
    size_t parseWorldInfo(MapBuilder&);

    size_t parseParams(std::vector<MapScriptParam>& params);
    std::pair<size_t, std::optional<std::string>> parseString();
};

}

#endif
