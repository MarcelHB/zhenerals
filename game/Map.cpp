#include <algorithm>

#include "fmt/core.h"

#include "common.h"
#include "Logging.h"
#include "Map.h"
#include "formats/TGAFile.h"

namespace ZH {

Map::Map(MapBuilder&& builder)
  : size(builder.size)
  , heightMap(std::move(builder.heightMap))
{
  prepareTextureIndex(builder.textureClasses);
  tesselateHeightMap(builder.textureClasses, builder.tileIndices);
}

void Map::prepareTextureIndex(std::vector<TextureClass>& textureClasses) {
  std::sort(
      textureClasses.begin()
    , textureClasses.end()
    , [](const TextureClass& a, const TextureClass& b) {
        return a.firstTile < b.firstTile;
      }
  );

  texturesIndex.resize(textureClasses.size());

  for (size_t i = 0; i < textureClasses.size(); ++i) {
    texturesIndex[i] = textureClasses[i].name;
  }
}

const std::vector<uint8_t>& Map::getHeightMap() const {
  return heightMap;
}

const std::vector<std::string>& Map::getTexturesIndex() const {
  return texturesIndex;
}

const std::vector<Map::VertexData>& Map::getVertexData() const {
  return verticesAndNormals;
}

const std::vector<uint32_t>& Map::getVertexIndices() const {
  return vertexIndices;
}

Size Map::getSize() const {
  return size;
}

void Map::tesselateHeightMap(
    const std::vector<TextureClass>& textureClasses
  , const std::vector<uint16_t>& tileIndex
) {
  TRACY(ZoneScoped);

  auto numUniqueVertices = size.x * size.y * 4;
  verticesAndNormals.resize(numUniqueVertices);

  auto numVertices = size.x * size.y * 6;
  vertexIndices.resize(numVertices);

  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      size_t baseIdx = (y * size.x + x) * 4;

      for (uint8_t i = 0; i < 4; ++i) {
        auto height = getHeight(x, y, i);
        auto& vertex = verticesAndNormals[baseIdx + i];
        auto& position = vertex.position;
        auto& normal = vertex.normal;

        float xOffset = 0.0f;
        float yOffset = 0.0f;
        if (i == 1 || i == 3) {
          xOffset = 1.0f;
        }
        if (i == 2 || i == 3) {
          yOffset = 1.0f;
        }

        position.x = x + xOffset;
        position.y = height;
        position.z = y + yOffset;

        setVertexUV(vertex, textureClasses, tileIndex[y * size.x + x], xOffset, yOffset);
      }

      size_t vertexIdx = (y * size.x + x) * 6;
      vertexIndices[vertexIdx] = baseIdx;
      vertexIndices[vertexIdx + 1] = baseIdx + 1;
      vertexIndices[vertexIdx + 2] = baseIdx + 2;
      vertexIndices[vertexIdx + 3] = baseIdx + 1;
      vertexIndices[vertexIdx + 4] = baseIdx + 3;
      vertexIndices[vertexIdx + 5] = baseIdx + 2;

      auto normal =
        glm::normalize(
          glm::cross(
              verticesAndNormals[baseIdx + 2].position - verticesAndNormals[baseIdx].position
            , verticesAndNormals[baseIdx + 1].position - verticesAndNormals[baseIdx].position
          )
        );

      for (uint8_t i = 0; i < 4; ++i) {
        verticesAndNormals[baseIdx + i].normal = normal;
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
      auto h4 = getHeight(x + 1, y + 1);

      return (h1 + h2 + h3 + h4) / 4.0f;
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
      auto h4 = getHeight(x - 1, y + 1);

      return (h1 + h2 + h3 + h4) / 4.0f;
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
      auto h4 = getHeight(x + 1, y - 1);

      return (h1 + h2 + h3 + h4) / 4.0f;
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
      auto h4 = getHeight(x - 1, y - 1);

      return (h1 + h2 + h3 + h4) / 4.0f;
    }
  }
}

void Map::setVertexUV(
   Map::VertexData& vertexData
 , const std::vector<TextureClass>& textureClasses
 , uint16_t vertexIndex
 , float xOffset
 , float yOffset
) {
  size_t textureIndex = 0;
  vertexIndex /= 4;

  for (; textureIndex < textureClasses.size(); ++textureIndex) {
    if (textureClasses[textureIndex].firstTile > vertexIndex) {
      break;
    }
  }
  if (textureIndex > 0) {
    textureIndex -= 1;
  }

  auto& textureClass = textureClasses[textureIndex];
  auto textureTileIndex = vertexIndex - textureClass.firstTile;
  auto x = textureTileIndex % textureClass.width;
  auto y = textureTileIndex / textureClass.width;

  auto unit = 1.0f / textureClass.width;

  vertexData.uv.x = x * unit + xOffset * unit;
  vertexData.uv.y = y * unit + yOffset * unit;
  vertexData.textureIdx = textureIndex;
}

}
