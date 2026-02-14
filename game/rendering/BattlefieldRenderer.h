// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_BATTLEFIELD_RENDERER
#define H_GAME_BATTLEFIELD_RENDERER

#include "../common.h"
#include "../Battlefield.h"
#include "../gfx/TextureCache.h"
#include "ModelRenderer.h"
#include "../inis/TerrainINI.h"
#include "../inis/WaterINI.h"
#include "../vugl/vugl_context.h"

namespace ZH {

class BattlefieldRenderer {
  public:
    BattlefieldRenderer(
        Vugl::Context&
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

    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;
    ModelRenderer modelRenderer;
    const TerrainINI::Terrains& terrains;
    const WaterINI::WaterSettings& waterSettings;
    glm::mat4 projectMatrix;

    bool hasWater = false;
    std::shared_ptr<Vugl::Texture> cloudTexture;
    glm::vec3 sunlightNormal;

    std::shared_ptr<Vugl::DescriptorSet> terrainDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> terrainPipeline;
    std::shared_ptr<Vugl::UniformBuffer> terrainUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> terrainVertices;
    std::shared_ptr<Vugl::Sampler> terrainTextureSampler;
    std::vector<std::shared_ptr<Vugl::Texture>> terrainTextures;
    std::unordered_map<uint64_t, ModelRenderer::BoundingSphere> boundingSpheres;
    std::vector<DrawCheck> drawChecks;

    std::shared_ptr<Vugl::DescriptorSet> waterDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> waterPipeline;
    std::shared_ptr<Vugl::Texture> waterTexture;
    std::shared_ptr<Vugl::UniformBuffer> waterUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> waterVertices;

    bool prepareModelData(Objects::Instance&);
    bool prepareModelDrawData(Objects::Instance&);
    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
    bool prepareTreeDrawData(Objects::Instance&);
    bool prepareWaterPipeline(Vugl::RenderPass&);
    bool prepareWaterVertices();

    void renderObjectInstances(Vugl::CommandBuffer&, size_t frameIdx, bool);
    void renderObjectInstance(Objects::Instance&, Vugl::CommandBuffer&, size_t frameIdx);
    void renderTerrain(Vugl::CommandBuffer&, size_t frameIdx);
    void renderWater(Vugl::CommandBuffer&, size_t frameIdx);
};

}

#endif
