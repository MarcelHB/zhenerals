#ifndef H_W3D_FILE
#define H_W3D_FILE

#include <istream>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "../common.h"
#include "../Logging.h"

namespace ZH {

struct W3DModel {
  struct Triangle {
    glm::uvec3 vertexIndices;
    uint32_t surfaceType;
    glm::vec3 normal;
    float distance = 0.0f;
  };

  struct MaterialPass {
    std::vector<uint32_t> materialIndices;
    std::vector<uint32_t> shaderIndices;
    std::vector<uint32_t> textureIndices;
    std::vector<glm::vec2> uv;
  };

  std::string name;
  std::string containerName;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals; // EVAL
  std::vector<Triangle> triangles;
  std::vector<std::array<uint8_t, 16>> shaderValues;
  std::vector<MaterialPass> materialPasses;
  std::vector<std::string> materials;
  std::vector<std::string> textures;
  glm::vec3 boundingBoxFrom;
  glm::vec3 boundingBoxTo;
  glm::vec3 boundingSphere;
  float boundingSphereRadius = 1.0f;
  glm::mat4 transformation {1.0f};
};

class W3DFile {
  public:
    struct Pivot {
      std::string name;
      std::optional<uint32_t> parentIdx = {};
      glm::mat4 transformation {1.0f};
    };

    W3DFile(std::istream&);
    std::vector<std::shared_ptr<W3DModel>> parse();
  private:
    bool broken = false;
    std::optional<size_t> currentMaterialIdx;
    std::optional<size_t> currentTextureIdx;
    std::optional<size_t> currentMaterialPassIdx;
    std::istream& stream;

    glm::vec3 hierarchyCenter {0.0f, 0.0f, 0.0f};
    std::vector<Pivot> pivots;

    size_t parseHeader(W3DModel&);
    size_t parseMaterialInfo(W3DModel&);
    size_t parseNextChunk(std::vector<std::shared_ptr<W3DModel>>&);

    template<typename T>
    size_t parseContiguous(std::vector<T>& vector, uint32_t chunkSize) {
      if (vector.empty()) {
        return 0;
      }

      if (chunkSize != vector.size() * sizeof(T)) {
        return 0;
      }

      stream.read(reinterpret_cast<char*>(vector.data()), chunkSize);
      if (stream.gcount() != chunkSize) {
        return 0;
      }

      return chunkSize;
    }

    template<typename T>
    size_t parseContiguousDyn(std::vector<T>& vector, uint32_t chunkSize) {
      vector.resize(chunkSize / sizeof(T));
      return parseContiguous(vector, chunkSize);
    }
};

}

#endif
