#ifndef H_GAME_GFX_MODEL
#define H_GAME_GFX_MODEL

#include <vector>

#include <glm/glm.hpp>

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

  static Model fromW3D(const W3DModel&);
};

}

#endif
