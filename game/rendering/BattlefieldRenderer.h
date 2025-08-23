#ifndef H_GAME_BATTLEFIELD_RENDERER
#define H_GAME_BATTLEFIELD_RENDERER

#include "../common.h"
#include "../Battlefield.h"
#include "../vugl/vugl_context.h"
#include "../gfx/FrameDisposable.h"
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

    bool init(Vugl::RenderPass&);
    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
  private:
    struct ModelData {
      alignas(16) glm::mat4 mvp;
      alignas(16) glm::vec3 sunlight;
      alignas(16) glm::mat4 normalMatrix;
    };

    struct ModelRenderData : public GFX::FrameDisposable {
      std::shared_ptr<Vugl::DescriptorSet> descriptorSet;
      std::shared_ptr<Vugl::UniformBuffer> uniformBuffer;
      std::shared_ptr<Vugl::CombinedSampler> sampler;
      ModelData modelData;
      uint32_t vertexKey = 0;
    };

    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    GFX::ModelCache& modelCache;
    Battlefield& battlefield;
    const TerrainINI::Terrains& terrains;
    const WaterINI::WaterSettings& waterSettings;
    glm::mat4 projectMatrix;

    bool hasWater = false;
    std::shared_ptr<Vugl::Texture> cloudTexture;
    glm::vec3 sunlightNormal;

    std::unordered_map<uint64_t, std::shared_ptr<ModelRenderData>> modelRenderData;
    // TODO disposal of unused
    std::unordered_map<uint32_t, std::shared_ptr<Vugl::ElementBuffer>> vertexData;
    std::shared_ptr<Vugl::Pipeline> modelPipeline;

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

    std::unordered_map<std::string, std::shared_ptr<Vugl::ElementBuffer>> vertexCache;

    bool prepareModelPipeline(Vugl::RenderPass&);
    bool prepareModelData(Objects::Instance&);
    bool prepareModelDrawData(Objects::Instance&);
    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
    bool prepareTreeDrawData(Objects::Instance&);
    bool prepareWaterPipeline(Vugl::RenderPass&);
    bool prepareWaterVertices();

    void renderObjectInstances(Vugl::CommandBuffer&, size_t frameIdx);
    void renderObjectInstance(Objects::Instance&, Vugl::CommandBuffer&, size_t frameIdx);
    void renderTerrain(Vugl::CommandBuffer&, size_t frameIdx);
    void renderWater(Vugl::CommandBuffer&, size_t frameIdx);
};

}

#endif
