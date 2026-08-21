// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_GFX_MODEL
#define H_GAME_GFX_MODEL

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "common.h"
#include "Geometry.h"
#include "formats/W3DFile.h"

namespace ZH {

struct Model {
  struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    // EVAL wasteful but alignment requirements in shader
    uint32_t pivotIdx = 0;
    uint32_t textureIdx = 0;
  };

  std::vector<VertexData> vertexData;
  std::vector<uint32_t> vertexIndices;
  std::vector<std::string> textures;
  glm::mat4 transformation {1.0f};

  glm::vec3 boundingBoxFrom;
  glm::vec3 boundingBoxTo;
  Sphere boundingSphere;

  bool backfaceCulling = true;

  static Model fromW3D(const W3DModel&);

  void attachPivots(const std::vector<uint16_t>&);
  std::array<glm::vec3, 2> getExtremes() const;
};

}

#endif
