#include <algorithm>
#include <omp.h>

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
  tesselateHeightMap(
      builder.textureClasses
    , builder.tileIndices
    , builder.flipStates
    , builder.blendTileIndices
    , builder.blendTileInfo
  );
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
  , const std::vector<uint8_t>& flipStates
  , const std::vector<uint16_t>& blendTileIndices
  , const std::vector<BlendTileInfo>& blendTileInfo
) {
  TRACY(ZoneScoped);

  auto numUniqueVertices = size.x * size.y * 4;
  verticesAndNormals.resize(numUniqueVertices);

  auto numVertices = size.x * size.y * 6;
  vertexIndices.resize(numVertices);

  auto statesWidthBytes = (size.x + 7) / 8;

#pragma omp parallel num_threads(4)
  {
    TRACY(ZoneScoped);
#pragma omp for
    for (size_t y = 0; y < size.y; ++y) {
      for (size_t x = 0; x < size.x; ++x) {
        uint16_t tileTextureIndex = tileIndex[y * size.x + x];

        uint16_t mainTextureIndex = 0;
        uint16_t blendTextureIndex = 0;
        auto textureClassIndex = tileTextureIndex / 4;

        for (; mainTextureIndex < textureClasses.size(); ++mainTextureIndex) {
          if (textureClasses[mainTextureIndex].firstTile > textureClassIndex) {
            break;
          }
        }

        if (mainTextureIndex > 0) {
          mainTextureIndex -= 1;
        }

        auto blendTileIdx = blendTileIndices[y * size.x + x];
        OptionalCRef<BlendTileInfo> blendTileInfoOpt;
        if (blendTileIdx > 0) {
          blendTileInfoOpt = {std::cref(blendTileInfo[blendTileIdx])};
        }

        if (blendTileInfoOpt) {
          textureClassIndex = blendTileInfoOpt->get().blendIdx / 4;
          for (; blendTextureIndex < textureClasses.size(); ++blendTextureIndex) {
            if (textureClasses[blendTextureIndex].firstTile > textureClassIndex) {
              break;
            }
          }

          if (blendTextureIndex > 0) {
            blendTextureIndex -= 1;
          }
        }

        bool flip = false;
        size_t baseIdx = (y * size.x + x) * 4;
        for (uint8_t i = 0; i < 4; ++i) {
          auto height = getHeight(x, y, i);
          auto& vertex = verticesAndNormals[baseIdx + i];
          auto& position = vertex.position;
          auto& normal = vertex.normal;

          float xOffset = 0.0f, yOffset = 0.0f;
          if (i == 1 || i == 3) {
            xOffset = 1.0f;
          }
          if (i == 2 || i == 3) {
            yOffset = 1.0f;
          }

          position.x = x + xOffset;
          position.y = height;
          position.z = y + yOffset;

          flip |= setVertexUV(
              vertex
            , tileTextureIndex
            , textureClasses
            , mainTextureIndex
            , blendTextureIndex
            , blendTileInfoOpt
            , i
          );
        }

        bool flipped = flip || flipStates[y * statesWidthBytes + (x >> 3)] & (1 << (x & 0x7));

        size_t vertexIdx = (y * size.x + x) * 6;
        vertexIndices[vertexIdx] = baseIdx;
        vertexIndices[vertexIdx + 1] = baseIdx + 1;
        if (!flipped) {
          vertexIndices[vertexIdx + 2] = baseIdx + 2;
          vertexIndices[vertexIdx + 3] = baseIdx + 1;
        } else {
          vertexIndices[vertexIdx + 2] = baseIdx + 3;
          vertexIndices[vertexIdx + 3] = baseIdx;
        }
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

  // bend the normals -- it doesn't work well for everything that is more an edge/corner
  // than a curve, but to be EVAL if relevant at all
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      // top
      if (y == 0 && x < size.x - 1) {
        auto&& normal1 = verticesAndNormals[y * size.x * 4 + x * 4 + 1].normal;
        auto&& normal2 = verticesAndNormals[y * size.x * 4 + (x + 1) * 4].normal;

        auto normal = glm::normalize(normal1 + normal2);
        normal1 = normal;
        normal2 = normal;
      }
      // bottom
      if (y == size.y - 1 && x < size.x - 1) {
        auto&& normal1 = verticesAndNormals[y * size.x * 4 + x * 4 + 3].normal;
        auto&& normal2 = verticesAndNormals[y * size.x * 4 + (x + 1) * 4 + 2].normal;

        auto normal = glm::normalize(normal1 + normal2);
        normal1 = normal;
        normal2 = normal;
      }

      // left
      if (x == 0 && y < size.y - 1) {
        auto&& normal1 = verticesAndNormals[y * size.x * 4 + x * 4 + 2].normal;
        auto&& normal2 = verticesAndNormals[(y + 1) * size.x * 4 + x * 4].normal;

        auto normal = glm::normalize(normal1 + normal2);
        normal1 = normal;
        normal2 = normal;
      }
      // right
      if (x == size.x - 1 && y < size.y - 1) {
        auto&& normal1 = verticesAndNormals[y * size.x * 4 + x * 4 + 3].normal;
        auto&& normal2 = verticesAndNormals[(y + 1) * size.x * 4 + x * 4 + 1].normal;

        auto normal = glm::normalize(normal1 + normal2);
        normal1 = normal;
        normal2 = normal;
      }

      if (x < size.x - 1 && y < size.y - 1) {
        auto&& normal1 = verticesAndNormals[y * size.x * 4 + x * 4 + 3].normal;
        auto&& normal2 = verticesAndNormals[y * size.x * 4 + (x + 1) * 4 + 2].normal;
        auto&& normal3 = verticesAndNormals[(y + 1) * size.x * 4 + x * 4 + 1].normal;
        auto&& normal4 = verticesAndNormals[(y + 1) * size.x * 4 + (x + 1) * 4].normal;

        auto normal = glm::normalize(normal1 + normal2 + normal3 + normal4);
        normal1 = normal;
        normal2 = normal;
        normal3 = normal;
        normal4 = normal;
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

bool Map::setVertexUV(
   Map::VertexData& vertexData
 , uint16_t tileTextureIndex
 , const std::vector<TextureClass>& textureClasses
,  uint16_t mainTextureIndex
,  uint16_t blendTextureIndex
 , OptionalCRef<BlendTileInfo> blendInfoOpt
 , uint8_t corner
) {
  auto textureClassIndex = tileTextureIndex / 4;

  auto& textureClass = textureClasses[mainTextureIndex];
  auto textureTileIndex = textureClassIndex - textureClass.firstTile;
  auto x = textureTileIndex % textureClass.width;
  auto y = textureTileIndex / textureClass.width;

  float xOffset = 0.0f, yOffset = 0.0f;
  if (corner == 1 || corner == 3) {
    xOffset = 1.0f;
  }
  if (corner == 2 || corner == 3) {
    yOffset = 1.0f;
  }

  auto unit = 1.0 / textureClass.width;
  // That only works for heigt map r/n
  float x2Offset = 0.0f, y2Offset = 0.0f;
  if (tileTextureIndex & 1) {
    x2Offset = unit * 0.5f;
  }
  if (tileTextureIndex & 2) {
    y2Offset = unit * 0.5f;
  }

  vertexData.uv.x = x2Offset + x * unit + xOffset * unit * 0.5f;
  vertexData.uv.y = y2Offset + y * unit + yOffset * unit * 0.5f;
  vertexData.uvCloud.x = vertexData.position.x / 128.0f;
  vertexData.uvCloud.y = vertexData.position.z / 128.0f;

  vertexData.textureIdx = mainTextureIndex;
  vertexData.textureIdx2 = blendTextureIndex;
  vertexData.uvAlpha = 0.0f;

  if (blendInfoOpt) {
    auto& bi = blendInfoOpt->get();
    if (bi.horizontal
          && (
            ((bi.inverted & 0x1) && (corner == 0 || corner == 2))
              || (!(bi.inverted & 0x1) && (corner == 1 || corner == 3))
            )
    ) {
      vertexData.uvAlpha = 1.0f;
      return bi.inverted & 0x2;
    } else if (bi.vertical
        && (
          ((bi.inverted & 0x1) && (corner == 0 || corner == 1))
            || (!(bi.inverted & 0x1) && (corner == 2 || corner == 3))
          )
    ) {
      vertexData.uvAlpha = 1.0f;
      return bi.inverted & 0x2;
    } else if (bi.rightDiagonal
        && (bi.inverted & 0x1)
        && (
          bi.longDiagonal && (corner == 0 || corner == 1 || corner == 3)
          || (!bi.longDiagonal && (corner == 1))
        )
    ) {
      vertexData.uvAlpha = 1.0f;
      return true;
    } else if (bi.rightDiagonal
        && !(bi.inverted & 0x1)
        && (
          bi.longDiagonal && (corner == 1 || corner == 2 || corner == 3)
          || (!bi.longDiagonal && (corner == 3))
        )
    ) {
      vertexData.uvAlpha = 1.0f;
    } else if (bi.leftDiagonal
        && (bi.inverted & 0x1)
        && (
          bi.longDiagonal && (corner == 0 || corner == 1 || corner == 2)
          || (!bi.longDiagonal && (corner == 0))
        )
    ) {
      vertexData.uvAlpha = 1.0f;
    } else if (bi.leftDiagonal
        && !(bi.inverted & 0x1)
        && (
          bi.longDiagonal && (corner == 0 || corner == 2 || corner == 3)
          || (!bi.longDiagonal && (corner == 2))
        )
    ) {
      vertexData.uvAlpha = 1.0f;
      return true;
    }
  }

  return false;
}

}
