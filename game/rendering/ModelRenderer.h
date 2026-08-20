// SPDX-License-Identifier: GPL-2.0

#ifndef H_RENDERING_MODEL
#define H_RENDERING_MODEL

#include "common.h"
#include "Config.h"
#include "Geometry.h"
#include "gfx/FrameDisposable.h"
#include "gfx/ModelCache.h"
#include "gfx/TextureCache.h"
#include "vugl/vugl_context.h"

namespace ZH {

class ModelRenderer {
  public:
    ModelRenderer(
        Vugl::Context&
      , const Config& config
      , GFX::TextureCache&
      , GFX::ModelCache&
    );

    void beginResourceCounting();
    void finishResourceCounting();

    bool preparePipeline(Vugl::RenderPass&);
    bool prepareModel(
        uint64_t id
      , const std::string& modelName
      , std::vector<glm::mat4>&& pivotMatrices = {glm::mat4 {1.0f}}
    );

    void bindPipeline(Vugl::CommandBuffer&);
    Sphere getBoundingSphere(uint64_t id) const;

    void updateModel(
        uint64_t id
      , uint32_t frameIdx
      , const glm::mat4& mvp
      , const glm::vec3& cameraPos
      , const glm::mat4& normal
      , const glm::vec3& sunlightNormal
    );
    bool renderModel(uint64_t id, Vugl::CommandBuffer&);
  private:
    struct ShaderData {
      alignas(16) glm::mat4 mvp;
      alignas(16) glm::vec3 sunlight;
      alignas(16) glm::mat4 normalMatrix;
      alignas(16) uint32_t pivotEnabler = 0;
    };

    struct PivotData {
      alignas(16) glm::mat4 transformation;
    };

    using OrderPair = std::pair<size_t, float>;
    struct RenderData : public GFX::FrameDisposable {
      std::vector<Vugl::DescriptorSet> descriptorSets;
      std::shared_ptr<Vugl::UniformBuffer> pivotBuffer;
      std::vector<glm::mat4> transformations;
      std::vector<Vugl::UniformBuffer> uniformBuffers;
      std::vector<ShaderData> shaderData;
      std::vector<PivotData> pivotData;
      uint32_t vertexKey = 0;
      size_t numModels = 1;
      std::vector<OrderPair> drawOrder;
      std::vector<bool> backfaceCulling;
      std::vector<Sphere> boundingSpheres;
      Sphere boundingSphere;
    };

    Vugl::Context& vuglContext;
    const Config& config;
    std::shared_ptr<Vugl::Pipeline> pipeline;
    GFX::ModelCache& modelCache;
    GFX::TextureCache& textureCache;
    std::unordered_map<uint64_t, RenderData> renderDataMap;
    std::unordered_map<uint32_t, std::shared_ptr<Vugl::ElementBuffer>> vertexData;

    void createPivotBuffer(RenderData&, std::vector<glm::mat4>&&);
};

}

#endif
