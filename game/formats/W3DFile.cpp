// SPDX-License-Identifier: GPL-2.0

#include <array>
#include <cstdint>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "../common.h"
#include "W3DFile.h"

namespace ZH {

W3DFile::W3DFile(std::istream& stream) : stream(stream) {}

#define read4() \
  stream.read(reinterpret_cast<char*>(&buffer4), 4); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return totalBytes; \
  }

#define readf() \
  stream.read(reinterpret_cast<char*>(&bufferf), 4); \
  bytesRead = stream.gcount(); \
  totalBytes += bytesRead; \
  if (bytesRead != 4) { \
    return totalBytes; \
  }

std::vector<std::shared_ptr<W3DModel>> W3DFile::parse() {
  TRACY(ZoneScoped);
  std::vector<std::shared_ptr<W3DModel>> models;

  while (!stream.eof() && !broken) {
    parseNextChunk(models);
  }

  if (broken) {
    return {};
  }

  return models;
}

// TODO: prevent a potential memory overkill over some non-plausible num values

size_t W3DFile::parseNextChunk(std::vector<std::shared_ptr<W3DModel>>& models) {
  uint32_t buffer4 = 0;
  size_t bytesRead = 0;
  size_t totalBytes = 0;

  read4()
  uint32_t chunkType = buffer4;
  read4()

  bool hasSubchunks = (buffer4 & (1 << 31)) > 0;
  uint32_t chunkSize = (buffer4 & 0x7FFFFFFF);

  std::shared_ptr<W3DModel> model;
  if (!models.empty()) {
    model = models.back();
  }

  if (hasSubchunks) {
    bytesRead = 0;
    switch (chunkType) {
      case 0x2B: // vertex material
        if (!currentMaterialIdx) {
          currentMaterialIdx = {0};
        } else {
          currentMaterialIdx = {1 + *currentMaterialIdx};
        }
        break;
      case 0x31: // texture (subs)
        if (!currentTextureIdx) {
          currentTextureIdx = {0};
        } else {
          currentTextureIdx = {1 + *currentTextureIdx};
        }
        break;
      case 0x38: // material pass (subs)
        if (!currentMaterialPassIdx) {
          currentMaterialPassIdx = {0};
        } else {
          currentMaterialPassIdx = {1 + *currentMaterialPassIdx};
        }
        break;
      case 0x0: // root
        model = models.emplace_back(std::make_shared<W3DModel>());
        currentMaterialIdx = std::nullopt;
        currentTextureIdx = std::nullopt;
        currentMaterialPassIdx = std::nullopt;
        break;
      case 0x2A: // vertex materials (subs)
      case 0x30: // textures (subs)
      case 0x48: // texture stage
      case 0x100: // hierarchy tree
      case 0x200: // animations
      case 0x700: // LOD ff
      case 0x702:
        break;
      default:
        WARN_ZH("W3DFile", "Unknown chunk type {}", chunkType);
        break;
    }

    while (bytesRead < chunkSize && !stream.eof() && !broken) {
      auto numBytes = parseNextChunk(models);
      bytesRead += numBytes;
      totalBytes += numBytes;
    }
  } else {
    if (model == nullptr && !(chunkType >= 0x101 && chunkType <= 0x203)) {
      broken = true;
      return totalBytes;
    }

    switch (chunkType) {
      case 0x22: // shade indices, skip
      case 0x2D: // vertex material info (?)
      case 0x3B: // vertex diffuse colors
      case 0x103:
      case 0x201:
      case 0x202:
      case 0x203:
      case 0x701:
      case 0x703:
      case 0x704:
        stream.seekg(chunkSize, std::ios::cur);
        totalBytes += chunkSize;
        break;
      case 0x2:  // vertices
        totalBytes += parseContiguous(model->vertices, chunkSize);
        break;
      case 0x3:  // normals
        totalBytes += parseContiguous(model->normals, chunkSize);
        break;
      case 0x1F: // header
        totalBytes += parseHeader(*model);
        break;
      case 0x20: // triangles
        totalBytes += parseContiguous(model->triangles, chunkSize);
        break;
      case 0x28: // material info
        totalBytes += parseMaterialInfo(*model);
        break;
      case 0x29: // shader info
        totalBytes += parseContiguous(model->shaderValues, chunkSize);
        break;
      case 0x2C: // vertex material name
        if (currentMaterialIdx) {
          if (*currentMaterialIdx >= model->materials.size()) {
            broken = true;
            return totalBytes;
          }

          auto& str = model->materials[*currentMaterialIdx];
          // trailing '\0'
          str.resize(chunkSize - 1);
          stream.read(str.data(), chunkSize - 1);
          totalBytes += stream.gcount();
          stream.seekg(1, std::ios::cur);
          totalBytes += 1;
        }
        break;
      case 0x32: // texture name
        if (currentTextureIdx) {
          if (*currentTextureIdx >= model->textures.size()) {
            broken = true;
            return totalBytes;
          }

          auto& str = model->textures[*currentTextureIdx];
          // trailing '\0'
          str.resize(chunkSize - 1);
          stream.read(str.data(), chunkSize - 1);
          bytesRead = stream.gcount();
          totalBytes += bytesRead;

          if (bytesRead != chunkSize - 1) {
            return totalBytes;
          }

          stream.seekg(1, std::ios::cur);
          totalBytes += 1;
        }
        break;
      case 0x39: // vertex material idxs
        if (currentMaterialPassIdx) {
          if (*currentMaterialPassIdx >= model->materialPasses.size()) {
            broken = true;
            return totalBytes;
          }

          totalBytes +=
            parseContiguousDyn(
                model->materialPasses[*currentMaterialPassIdx].materialIndices
              , chunkSize
            );
        }
        break;
      case 0x3A: // shader idxs
        if (currentMaterialPassIdx) {
          if (*currentMaterialPassIdx >= model->materialPasses.size()) {
            broken = true;
            return totalBytes;
          }

          totalBytes +=
            parseContiguousDyn(
                model->materialPasses[*currentMaterialPassIdx].shaderIndices
              , chunkSize
            );
        }
        break;
      case 0x49: // texture idxs
        if (currentMaterialPassIdx) {
          if (*currentMaterialPassIdx >= model->materialPasses.size()) {
            broken = true;
            return totalBytes;
          }

          totalBytes +=
            parseContiguousDyn(
                model->materialPasses[*currentMaterialPassIdx].textureIndices
              , chunkSize
            );
        }
        break;
      case 0x4A: // UV
        if (currentMaterialPassIdx) {
          if (*currentMaterialPassIdx >= model->materialPasses.size()) {
            broken = true;
            return totalBytes;
          }

          totalBytes +=
            parseContiguousDyn(
                model->materialPasses[*currentMaterialPassIdx].uv
              , chunkSize
            );
        }
        break;
      case 0x101: { // hierarchy tree header
        stream.seekg(20, std::ios::cur);
        totalBytes += 20;

        read4()
        pivots.resize(buffer4);

        float bufferf = 0.0f;
        for (size_t i = 0; i < 3; ++i) {
          readf()
          hierarchyCenter[i] = bufferf;
        }

        break;
      }
      case 0x102:
        for (auto& pivot : pivots) {
          auto& str = pivot.name;
          str.resize(16);
          stream.read(str.data(), 16);
          bytesRead = stream.gcount();
          totalBytes += bytesRead;
          if (bytesRead != 16) {
            return totalBytes;
          }

          auto nullPos = str.find('\0');
          if (nullPos != std::string::npos) {
            str.resize(nullPos);
          }

          read4()
          if (buffer4 != 0xFFFFFFFF) {
            pivot.parentIdx = {buffer4};
          }

          float bufferf = 0.0f;
          glm::vec3 translation;
          for (size_t i = 0; i < 3; ++i) {
            readf()
            translation[i] = bufferf;
          }

          stream.seekg(12, std::ios::cur);
          totalBytes += 12;

          glm::quat q;
          for (size_t i = 0; i < 4; ++i) {
            readf()
            q[i] = bufferf;
          }

          pivot.transformation =
            glm::mat4_cast(q) *
              glm::translate(pivot.transformation, translation);
        }
        break;
      default:
        WARN_ZH("W3DFile", "Unknown chunk type {}", chunkType);
        stream.seekg(chunkSize, std::ios::cur);
        totalBytes += chunkSize;
    }
  }

  if (totalBytes != chunkSize + 8) {
    WARN_ZH("W3DFile", "Error at chunk {}: {} (read) vs. {} (spec.)", chunkType, totalBytes, chunkSize + 8);
    broken = true;
  }

  return totalBytes;
}

size_t W3DFile::parseMaterialInfo(W3DModel& model) {
  uint32_t buffer4 = 0;
  size_t bytesRead = 0;
  size_t totalBytes = 0;

  read4()
  model.materialPasses.resize(buffer4);

  read4()
  if (buffer4 != model.materials.size()) {
    WARN_ZH("W3DFile", "Material info and header counter mismatch")
  }
  model.materials.resize(buffer4);

  read4()
  model.shaderValues.resize(buffer4);

  read4()
  model.textures.resize(buffer4);

  return totalBytes;
}

size_t W3DFile::parseHeader(W3DModel& model) {
  uint32_t buffer4 = 0;
  float bufferf = 0.0f;
  size_t bytesRead = 0;
  size_t totalBytes = 0;

  stream.seekg(4, std::ios::cur);
  read4()
  uint32_t flags = buffer4;

  model.name.resize(16);
  model.containerName.resize(16);

  stream.read(model.name.data(), 16);
  bytesRead = stream.gcount();
  totalBytes += bytesRead;
  if (bytesRead != 16) {
    return totalBytes;
  }
  auto nullPos = model.name.find('\0');
  if (nullPos != std::string::npos) {
    model.name.resize(nullPos);
  }

  auto lookup =
    std::find_if(pivots.cbegin(), pivots.cend(), [&model](const Pivot& p) {
      return model.name == p.name;
    });

  if (lookup != pivots.cend()) {
    model.transformation = lookup->transformation;

    auto *p = &*lookup;
    while (p->parentIdx) {
      p = &pivots[*p->parentIdx];
      auto translation = model.transformation[3];
      model.transformation[3] = glm::vec4 {0.0f, 0.0f, 0.0f, 1.0f};
      model.transformation *= p->transformation;
      model.transformation = glm::translate(model.transformation, glm::vec3 {translation});
    }
  }

  stream.read(model.containerName.data(), 16);
  bytesRead = stream.gcount();
  totalBytes += bytesRead;
  if (bytesRead != 16) {
    return totalBytes;
  }
  nullPos = model.containerName.find('\0');
  if (nullPos != std::string::npos) {
    model.containerName.resize(nullPos);
  }

  read4()
  model.triangles.resize(buffer4);

  read4()
  model.vertices.resize(buffer4);
  model.normals.resize(buffer4);

  read4()
  model.materials.resize(buffer4);

  // ignore for now, whatever it is
  stream.seekg(16, std::ios::cur);

  read4()
  uint32_t vertexChannels = buffer4;

  read4()
  uint32_t faceChannels = buffer4;

  std::array<glm::vec3, 3> vectors;
  for (uint8_t i = 0; i < 3; ++i) {
    for (uint8_t j = 0; j < 3; ++j) {
      readf()
      vectors[i][j] = bufferf;
    }
  }
  model.boundingBoxFrom = std::move(vectors[0]);
  model.boundingBoxTo = std::move(vectors[1]);
  model.boundingSphere = std::move(vectors[2]);

  readf()
  model.boundingSphereRadius = bufferf;

  return totalBytes + 20;
}

}
