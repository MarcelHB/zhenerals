// SPDX-License-Identifier: GPL-2.0

#ifndef H_RENDERING_MODEL
#define H_RENDERING_MODEL

#include "../common.h"
#include "../gfx/FrameDisposable.h"
#include "../gfx/ModelCache.h"
#include "../gfx/TextureCache.h"
#include "../vugl/vugl_context.h"

namespace ZH {

class ModelRenderer {
  public:
    struct BoundingSphere {
      BoundingSphere() = default;
      BoundingSphere(glm::vec3 && pos, float r) : position(std::move(pos)), radius(r) {}

      glm::vec3 position {0.0f};
      float radius = 0.0f;
    };

    ModelRenderer(
        Vugl::Context&
      , GFX::TextureCache&
      , GFX::ModelCache&
    );

    void beginResourceCounting();
    void finishResourceCounting();

    bool preparePipeline(Vugl::RenderPass&);
    bool prepareModel(uint64_t id, const std::string&);

    void bindPipeline(Vugl::CommandBuffer&);
    BoundingSphere getBoundingSphere(uint64_t id) const;
    bool needsUpdate(uint64_t id, size_t frameIdx) const;
    void updateModel(
        uint64_t id
      , size_t frameIdx
      , bool newMatrices
      , const glm::mat4& mvp
      , const glm::mat4& camera
      , const glm::mat4& normal
      , const glm::vec3& sunlightNormal
    );
    bool renderModel(uint64_t id, Vugl::CommandBuffer&);
  private:
    struct ShaderData {
      alignas(16) glm::mat4 mvp;
      alignas(16) glm::vec3 sunlight;
      alignas(16) glm::mat4 normalMatrix;
    };

    struct RenderData : public GFX::FrameDisposable {
      std::vector<Vugl::DescriptorSet> descriptorSets;
      std::vector<glm::mat4> transformations;
      std::vector<Vugl::UniformBuffer> uniformBuffers;
      std::vector<ShaderData> shaderData;
      uint32_t vertexKey = 0;
      size_t numModels = 1;
      uint64_t frameIdxSet = 0;
      std::vector<glm::vec3> orderData;
      std::vector<size_t> drawOrder;
    };

    Vugl::Context& vuglContext;
    std::shared_ptr<Vugl::Pipeline> pipeline;
    GFX::ModelCache& modelCache;
    GFX::TextureCache& textureCache;
    std::unordered_map<uint64_t, std::shared_ptr<RenderData>> renderDataMap;
    std::unordered_map<uint32_t, std::shared_ptr<Vugl::ElementBuffer>> vertexData;

    void calculateBoundingCorners(const Model&, RenderData&, size_t);
};

}

#endif
