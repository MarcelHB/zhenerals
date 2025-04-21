#include "Map.h"

namespace ZH {

Map::Map(MapBuilder&& builder)
  : size(builder.size)
  , heightMap(std::move(builder.heightMap))
{}

const std::vector<uint8_t>& Map::getHeightMap() const {
  return heightMap;
}

Size Map::getSize() const {
  return size;
}

}
