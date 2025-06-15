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

  return model;
}

}
