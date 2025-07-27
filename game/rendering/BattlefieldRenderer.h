#ifndef H_GAME_BATTLEFIELD_RENDERER
#define H_GAME_BATTLEFIELD_RENDERER

#include "../common.h"
#include "../Battlefield.h"
#include "../vugl/vugl_context.h"
#include "../gfx/ModelCache.h"
#include "../gfx/TextureCache.h"
#include "../inis/TerrainINI.h"
#include "../inis/WaterINI.h"

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

    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
  private:
    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    GFX::ModelCache& modelCache;
    Battlefield& battlefield;
    const TerrainINI::Terrains& terrains;
    const WaterINI::WaterSettings& waterSettings;

    bool hasWater = false;
    bool waterSetupAttempted = false;
    std::shared_ptr<Vugl::Texture> cloudTexture;

    std::shared_ptr<Vugl::Pipeline> modelPipline;

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

    bool prepareModelPipeline(Vugl::RenderPass&);
    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
    bool prepareWaterPipeline(Vugl::RenderPass&);
    bool prepareWaterVertices();
};

}

#endif
