// SPDX-License-Identifier: GPL-2.0

#include <limits>

#include "Model.h"

namespace ZH {

Model Model::fromW3D(const W3DModel& w3d) {
  Model model;

  model.vertexData.resize(w3d.vertices.size());

  // EVAL multi pass stuff
  auto& materialPass = w3d.materialPasses.back();
  // EVAL uvs of some models when not here
  bool hasUVs = !materialPass.uv.empty();

  for (size_t i = 0; i < w3d.vertices.size(); ++i) {
    auto& vertexData = model.vertexData[i];
    vertexData.position = w3d.vertices[i];
    vertexData.normal = w3d.normals[i];
    if (hasUVs) {
      vertexData.uv = materialPass.uv[i];
    }
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

  model.boundingBoxFrom = w3d.boundingBoxFrom;
  model.boundingBoxTo = w3d.boundingBoxTo;
  model.boundingSphere = w3d.boundingSphere;
  model.boundingSphereRadius = w3d.boundingSphereRadius;

  if (w3d.flags & 0x2000) {
    model.backfaceCulling = false;
  }

  return model;
}

std::array<glm::vec3, 2> Model::getExtremes() const {
  // min, max
  std::array<glm::vec3, 2> extremes;

  auto pMin = transformation * glm::vec4 {boundingBoxFrom, 1.0f};
  auto pMax = transformation * glm::vec4 {boundingBoxTo, 1.0f};

  extremes[0] = glm::vec3 {pMin};
  extremes[1] = glm::vec3 {pMax};

  return extremes;
}

}
