// SPDX-License-Identifier: GPL-2.0

#include <algorithm>
#include <fstream>
#include <omp.h>

#include <glm/gtc/matrix_transform.hpp>

#include "fmt/core.h"

#include "common.h"
#include "Geometry.h"
#include "Logging.h"
#include "Map.h"
#include "formats/TGAFile.h"

namespace ZH {

Map::Map(MapBuilder& builder)
  : size(builder.size)
  , padding(builder.borderSize)
  , heightMap(std::move(builder.heightMap))
{
  worldOffsetMatrix =
    glm::translate(
      glm::mat4 {1.0},
      glm::vec3 {
          padding * GRID_TO_GAME_SCALE
        , 0.0f
        , padding * GRID_TO_GAME_SCALE
      }
    );

  prepareTextureIndex(builder.textureClasses);
  tesselateHeightMap(
      builder.textureClasses
    , builder.tileIndices
    , builder.blendTileIndices
    , builder.blendTileInfo
  );
  prepareWaters(builder.polygonTriggers);
}

void Map::prepareWaters(const std::vector<PolygonTrigger>& polygonTriggers) {
  TRACY(ZoneScoped);

  glm::mat4 axisFlip {1.0f};
  axisFlip[1][1] = 0.0f;
  axisFlip[1][2] = 1.0f;
  axisFlip[2][1] = 1.0f;
  axisFlip[2][2] = 0.0f;

  auto scaleMatrix =
    glm::scale(
        glm::mat4 {1.0f}
      , glm::vec3 {1.0f/GRID_TO_GAME_SCALE, 1.0f, 1.0f/GRID_TO_GAME_SCALE}
    );

  auto waterMatrix =
    axisFlip
    * scaleMatrix
    * worldOffsetMatrix
    * axisFlip;

  size_t numWaterTiles = 0;
  for (auto& pt : polygonTriggers) {
    if (!pt.water || pt.points.empty()) {
      continue;
    }

    if (waterState.size() == 0) {
      waterState.resize(size.x * size.y);
    }

    auto waterPoints = getPointsInPolygon(size, pt.points, waterMatrix);
    auto waterHeight = static_cast<uint16_t>(pt.points[0].z);

    for (size_t y = 0; y < size.y; ++y) {
      for (size_t x = 0; x < size.x; ++x) {
        auto idx = y * size.x + x;
        if (waterState[idx].depth == 0 && waterPoints[idx] > 0) {
          waterState[idx].depth =
            waterHeight
              - std::min(static_cast<float>(waterHeight), heightMap[idx] * TERRAIN_HEIGHT_SCALE);
          waterState[idx].surfaceHeight = waterHeight;
          if (waterState[idx].depth > 0.0f) {
            numWaterTiles++;
          }
        }
      }
    }
  }

  if (numWaterTiles == 0) {
    return;
  }

  waterVertices.resize(numWaterTiles * 6);

  auto getWaterDepth = [this](size_t x, size_t y, uint8_t i) -> float {
    float h[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    h[0] = waterState[y * size.x + x].depth;

    if (i == 0) {
      h[1] = waterState[y * size.x + (x - 1)].depth;
      h[2] = waterState[(y - 1) * size.x + (x - 1)].depth;
      h[3] = waterState[(y - 1) * size.x + x].depth;
    } else if (i == 1) {
      h[1] = waterState[(y - 1) * size.x + x].depth;
      h[2] = waterState[(y - 1) * size.x + (x + 1)].depth;
      h[3] = waterState[y * size.x + (x + 1)].depth;
    } else if (i == 3) {
      h[1] = waterState[y * size.x + (x + 1)].depth;
      h[2] = waterState[(y + 1) * size.x + (x + 1)].depth;
      h[3] = waterState[(y + 1) * size.x + x].depth;
    } else {
      h[1] = waterState[(y + 1) * size.x + x].depth;
      h[2] = waterState[(y + 1) * size.x + (x - 1)].depth;
      h[3] = waterState[y * size.x + (x - 1)].depth;
    }

    float sum = 0.0f;
    for (uint8_t j = 0; j < 4; j++) {
      if (h[j] == 0.0f) {
        return 0.0f;
      }
      sum += std::min(1.0f, h[j] / 4.0f);
    }

    return sum / 4.0f;
  };

  size_t t = 0;
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      auto idx = y * size.x + x;
      if (waterState[idx].depth == 0.0f) {
        continue;
      }

      size_t vertexIndex = t * 6;
      /*
       *  0-1/5
       *  | / |
       *  2/4-3
       */
      for (uint8_t i = 0; i < 6; ++i) {
        auto height = waterState[idx].surfaceHeight;
        auto& vertex = waterVertices[vertexIndex + i];
        auto& position = vertex.position;

        float xOffset = 0.0f, yOffset = 0.0f;
        if (i == 1 || i == 3 || i == 5) {
          xOffset = 1.0f;
        }
        if (i == 2 || i == 3 || i == 4) {
          yOffset = 1.0f;
        }

        position.x = x + xOffset;
        position.y = height;
        position.z = y + yOffset;

        // Assuming 4x4 for now, three non-opaque depth levels
        vertex.uv.x = ((x % 4) + xOffset) * 0.25f;
        vertex.uv.y = ((y % 4) + yOffset) * 0.25f;

        vertex.uvCloud.x = position.x / 128.0f;
        vertex.uvCloud.y = position.z / 128.0f;

        // softening depth levels/coast lines
        if (i == 4 || i == 5) {
          continue;
        }

        if (x == 0 || y == 0 || x == size.x - 1 || y == size.y - 1) {
          vertex.opacity = 0.0f;
        } else {
          vertex.opacity = getWaterDepth(x, y, i);

          if (i == 1) {
            waterVertices[vertexIndex + 5].opacity = vertex.opacity;
          } else if (i == 2) {
            waterVertices[vertexIndex + 4].opacity = vertex.opacity;
          }
        }
      }

      bool flip =
        (waterVertices[vertexIndex + 1].opacity == 0.0f && waterVertices[vertexIndex + 2].opacity != 0.0f)
          || (waterVertices[vertexIndex + 1].opacity != 0.0f && waterVertices[vertexIndex + 2].opacity == 0.0f);

      if (flip) {
        waterVertices[vertexIndex + 1] = waterVertices[vertexIndex + 3];
        waterVertices[vertexIndex + 4] = waterVertices[vertexIndex];
      }

      t += 1;
    }
  }
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

const std::vector<Map::WaterState>& Map::getWater() const {
  return waterState;
}

const std::vector<Map::WaterVertexData>& Map::getWaterVertices() const {
  return waterVertices;
}

const glm::mat4& Map::getWorldOffsetMatrix() const {
  return worldOffsetMatrix;
}

Size Map::getSize() const {
  return size;
}

void Map::tesselateHeightMap(
    const std::vector<TextureClass>& textureClasses
  , const std::vector<uint16_t>& tileIndex
  , const std::vector<uint16_t>& blendTileIndices
  , const std::vector<BlendTileInfo>& blendTileInfo
) {
  TRACY(ZoneScoped);

  auto numUniqueVertices = size.x * size.y * 4;
  verticesAndNormals.resize(numUniqueVertices);

  auto numVertices = size.x * size.y * 6;
  vertexIndices.resize(numVertices);

  auto statesWidthBytes = (size.x + 7) / 8;
  auto statesLength = statesWidthBytes * size.y;
  flipStates.resize(statesLength);

  auto terrainScaleMatrix =
    glm::scale(
        glm::mat4 {1.0f}
      , glm::vec3 {10.0f, Map::TERRAIN_HEIGHT_SCALE, 10.0f}
      );

  auto transformNormal = [this, &terrainScaleMatrix](size_t a, size_t b, size_t c) {
    return
      glm::normalize(
        glm::cross(
            glm::vec3 { terrainScaleMatrix * glm::vec4 {verticesAndNormals[a].position - verticesAndNormals[c].position, 1.0f}}
          , glm::vec3 { terrainScaleMatrix * glm::vec4 {verticesAndNormals[b].position - verticesAndNormals[c].position, 1.0f}}
        )
      );
  };

#pragma omp parallel num_threads(4)
  {
    TRACY(ZoneScoped);
#pragma omp for
    for (size_t y = 0; y < size.y; ++y) {
      for (size_t x = 0; x < size.x; ++x) {
        uint16_t tileTextureIndex = tileIndex[y * size.x + x];

        uint16_t mainTextureIndex = 0;
        uint16_t blendTextureIndex = 0;
        uint16_t textureClassIndex = tileTextureIndex / 4;

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
          auto pair = getHeight(x, y, i);
          auto height = pair.first;
          flip |= pair.second;
          auto& vertex = verticesAndNormals[baseIdx + i];
          auto& position = vertex.position;

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

        if (flip) {
          flipStates[y * statesWidthBytes + (x >> 3)] |= (1 << (x & 0x7));
        }

        size_t vertexIdx = (y * size.x + x) * 6;
        vertexIndices[vertexIdx] = baseIdx;
        vertexIndices[vertexIdx + 1] = baseIdx + 1;

        glm::vec3 n1, n2;
        if (flip) {
          vertexIndices[vertexIdx + 2] = baseIdx + 2;
          vertexIndices[vertexIdx + 3] = baseIdx + 1;

          n1 = transformNormal(baseIdx + 2, baseIdx + 1, baseIdx);
          n2 = transformNormal(baseIdx + 1, baseIdx + 2, baseIdx + 3);
        } else {
          vertexIndices[vertexIdx + 2] = baseIdx + 3;
          vertexIndices[vertexIdx + 3] = baseIdx;

          n1 = transformNormal(baseIdx, baseIdx + 3, baseIdx + 1);
          n2 = transformNormal(baseIdx + 3, baseIdx, baseIdx + 2);
        }
        vertexIndices[vertexIdx + 4] = baseIdx + 3;
        vertexIndices[vertexIdx + 5] = baseIdx + 2;

        glm::vec3 n3 = glm::normalize(n1 + n2);
        if (flip) {
          verticesAndNormals[baseIdx].normal = n1;
          verticesAndNormals[baseIdx + 1].normal = n3;
          verticesAndNormals[baseIdx + 2].normal = n3;
          verticesAndNormals[baseIdx + 3].normal = n2;
        } else {
          verticesAndNormals[baseIdx].normal = n3;
          verticesAndNormals[baseIdx + 1].normal = n1;
          verticesAndNormals[baseIdx + 2].normal = n2;
          verticesAndNormals[baseIdx + 3].normal = n3;
        }
      }
    }
  }

  auto tooSheered = [](const glm::vec3& a, const glm::vec3& b) -> bool {
    return std::acos(glm::dot(a, b)) > glm::radians(45.0f);
  };

  // bend the normals
  std::array<size_t, 4> indices;
  for (size_t y = 0; y < size.y; ++y) {
    for (size_t x = 0; x < size.x; ++x) {
      uint8_t normalTests = 0;

      // top
      if (y == 0 && x < size.x - 1) {
        normalTests = 2;
        indices[0] = y * size.x * 4 + x * 4 + 1;
        indices[1] = y * size.x * 4 + (x + 1) * 4;
      // bottom
      } else if (y == size.y - 1 && x < size.x - 1) {
        normalTests = 2;
        indices[0] = y * size.x * 4 + x * 4 + 3;
        indices[1] = y * size.x * 4 + (x + 1) * 4 + 2;
      // left
      } else if (x == 0 && y < size.y - 1) {
        normalTests = 2;
        indices[0] = y * size.x * 4 + x * 4 + 2;
        indices[1] = (y + 1) * size.x * 4 + x * 4;
      // right
      } else if (x == size.x - 1 && y < size.y - 1) {
        normalTests = 2;
        indices[0] = y * size.x * 4 + x * 4 + 3;
        indices[1] = (y + 1) * size.x * 4 + x * 4 + 1;
      // check: right, below, below-right
      } else if (x < size.x - 1 && y < size.y - 1) {
        normalTests = 4;
        indices[0] = y * size.x * 4 + x * 4 + 3;
        indices[1] = y * size.x * 4 + (x + 1) * 4 + 2;
        indices[2] = (y + 1) * size.x * 4 + x * 4 + 1;
        indices[3] = (y + 1) * size.x * 4 + (x + 1) * 4;
      }

      if (normalTests == 0) {
        continue;
      }

      if (normalTests == 2) {
        auto n1 = verticesAndNormals[indices[0]].normal;
        auto n2 = verticesAndNormals[indices[1]].normal;

        if (tooSheered(n1, n2)) {
          continue;
        }

        auto n = glm::normalize(n1 + n2);
        verticesAndNormals[indices[0]].normal = n;
        verticesAndNormals[indices[1]].normal = n;
      } else if (normalTests == 4) {
        uint8_t isSet = 0;
        // for each normal, check which other ones are compatible
        for (uint8_t i = 0; i < 3; ++i) {
          if (isSet & (1 << i)) {
            continue;
          }

          uint8_t accepted = (1 << i);
          glm::vec3 nI = verticesAndNormals[indices[i]].normal;
          glm::vec3 nN = nI;

          for (uint8_t j = i + 1; j < 4; ++j) {
            glm::vec3 nJ = verticesAndNormals[indices[j]].normal;

            if (!tooSheered(nI, nJ)) {
              accepted |= (1 << j);
              nN += nJ;
            }
          }

          // propagate new normal to every tile that is compatible
          nN = glm::normalize(nN);
          for (uint8_t j = i; j < 4; ++j) {
            if (accepted & (1 << j)) {
              verticesAndNormals[indices[j]].normal = nN;
            }
          }
          isSet |= accepted;
        }
      }
    }
  }
}

float Map::getCenterHeight(const glm::vec2& pos) {
  size_t x = static_cast<size_t>(pos.x / 10);
  size_t y = static_cast<size_t>(pos.y / 10);

  auto statesWidthBytes = (size.x + 7) / 8;
  auto flipped = flipStates[y * statesWidthBytes + (x >> 3)] & (1 << (x & 0x7));

  auto h0 = getHeight(x, y, 0).first;
  auto h1 = getHeight(x, y, 1).first;
  auto h2 = getHeight(x, y, 2).first;
  auto h3 = getHeight(x, y, 3).first;

  float subX = pos.x - (x * 10.0f);
  float subY = pos.y - (y * 10.0f);

  if (flipped) {
    // lower left
    if (subY > 1.0f - subX) {
      return interpolateVertexTriangle(
          glm::vec3 { 0.0f, h2,  0.0f}
        , glm::vec3 {10.0f, h3,  0.0f}
        , glm::vec3 { 0.0f, h0, 10.0f}
        , glm::vec2 {subX, subY}
      );
    // upper right
    } else {
      return interpolateVertexTriangle(
          glm::vec3 {10.0f, h3,  0.0f}
        , glm::vec3 {10.0f, h1, 10.0f}
        , glm::vec3 { 0.0f, h0, 10.0f}
        , glm::vec2 {subX, subY}
      );
    }
  } else {
    // lower right
    if (subX < subY) {
      return interpolateVertexTriangle(
          glm::vec3 { 0.0f, h2,  0.0f}
        , glm::vec3 {10.0f, h3,  0.0f}
        , glm::vec3 {10.0f, h1, 10.0f}
        , glm::vec2 {subX, subY}
      );
    // upper left
    } else {
      return interpolateVertexTriangle(
          glm::vec3 { 0.0f, h2,  0.0f}
        , glm::vec3 { 0.0f, h0, 10.0f}
        , glm::vec3 {10.0f, h1, 10.0f}
        , glm::vec2 {subX, subY}
      );
    }
  }
}

// fields
//   01
//   23
std::pair<float, bool> getHeigthFromCorners(float f0, float f1, float f2, float f3, uint8_t corner) {
  // 01
  // 11
  if (f1 - f0 > Map::CLIFF_SLOPE
      && f2 - f0 > Map::CLIFF_SLOPE
      && f3 - f0 > Map::CLIFF_SLOPE) {
    return std::make_pair((f1 + f2 + f3) / 3.0f, false);
  }

  // 00
  // 11
  if (f1 - f0 < Map::CLIFF_SLOPE
      && f2 - f0 > Map::CLIFF_SLOPE
      && f3 - f0 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1) / 2.0f, false);
  }

  // 00
  // 10
  if (f1 - f0 < Map::CLIFF_SLOPE
      && f2 - f0 < Map::CLIFF_SLOPE
      && f3 - f0 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1 + f2) / 3.0f, corner == 3);
  }

  // 10
  // 00
  if (f0 - f1 > Map::CLIFF_SLOPE
      && f0 - f2 > Map::CLIFF_SLOPE
      && f0 - f3 > Map::CLIFF_SLOPE) {
    return std::make_pair((f1 + f2 + f3) / 3.0f, false);
  }

  // 01 -- EVAL
  // 00
  if (f1 - f0 > Map::CLIFF_SLOPE
      && f1 - f2 > Map::CLIFF_SLOPE
      && f1 - f3 > Map::CLIFF_SLOPE) {
    return std::make_pair(f1, corner == 0);
  }

  // 00
  // 01
  if (f3 - f0 > Map::CLIFF_SLOPE
      && f3 - f2 > Map::CLIFF_SLOPE
      && f3 - f3 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1 + f2) / 3.0f, false);
  }

  // 10 -- EVAL
  // 11
  if (f0 - f1 > Map::CLIFF_SLOPE
      && f2 - f1 > Map::CLIFF_SLOPE
      && f3 - f1 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f2 + f3) / 3.0f, corner == 0);
  }

  // 11
  // 10
  if (f0 - f3 > Map::CLIFF_SLOPE
      && f1 - f3 > Map::CLIFF_SLOPE
      && f2 - f3 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1 + f2) / 3.0f, false);
  }

  // 11
  // 01
  if (f0 - f2 > Map::CLIFF_SLOPE
      && f1 - f2 > Map::CLIFF_SLOPE
      && f3 - f2 > Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1 + f3) / 3.0f, corner == 3);
  }

  // 01
  // 01
  if (f1 - f0 > Map::CLIFF_SLOPE
      && f3 - f0 > Map::CLIFF_SLOPE
      && f2 - f0 < Map::CLIFF_SLOPE) {
    return std::make_pair((f1 + f3) / 2.0f, false);
  }

  // 11
  // 00
  if (f0 - f2 > Map::CLIFF_SLOPE
      && f0 - f3 > Map::CLIFF_SLOPE
      && f1 - f0 < Map::CLIFF_SLOPE) {
    return std::make_pair((f0 + f1) / 2.0f, false);
  }

  // 10
  // 10
  if (f0 - f1 > Map::CLIFF_SLOPE
      && f0 - f3 > Map::CLIFF_SLOPE
      && f2 - f0 < Map::CLIFF_SLOPE) {
    return std::make_pair((f1 + f3) / 2.0f, false);
  }

  return std::make_pair((f0 + f1 + f2 + f3) / 4.0f, false);
};

std::pair<float, bool> Map::getHeight(size_t x, size_t z, uint8_t corner) {
  auto getHeight = [this](size_t x, size_t z) {
    return heightMap[z * size.x + x];
  };

  // described as seen in the world editor, lower left to upper right
  // top-right
  if (corner == 3) {
    // map corner
    if (x == size.x - 1 && z == size.y - 1) {
      return std::make_pair(getHeight(x, z), false);
    } // neighbour to right
    else if (x != size.x - 1 && z == size.y - 1) {
      auto h1 = getHeight(x + 1, z);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h1, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighbour above
    else if (x == size.x - 1 && z != size.y -1) {
      auto h1 = getHeight(x, z + 1);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h1, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighours to right and above
    else {
      auto h1 = getHeight(x + 1, z);
      auto h2 = getHeight(x, z + 1);
      auto h3 = getHeight(x, z);
      auto h4 = getHeight(x + 1, z + 1);

      return getHeigthFromCorners(h2, h4, h3, h1, corner);
    }
  } // top-left
  else if (corner == 2) {
    // map corner
    if (x == 0 && z == size.y - 1) {
      return std::make_pair(getHeight(x, z), false);
    } // neighbour to left
    else if (x != 0 && z == size.y - 1) {
      auto h1 = getHeight(x - 1, z);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h2, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighbour above
    else if (x == 0 && z != size.y - 1) {
      auto h1 = getHeight(x, z + 1);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h1, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighours to left and above
    else {
      auto h1 = getHeight(x - 1, z);
      auto h2 = getHeight(x, z + 1);
      auto h3 = getHeight(x, z);
      auto h4 = getHeight(x - 1, z + 1);

      return getHeigthFromCorners(h4, h2, h1, h3, corner);
    }
  } // bottom-right
  else if (corner == 1) {
    // map corner
    if (x == size.x - 1 && z == 0) {
      return std::make_pair(getHeight(x, z), false);
    } // neighbour to right
    else if (x != size.x - 1 && z == 0) {
      auto h1 = getHeight(x + 1, z);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h1, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighbour below
    else if (x == size.x - 1 && z != 0) {
      auto h1 = getHeight(x, z - 1);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h2, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighours to right and below
    else {
      auto h1 = getHeight(x + 1, z);
      auto h2 = getHeight(x, z - 1);
      auto h3 = getHeight(x, z);
      auto h4 = getHeight(x + 1, z - 1);

      return getHeigthFromCorners(h3, h1, h2, h4, corner);
    }
  } // bottom-left
  else {
    // map corner
    if (x == 0 && z == 0) {
      return std::make_pair(getHeight(x, z), false);
    } // neighbour to left
    else if (x != 0 && z == 0) {
      auto h1 = getHeight(x - 1, z);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h2, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighbour below
    else if (x == 0 && z != 0) {
      auto h1 = getHeight(x, z - 1);
      auto h2 = getHeight(x, z);

      if (h1 - h2 > Map::CLIFF_SLOPE || h2 - h1 > Map::CLIFF_SLOPE) {
        return std::make_pair(h2, false);
      }

      return std::make_pair((h1 + h2) / 2.0f, false);
    } // neighbour below and to left
    else {
      auto h1 = getHeight(x - 1, z);
      auto h2 = getHeight(x, z - 1);
      auto h3 = getHeight(x, z);
      auto h4 = getHeight(x - 1, z - 1);

      return getHeigthFromCorners(h1, h3, h4, h2, corner);
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

    if (bi.customBlendEdgeClass) {
      return false;
    }

    // 0-1
    // |/|
    // 2-3
    // right: /, left: \, long diagonal: diag vertices line transparent

    bool inverted = bi.inverted & 0x1;
    vertexData.uvAlpha =
         (bi.horizontal && ((inverted && (corner == 0 || corner == 2)) || (!inverted && (corner == 1 || corner == 3))))
      || (bi.vertical && ((inverted && (corner == 0 || corner == 1)) || (!inverted && (corner == 2 || corner == 3))))
      || (bi.rightDiagonal && ((inverted && (corner == 1 || (bi.longDiagonal && (corner == 0 || corner == 3))))
            || (!inverted && (corner == 3 || (bi.longDiagonal && (corner == 1 || corner == 2))))))
      || (bi.leftDiagonal && ((inverted && (corner == 0 || (bi.longDiagonal && (corner == 1 || corner == 2))))
            || (!inverted && (corner == 2 || (bi.longDiagonal && (corner == 0 || corner == 3))))))
      ? 1.0f : 0.0f;

    return (
      (bi.horizontal && (bi.inverted & 0x2))
        || (bi.vertical && (bi.inverted & 0x2))
        || (bi.rightDiagonal && !inverted)
        || (bi.leftDiagonal && inverted)
    );
  }

  return false;
}

}
