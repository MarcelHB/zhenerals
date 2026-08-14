// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_BATTLEFIELD_RENDERER
#define H_GAME_BATTLEFIELD_RENDERER

#include "common.h"
#include "Config.h"
#include "Battlefield.h"
#include "gfx/TextureCache.h"
#include "InstanceRenderer.h"
#include "PatchRenderer.h"
#include "inis/RoadsBridgesINI.h"
#include "inis/TerrainINI.h"
#include "inis/WaterINI.h"
#include "vugl/vugl_context.h"

namespace ZH {

class BattlefieldRenderer {
  public:
    BattlefieldRenderer(
        Vugl::Context&
      , const Config&
      , Battlefield&
      , GFX::TextureCache&
      , GFX::ModelCache&
      , std::shared_ptr<ResourceLoader> iniResourceLoader
    );
    BattlefieldRenderer(const BattlefieldRenderer&) = delete;

    bool init(Vugl::RenderPass&);
    void createRenderList(Vugl::CommandBuffer&, uint32_t, Vugl::RenderPass&);
  private:
    struct DrawCheck {
      std::shared_ptr<Objects::Instance> instance;
      ModelRenderer::BoundingSphere sphere;
      float dist = 0.0f;
      bool draw = true;
    };

    Vugl::Context& vuglContext;
    const Config& config;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;
    InstanceRenderer instanceRenderer;
    std::shared_ptr<PatchRenderer> patchRenderer;
    std::shared_ptr<ResourceLoader> iniResourceLoader;

    TerrainINI::Terrains terrains;
    WaterINI::WaterSettings waterSettings;
    RoadsBridgesINI::Roads roads;
    RoadsBridgesINI::Bridges bridges;

    glm::mat4 terrainScaleMatrix;
    glm::mat4 waterScaleMatrix;

    std::vector<DrawCheck> drawChecks;

    bool hasWater = false;
    std::shared_ptr<Vugl::Texture> cloudTexture;

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

    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
    bool prepareWaterPipeline(Vugl::RenderPass&);
    bool prepareWaterVertices();

    void renderObjectInstances(Vugl::CommandBuffer&, uint32_t frameIdx, bool);
    void renderObjectInstance(Objects::Instance&, Vugl::CommandBuffer&, uint32_t frameIdx);
    void renderTerrain(Vugl::CommandBuffer&, uint32_t frameIdx);
    void renderWater(Vugl::CommandBuffer&, uint32_t frameIdx);
};

}

#endif
