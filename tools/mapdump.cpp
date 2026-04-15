// SPDX-License-Identifier: GPL-2.0

#include <fstream>
#include <istream>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "fmt/core.h"

#include "../game/Config.h"
#include "../game/formats/Dict.h"
#include "../game/InflatingStream.h"
#include "../game/Logger.h"
#include "../game/ResourceLoader.h"

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
  stringOpt = parseString(stream); \
  totalBytes += stringOpt.first; \
  if (!stringOpt.second) { \
    return totalBytes; \
  }

struct State {
  bool dumpHeightMap = false;
  std::unordered_map<uint32_t, std::string> chunkLabels;
  uint32_t width = 0;
  uint32_t height = 0;
};

struct ChunkMetaData {
  uint32_t id;
  uint16_t version;
  uint32_t payloadSize;
};

using StringOpt = std::pair<size_t, std::optional<std::string>>;

StringOpt parseString(ZH::InflatingStream& stream) {
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

template <typename... ARGS>
void dump(uint16_t depth, fmt::format_string<ARGS...> message, ARGS&& ...args) {
  fmt::print(std::cout, "{}", std::string(depth * 2,  ' '));
  fmt::print(std::cout, message, std::forward<ARGS>(args)...);
  fmt::print(std::cout, "\n");
}

void dumpDict(const ZH::Dict& dict, uint16_t depth) {
  auto d1 = depth + 1;
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    dump(depth, "{}: ", it.key());
    switch (it.type()) {
      case ZH::Dict::DictType::BOOL:
        dump(d1, "Type: bool");
        dump(d1, "Value: {}", dict.getBool(it.key()).value_or(false));
        break;
      case ZH::Dict::DictType::INT:
        dump(d1, "Type: int");
        dump(d1, "Value: {}", dict.getInt(it.key()).value_or(0));
        break;
      case ZH::Dict::DictType::FLOAT:
        dump(d1, "Type: float");
        dump(d1, "Value: {}", dict.getFloat(it.key()).value_or(0.0f));
        break;
      case ZH::Dict::DictType::STRING: {
        dump(d1, "Type: string");
        auto opt = dict.getString(it.key());
        if (opt) {
          dump(d1, "Value: {}", opt->get());
        }
        break;
      }
      case ZH::Dict::DictType::U16STRING: {
        dump(d1, "Type: u16 string");
        break;
      }
      default:
        dump(d1, "Type: UNKNOWN");
    }
  }
}

size_t parseNextChunk(ZH::InflatingStream& stream, State& state, uint16_t depth);

size_t parseChunk(
    ZH::InflatingStream& stream
  , State& state
  , const std::string& chunkType
  , const ChunkMetaData& metaData
  , uint16_t depth
) {
  size_t totalBytes = 0;
  size_t bytesRead = 0;
  uint8_t buffer1 = 0;
  uint32_t buffer4 = 0;
  float bufferf = 0.0f;
  StringOpt stringOpt;

  auto d1 = depth + 1;
  auto d2 = d1 + 1;
  if (chunkType == "BlendTileData") {
    read4()
    dump(depth, "# indices: {}", buffer4);
    stream.seekg(4 * buffer4, std::ios::cur);
    totalBytes += 4 * buffer4;

    if (metaData.version >= 6) {
      stream.seekg(2 * buffer4, std::ios::cur);
      totalBytes += 2 * buffer4;
    }
    if (metaData.version >= 5) {
      stream.seekg(2 * buffer4, std::ios::cur);
      totalBytes += 2 * buffer4;
    }

    auto statesWidthBytes = (state.width + 7) / 8;
    auto statesLength = statesWidthBytes * state.height;

    if (metaData.version == 7) {
      auto widthBytes = (state.width + 1) / 8;
      auto cliffDataLength = widthBytes * state.height;
      stream.seekg(cliffDataLength, std::ios::cur);
      totalBytes += cliffDataLength;
    } else if (metaData.version > 7) {
      stream.seekg(statesWidthBytes * state.height, std::ios::cur);
      totalBytes += statesWidthBytes * state.height;
    }

    read4()
    dump(depth, "# bitmap tiles: {}", buffer4);
    read4()
    dump(depth, "# blend tiles: {}", buffer4);

    if (metaData.version >= 5) {
      read4()
      dump(depth, "# cliff info: {}", buffer4);
    }

    read4()
    auto numTextureClasses = buffer4;
    dump(depth, "# texture classes: {}", numTextureClasses);

    for (size_t i = 0; i < numTextureClasses; ++i) {
      dump(depth, "{}:", i);
      read4();
      dump(d1, "1st tile: {}", buffer4);
      read4();
      dump(d1, "# tiles: {}", buffer4);
      read4();
      dump(d1, "width: {}", buffer4);
      read4();
      dump(d1, "isGDF: {}", buffer4);

      readString()
      dump(d1, "name: {}", *stringOpt.second);
    }

    if (metaData.version >= 4) {
      read4();
      dump(depth, "# edge tiles: {}", buffer4);

      read4()
      auto numEdgeTextureClasses = buffer4;
      dump(depth, "# edge texture classes: {}", numEdgeTextureClasses);

      for (size_t i = 0; i < numEdgeTextureClasses; ++i) {
        dump(depth, "{}:", i);
        read4();
        dump(d1, "1st tile: {}", buffer4);
        read4();
        dump(d1, "# tiles: {}", buffer4);
        read4();
        dump(d1, "width: {}", buffer4);

        readString()
        dump(d1, "name: {}", *stringOpt.second);
      }
    }
  } else if (chunkType == "HeightMapData") {
    read4()
    state.width = buffer4;

    read4()
    state.height = buffer4;

    dump(depth, "Width: {}", state.width);
    dump(depth, "Height: {}", state.height);

    if (metaData.version >= 3) {
      read4()
      dump(depth, "Border size: {}", buffer4);
    }

    if (metaData.version >= 4) {
      read4()
      auto numBorders = buffer4;
      dump(depth, "# borders: {}", numBorders);

      for (size_t i = 0; i < numBorders; ++i) {
        dump(depth, "{}:", i);
        read4()
        dump(d1, "x: {}", buffer4);
        read4()
        dump(d1, "y: {}", buffer4);
      }
    }

    read4()
    auto numBytes = buffer4;

    if (!state.dumpHeightMap) {
      stream.seekg(numBytes, std::ios::cur);
      totalBytes += numBytes;
      return totalBytes;
    }

    std::string filename {"heightmap.ppm"};
    std::fstream f {filename, std::ios::out | std::ios::trunc};
    fmt::print(f, "P2\n");
    fmt::print(f, "{} {}\n", state.width, state.height);
    fmt::print(f, "255\n");

    for (size_t i = 0; i < numBytes; ++i) {
      read1()
      fmt::print(f, "{} ", buffer1);

      if (i % state.width == state.width - 1) {
        fmt::print(f, "\n");
      }
    }
  } else if (chunkType == "Object") {
    readf()
    dump(depth, "X: {}", bufferf);
    readf()
    dump(depth, "Y: {}", bufferf);
    readf()
    dump(depth, "Z: {}", bufferf);
    readf()
    dump(depth, "Angle: {}", bufferf);
    read4()
    dump(depth, "Flags: 0x{:x}", buffer4);
    readString()
    dump(depth, "Name: {}", *stringOpt.second);

    if (metaData.version >= 2) {
      ZH::Dict dict;
      totalBytes += dict.parse(state.chunkLabels, stream);
      dumpDict(dict, d1);
    }
  } else if (chunkType == "ObjectsList") {
    bytesRead = 0;
    while (bytesRead < metaData.payloadSize) {
      auto read = parseNextChunk(stream, state, d1);
      if (read == 0) {
        break;
      }

      bytesRead += read;
    }
    totalBytes += bytesRead;
  } else if (chunkType == "SidesList") {
    read4()
    auto numPlayers = buffer4;
    dump(depth, "# players: {}", numPlayers);

    for (size_t i = 0; i < numPlayers; ++i) {
      dump(depth, "{}:", i);
      ZH::Dict dict;
      totalBytes += dict.parse(state.chunkLabels, stream);
      dumpDict(dict, d1);

      read4()
      auto numBuildListElements = buffer4;
      dump(d1, "# num build list elems: {}", numBuildListElements);

      for (size_t j = 0; j < numBuildListElements; ++j) {
        dump(d1, "{}: ", j);
        readString()
        dump(d2, "Building: {}", *stringOpt.second);
        readString()
        dump(d2, "Template: {}", *stringOpt.second);

        readf()
        dump(d2, "X: {}", bufferf);
        readf()
        dump(d2, "Y: {}", bufferf);
        readf()
        dump(d2, "Z: {}", bufferf);
        readf()
        dump(d2, "Angle: {}", bufferf);
        read1()
        dump(d2, "Prebuilt: {}", buffer1);
        read1()
        dump(d2, "# rebuilt: {}", buffer1);

        if (metaData.version >= 3) {
          readString()
          dump(d2, "Script: {}", *stringOpt.second);
          read4()
          dump(d2, "Health: {}", buffer4);
          read1()
          dump(d2, "Whiner: {}", buffer1 > 0);
          read1()
          dump(d2, "Unsellable {}", buffer1 > 0);
          read1()
          dump(d2, "Repairable {}", buffer1 > 0);
        }
      }
    }

    if (metaData.version >= 2) {
      read4()
      auto numTeams = buffer4;
      dump(depth, "# teams: {}", numTeams);
      for (size_t i = 0; i < numTeams; ++i) {
        dump(depth, "{}:", i);
        ZH::Dict dict;
        totalBytes += dict.parse(state.chunkLabels, stream);
        dumpDict(dict, d1);
      }
    }
  } else if (chunkType == "PolygonTriggers") {
    read4()
    auto numTriggers = buffer4;
    dump(depth, "# triggers: {}", numTriggers);

    for (size_t i = 0; i < numTriggers; ++i) {
      dump(depth, "{}:", i);

      readString()
      dump(d1, "Name: {}", *stringOpt.second);

      if (metaData.version >= 4) {
        readString()
        dump(d1, "Layer: {}", *stringOpt.second);
      }

      read4()
      dump(d1, "ID: {}", buffer4);

      if (metaData.version >= 2) {
        read1()
        dump(d1, "Water: {}", buffer1 > 0);
      }

      if (metaData.version >= 3) {
        read1()
        dump(d1, "River: {}", buffer1 > 0);
        read4()
        dump(d1, "River start: {}", buffer4);
      }

      read4()
      auto numPts = buffer4;
      dump(d1, "# points: {}", numPts);

      for (size_t j = 0; j < numPts; ++j) {
        dump(d1, "{}:", j);
        read4()
        dump(d2, "X: {}", static_cast<int32_t>(buffer4));
        read4()
        dump(d2, "Y: {}", static_cast<int32_t>(buffer4));
        read4()
        dump(d2, "Z: {}", static_cast<int32_t>(buffer4));
      }
    }
  } else if (chunkType == "WorldInfo") {
    ZH::Dict dict;
    totalBytes += dict.parse(state.chunkLabels, stream);
    dumpDict(dict, depth);
  }

  return totalBytes;
}

size_t parseNextChunk(ZH::InflatingStream& stream, State& state, uint16_t depth) {
  ChunkMetaData md;

  uint32_t buffer4 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return 0;
  }
  md.id = buffer4;

  uint32_t buffer2 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer2), 2) != 2) {
    return 0;
  }
  md.version = buffer2;

  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return 0;
  }
  md.payloadSize = buffer4;

  dump(depth, "Chunk");
  dump(depth + 1, "ID: {}", md.id);

  auto typeLookup = state.chunkLabels.find(md.id);
  if (typeLookup == state.chunkLabels.cend()) {
    dump(depth + 1, "Name: UNKNOWN");
    stream.seekg(md.payloadSize, std::ios::cur);
    return md.payloadSize;
  }

  dump(depth + 1, "Name: {}", typeLookup->second);
  dump(depth + 1, "Version: {}", md.version);

  size_t bytesRead = parseChunk(stream, state, typeLookup->second, md, depth + 1);
  if (bytesRead > md.payloadSize) {
    dump(depth, "Error, size excess: {} > {}", bytesRead, md.payloadSize);
    return 0;
  } else {
    auto left = md.payloadSize - bytesRead;
    stream.seekg(left, std::ios::cur);
    bytesRead += left;
  }

  return bytesRead;
}

bool parseMap(ZH::InflatingStream& stream, State& state) {
  uint32_t buffer4 = 0;
  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return false;
  }

  if (buffer4 != 0x704D6B43) { // 'CkMp'
    return false;
  }

  if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
    return false;
  }

  std::vector<char> strBuffer;
  strBuffer.resize(255);

  dump(0, "TOC:");

  auto numToCEntries = buffer4;
  for (uint32_t i = 0; i < numToCEntries; ++i) {
    uint8_t strLength = 0;

    if (stream.read(reinterpret_cast<char*>(&strLength), 1) != 1) {
      return false;
    }

    if (stream.read(strBuffer.data(), strLength) != strLength) {
      return false;
    }

    std::string entry {strBuffer.data(), strLength};
    if (stream.read(reinterpret_cast<char*>(&buffer4), 4) != 4) {
      return false;
    }
    dump(1, "{}: {}", buffer4, entry);

    state.chunkLabels.emplace(buffer4, std::move(entry));
  }

  auto broken = false;
  while (!stream.eof() && !broken) {
    broken = parseNextChunk(stream, state, 0) == 0;
  }

  return !broken;
}

int main(int argc, char **argv) {
  ZH::Logger logger;
  logger.start();

  if (argc < 2) {
    std::cerr << "Please supply a map name." << std::endl;
    return 1;
  }

  State state;
  if (argc > 2) {
    for (int i = 2; i < argc; ++i) {
      std::string arg {argv[i]};
      if (arg == "-hm") {
        state.dumpHeightMap = true;
      }
    }
  }

  ZH::Config config;
  auto mapsLoader =
    std::shared_ptr<ZH::ResourceLoader>(
      new ZH::ResourceLoader {{"MapsZH.big", "ZH_Generals/Maps.big"} , config.baseDir}
    );

  auto path = fmt::format("maps\\{}\\{}.map", argv[1], argv[1]);
  auto lookup = mapsLoader->getFileStream(path, true);
  if (!lookup) {
    std::cerr << "Map does not exist." << std::endl;
    return 1;
  }

  auto stream = lookup->getStream();
  ZH::InflatingStream inflatingStream {stream};

  auto broken = !parseMap(inflatingStream, state);

  return broken ? 1 : 0;
}
