#include "common.h"
#include "Map.h"

namespace ZH {

Map::Map(MapBuilder&& builder)
  : size(builder.size)
  , heightMap(std::move(builder.heightMap))
{
  tesselateHeightMap();
}

const std::vector<uint8_t>& Map::getHeightMap() const {
  return heightMap;
}

Size Map::getSize() const {
  return size;
}

const std::vector<Map::VertexData>& Map::getVertexData() const {
  return verticesAndNormals;
}

const std::vector<uint32_t>& Map::getVertexIndices() const {
  return vertexIndices;
}

void Map::tesselateHeightMap() {
  TRACY(ZoneScoped);

  /*
   0 x0/y0-----x1/y0 1
       | \  1 / |
       |  \  /  |
       | 2 \/ 4 |
       |   /\   |
       |  /  \  |
       | / 3  \ |
   2 x0/y1-----x1/y1 3

     with each triangle indexed:

         1-----2
          \   /
           \ /
            0
   */

  auto numUniqueVertices = size.x * size.y + (size.x + 1) * (size.y + 1);
  // centers: size.x * size.y
  // top-lefts: size.x * size.y
  // bottoms: size.x + 1
  // rights: size.y
  verticesAndNormals.resize(numUniqueVertices * 2);

  auto numVertices = size.x * size.y * 12;
  vertexIndices.resize(numVertices);

  // Setup of index slots
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      // center
      auto h1 = getHeight(x, y, 0);
      auto h2 = getHeight(x, y, 1);
      auto h3 = getHeight(x, y, 2);
      auto h4 = getHeight(x, y, 3);

      auto centerHeight = (h1 + h2 + h3 + h4) / 4.0f;
      auto centerIdx = y * size.x + x;

      auto& centerPos = verticesAndNormals[centerIdx].position;
      centerPos.x = x + 0.5f;
      centerPos.y = centerHeight;
      centerPos.z = y + 0.5f;

      // top left
      auto topLeftIdx = size.x * size.y + y * size.x + x;
      auto& topLeftPos = verticesAndNormals[topLeftIdx].position;
      topLeftPos.x = x;
      topLeftPos.y = h1;
      topLeftPos.z = y;
    }
  }

  // bottom
  for (size_t x = 0; x <= size.x; ++x) {
    auto bottomIdx = 2 * size.x * size.y + x;
    auto height = getHeight(x, size.y - 1, 2);

    auto& topLeftPos = verticesAndNormals[bottomIdx].position;
    topLeftPos.x = x;
    topLeftPos.y = height;
    topLeftPos.z = size.y;
  }

  // right
  for (size_t y = 0; y < size.y; ++y) {
    auto rightIdx = 2 * size.x * size.y + size.x + 1 + y;
    auto height = getHeight(size.x - 1, y, 1);

    auto& rightPos = verticesAndNormals[rightIdx].position;
    rightPos.x = size.x;
    rightPos.y = height;
    rightPos.z = y;
  }

  // Connecting the dots (can be merged into the top loop when tested)
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      auto baseIdx = y * size.x * 12 + x * 12;

      // top triangle
      vertexIndices[baseIdx] = y * size.x + x;
      vertexIndices[baseIdx + 1] = size.x * size.y + y * size.x + x;

      if (x == size.x - 1) {
        vertexIndices[baseIdx + 2] = 2 * size.x * size.y + size.x + 1 + y;
      } else {
        vertexIndices[baseIdx + 2] = size.x * size.y + y * size.x + (x + 1);
      }

      // left triangle
      vertexIndices[baseIdx + 3] = y * size.x + x;
      if (y == size.y - 1) {
        vertexIndices[baseIdx + 4] = 2 * size.x * size.y + x;
      } else {
        vertexIndices[baseIdx + 4] = size.x * size.y + (y + 1) * size.x + x;
      }
      vertexIndices[baseIdx + 5] = size.x * size.y + y * size.x + x;

      // bottom triangle
      vertexIndices[baseIdx + 6] = y * size.x + x;
      if (y == size.y - 1) {
        vertexIndices[baseIdx + 7] = 2 * size.x * size.y + x + 1;
      } else {
        if (x == size.x - 1) {
          vertexIndices[baseIdx + 7] = 2 * size.x * size.y + size.x + 1 + y + 1;
        } else {
          vertexIndices[baseIdx + 7] = size.x * size.y + (y + 1) * size.x + x + 1;
        }
      }

      if (y == size.y - 1) {
        vertexIndices[baseIdx + 8] = 2 * size.x * size.y + x;
      } else {
        vertexIndices[baseIdx + 8] = size.x * size.y + (y + 1) * size.x + x;
      }

      // right triangle
      vertexIndices[baseIdx + 9] = y * size.x + x;
      if (x == size.x - 1) {
        vertexIndices[baseIdx + 10] = 2 * size.x * size.y + size.x + 1 + y;
      } else {
        vertexIndices[baseIdx + 10] = size.x * size.y + y * size.x + x + 1;
      }

      if (y == size.y - 1) {
        vertexIndices[baseIdx + 11] = 2 * size.x * size.y + x + 1;
      } else {
        if (x == size.x - 1) {
          vertexIndices[baseIdx + 11] = 2 * size.x * size.y + size.x + 1 + y + 1;
        } else {
          vertexIndices[baseIdx + 11] = size.x * size.y + (y + 1) * size.x + x + 1;
        }
      }
    }
  }
}

float Map::getCenterHeight(size_t x, size_t y) {
   auto h1 = getHeight(x, y, 0);
   auto h2 = getHeight(x, y, 1);
   auto h3 = getHeight(x, y, 2);
   auto h4 = getHeight(x, y, 3);

   return (h1 + h2 + h3 + h4) / 4.0f;
}

float Map::getHeight(size_t x, size_t y, uint8_t corner) {
  auto getHeight = [this](size_t x, size_t y) {
    return heightMap[y * size.x + x] * 0.025f;
  };

  // bottom-right
  if (corner == 3) {
    // map corner
    if (x == size.x - 1 && y == size.y - 1) {
      return getHeight(x, y);
    } // neighbour to right
    else if (x != size.x - 1 && y == size.y - 1) {
      auto h1 = getHeight(x + 1, y);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighbour below
    else if (x == size.x - 1 && y != size.y -1) {
      auto h1 = getHeight(x, y + 1);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighours to right and below
    else {
      auto h1 = getHeight(x + 1, y);
      auto h2 = getHeight(x, y + 1);
      auto h3 = getHeight(x, y);

      return (h1 + h2 + h3) / 3.0f;
    }
  } // bottom-left
  else if (corner == 2) {
    // map corner
    if (x == 0 && y == size.y - 1) {
      return getHeight(x, y);
    } // neighbour to left
    else if (x != 0 && y == size.y - 1) {
      auto h1 = getHeight(x - 1, y);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighbour below
    else if (x == 0 && y != size.y - 1) {
      auto h1 = getHeight(x, y + 1);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighours to left and below
    else {
      auto h1 = getHeight(x - 1, y);
      auto h2 = getHeight(x, y + 1);
      auto h3 = getHeight(x, y);

      return (h1 + h2 + h3) / 3.0f;
    }
  } // top-right
  else if (corner == 1) {
    // map corner
    if (x == size.x - 1 && y == 0) {
      return getHeight(x, y);
    } // neighbour to right
    else if (x != size.x - 1 && y == 0) {
      auto h1 = getHeight(x + 1, y);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighbour above
    else if (x == size.x - 1 && y != 0) {
      auto h1 = getHeight(x, y - 1);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighours to right and above
    else {
      auto h1 = getHeight(x + 1, y);
      auto h2 = getHeight(x, y - 1);
      auto h3 = getHeight(x, y);

      return (h1 + h2 + h3) / 3.0f;
    }
  } // top-left
  else {
    // map corner
    if (x == 0 && y == 0) {
      return getHeight(x, y);
    } // neighbour to left
    else if (x != 0 && y == 0) {
      auto h1 = getHeight(x - 1, y);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighbour above
    else if (x == 0 && y != 0) {
      auto h1 = getHeight(x, y - 1);
      auto h2 = getHeight(x, y);

      return (h1 + h2) / 2.0f;
    } // neighbour above and to left
    else {
      auto h1 = getHeight(x - 1, y);
      auto h2 = getHeight(x, y - 1);
      auto h3 = getHeight(x, y);

      return (h1 + h2 + h3) / 3.0f;
    }
  }
}

}
