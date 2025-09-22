#include <limits>

#include "Model.h"

namespace ZH {

Model Model::fromW3D(const W3DModel& w3d) {
  Model model;

  model.vertexData.resize(w3d.vertices.size());

  // EVAL multi pass stuff
  auto& materialPass = w3d.materialPasses[0];

  for (size_t i = 0; i < w3d.vertices.size(); ++i) {
    auto& vertexData = model.vertexData[i];
    vertexData.position = w3d.vertices[i];
    vertexData.normal = w3d.normals[i];
    vertexData.uv = materialPass.uv[i];
  }

  model.vertexIndices.resize(w3d.triangles.size() * 3);
  for (size_t i = 0; i < w3d.triangles.size(); ++i) {
    for (uint8_t j = 0; j < 3; ++j) {
      model.vertexIndices[i * 3 + j] = w3d.triangles[i].vertexIndices[j];
    }
  }

  model.textures = w3d.textures;
  model.textureIndices = materialPass.textureIndices;
  model.transformation = w3d.transformation;

  return model;
}

std::array<glm::vec3, 2> Model::getExtremes() const {
  // min, max
  std::array<glm::vec3, 2> extremes;
  extremes[0] = glm::vec3 {std::numeric_limits<float>::max()};
  extremes[1] = glm::vec3 {std::numeric_limits<float>::min()};

  for (auto& vd : vertexData) {
    auto& pos = vd.position;
    auto v = transformation * glm::vec4 {pos, 1.0f};

    for (size_t i = 0; i < 3; ++i) {
      if (v[i] < extremes[0][i]) {
        extremes[0][i] = v[i];
      }
      if (v[i] > extremes[1][i]) {
        extremes[1][i] = v[i];
      }
    }
  }

  return extremes;
}

}
