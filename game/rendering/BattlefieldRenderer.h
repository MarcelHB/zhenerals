// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_BATTLEFIELD_RENDERER
#define H_GAME_BATTLEFIELD_RENDERER

#include "../common.h"
#include "../Config.h"
#include "../Battlefield.h"
#include "../gfx/TextureCache.h"
#include "InstanceRenderer.h"
#include "../inis/TerrainINI.h"
#include "../inis/WaterINI.h"
#include "../vugl/vugl_context.h"

namespace ZH {

class BattlefieldRenderer {
  public:
    BattlefieldRenderer(
        Vugl::Context&
      , const Config&
      , Battlefield&
      , GFX::TextureCache&
      , GFX::ModelCache&
      , const TerrainINI::Terrains& terrains
      , const WaterINI::WaterSettings& waterSettings
    );
    BattlefieldRenderer(const BattlefieldRenderer&) = delete;

    bool init(Vugl::RenderPass&);
    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
  private:
    struct DrawCheck {
      std::shared_ptr<Objects::Instance> instance;
      ModelRenderer::BoundingSphere sphere;
      float dist = 0.0f;
      bool draw = true;
    };

    struct ScorchUBData {
      alignas(16) glm::mat4 mvp;
      alignas(16) glm::mat4 uv;
      alignas(16) glm::vec3 sunlight;
    };

    // TODO frame disposability
    struct ScorchData {
      glm::vec3 position;
      float radius = 1.0f;
      glm::mat4 uv;
      std::shared_ptr<Vugl::DescriptorSet> descriptorSet;
      std::shared_ptr<Vugl::UniformBuffer> uniformBuffer;
    };

    struct ScorchOrderData {
      ScorchData *scorch = nullptr;
      bool draw = true;
      float dist = 0.0f;
    };

    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;
    InstanceRenderer instanceRenderer;
    const TerrainINI::Terrains& terrains;
    const WaterINI::WaterSettings& waterSettings;
    glm::mat4 terrainScaleMatrix;
    glm::mat4 waterScaleMatrix;

    std::vector<DrawCheck> drawChecks;

    bool hasWater = false;
    std::shared_ptr<Vugl::Texture> cloudTexture;
    glm::vec3 sunlightNormal;

    std::shared_ptr<Vugl::Pipeline> patchPipeline;
    std::shared_ptr<Vugl::ElementBuffer> patchVertices;

    std::shared_ptr<Vugl::CombinedSampler> scorchTextureSampler;
    std::unordered_map<uint64_t, ScorchData> scorchData;
    uint64_t scorchFrameIdxSet = 0;
    std::vector<ScorchOrderData> scorchOrderData;

    std::shared_ptr<Vugl::DescriptorSet> terrainDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> terrainPipeline;
    std::shared_ptr<Vugl::UniformBuffer> terrainUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> terrainVertices;
    std::shared_ptr<Vugl::Sampler> terrainTextureSampler;
    std::vector<std::shared_ptr<Vugl::Texture>> terrainTextures;

    std::shared_ptr<Vugl::DescriptorSet> waterDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> waterPipeline;
    std::shared_ptr<Vugl::Texture> waterTexture;
    std::shared_ptr<Vugl::UniformBuffer> waterUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> waterVertices;

    bool preparePatches(Vugl::RenderPass&);
    bool prepareScorches();
    bool prepareScorchData(const Battlefield::ScorchData&);
    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
    bool prepareWaterPipeline(Vugl::RenderPass&);
    bool prepareWaterVertices();

    void renderObjectInstances(Vugl::CommandBuffer&, size_t frameIdx, bool);
    void renderObjectInstance(Objects::Instance&, Vugl::CommandBuffer&, size_t frameIdx);
    void renderPatches(Vugl::CommandBuffer&, size_t frameIdx, bool);
    void renderTerrain(Vugl::CommandBuffer&, size_t frameIdx);
    void renderWater(Vugl::CommandBuffer&, size_t frameIdx);
};

}

#endif
