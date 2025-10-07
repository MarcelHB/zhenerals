// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_GFX_MODEL
#define H_GAME_GFX_MODEL

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "../common.h"
#include "../formats/W3DFile.h"

namespace ZH {

struct Model {
  struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
  };

  std::vector<VertexData> vertexData;
  std::vector<uint32_t> vertexIndices;
  std::vector<std::string> textures;
  std::vector<uint32_t> textureIndices;
  glm::mat4 transformation {1.0f};

  static Model fromW3D(const W3DModel&);
  std::array<glm::vec3, 2> getExtremes() const;
};

}

#endif
