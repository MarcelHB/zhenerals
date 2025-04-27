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

  std::vector<bool> isFour {};
  isFour.resize(size.x * size.y, false);

  size_t numTwo = 0;
  size_t numFour = 0;

  // Scan where we want 2 or 4 triangles (not planar)
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      auto h1 = getHeight(x, y, 0);
      auto h2 = getHeight(x, y, 1);
      auto h3 = getHeight(x, y, 2);
      auto h4 = getHeight(x, y, 3);

      if (h1 == h2 && h2 == h3 && h3 == h4) {
        numTwo++;
      } else {
        numFour++;
        isFour[y * size.x + x] = true;
      }
    }
  }

  // four centers: n4
  // top-lefts: size.x * size.y
  // bottoms: size.x + 1
  // rights: size.y
  auto numUniqueVertices = numFour + size.x * size.y + size.x + 1 + size.y + 1;
  verticesAndNormals.resize(numUniqueVertices);

  auto numVertices = numFour * 12 + numTwo * 6;
  vertexIndices.resize(numVertices);

  size_t i4 = 0;
  // Setup of index slots
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      auto h1 = getHeight(x, y, 0);

      if (isFour[y * size.x + x]) {
        // center
        auto h2 = getHeight(x, y, 1);
        auto h3 = getHeight(x, y, 2);
        auto h4 = getHeight(x, y, 3);

        auto centerHeight = (h1 + h2 + h3 + h4) / 4.0f;
        auto centerIdx = i4++;

        auto& centerPos = verticesAndNormals[centerIdx].position;
        centerPos.x = x + 0.5f;
        centerPos.y = centerHeight;
        centerPos.z = y + 0.5f;
      }

      // top left
      auto topLeftIdx = numFour + y * size.x + x;
      auto& topLeftPos = verticesAndNormals[topLeftIdx].position;
      topLeftPos.x = x;
      topLeftPos.y = h1;
      topLeftPos.z = y;
    }
  }

  // bottom
  for (size_t x = 0; x <= size.x; ++x) {
    auto bottomIdx = numFour + size.x * size.y + x;
    auto height = getHeight(x, size.y - 1, 2);

    auto& topLeftPos = verticesAndNormals[bottomIdx].position;
    topLeftPos.x = x;
    topLeftPos.y = height;
    topLeftPos.z = size.y;
  }

  // right
  for (size_t y = 0; y < size.y; ++y) {
    auto rightIdx = numFour + size.x * size.y + size.x + 1 + y;
    auto height = getHeight(size.x - 1, y, 1);

    auto& rightPos = verticesAndNormals[rightIdx].position;
    rightPos.x = size.x;
    rightPos.y = height;
    rightPos.z = y;
  }

  /*
     four triangles:

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

  two triangles: top-right + bottom-left

   */

  size_t baseIdx = 0;
  i4 = 0;

  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      if (isFour[y * size.x + x]) {
        // top triangle
        vertexIndices[baseIdx] = i4;
        vertexIndices[baseIdx + 1] = numFour + y * size.x + x;

        if (x == size.x - 1) {
          vertexIndices[baseIdx + 2] = numFour + size.x * size.y + size.x + 1 + y;
        } else {
          vertexIndices[baseIdx + 2] = numFour + y * size.x + (x + 1);
        }

        // left triangle
        vertexIndices[baseIdx + 3] = i4;
        if (y == size.y - 1) {
          vertexIndices[baseIdx + 4] = numFour + size.x * size.y + x;
        } else {
          vertexIndices[baseIdx + 4] = numFour + (y + 1) * size.x + x;
        }
        vertexIndices[baseIdx + 5] = vertexIndices[baseIdx + 2];

        // bottom triangle
        vertexIndices[baseIdx + 6] = i4;
        if (y == size.y - 1) {
          vertexIndices[baseIdx + 7] = numFour + size.x * size.y + x + 1;
        } else {
          if (x == size.x - 1) {
            vertexIndices[baseIdx + 7] = numFour + size.x * size.y + size.x + 1 + y + 1;
          } else {
            vertexIndices[baseIdx + 7] = numFour + (y + 1) * size.x + x + 1;
          }
        }

        vertexIndices[baseIdx + 8] = vertexIndices[baseIdx + 4];

        // right triangle
        vertexIndices[baseIdx + 9] = i4;
        vertexIndices[baseIdx + 10] = vertexIndices[baseIdx + 2];
        vertexIndices[baseIdx + 11] = vertexIndices[baseIdx + 7];

        i4++;
        baseIdx += 12;
      } else {
        // top right triangle
        // top right -> bottom right -> top left
        if (x == size.x - 1) {
          vertexIndices[baseIdx] = numFour + size.x * size.y + size.x + 1 + y;
        } else {
          vertexIndices[baseIdx] = numFour + y * size.y + x + 1;
        }

        if (y == size.y - 1) {
          vertexIndices[baseIdx + 1] = numFour + size.x * size.y + x + 1;
        } else {
          if (x == size.x - 1) {
            vertexIndices[baseIdx + 1] = numFour + size.x * size.y + size.x + 1 + y + 1;
          } else {
            vertexIndices[baseIdx + 1] = numFour + (y + 1) * size.y + x + 1;
          }
        }

        vertexIndices[baseIdx + 2] = numFour + y * size.y + x;

        if (y == size.y - 1) {
          vertexIndices[baseIdx + 3] = numFour + size.x * size.x + y;
        } else {
          vertexIndices[baseIdx + 3] = numFour + (y + 1) * size.y + x;
        }

        vertexIndices[baseIdx + 4] = vertexIndices[baseIdx + 2];
        vertexIndices[baseIdx + 5] = vertexIndices[baseIdx + 1];

        baseIdx += 6;
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
    return heightMap[y * size.x + x] * 0.075f;
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
