#ifndef H_GAME_MAP_RENDERER
#define H_GAME_MAP_RENDERER

#include "../Battlefield.h"
#include "../vugl/vugl_context.h"
#include "../gfx/TextureCache.h"
#include "../inis/TerrainINI.h"

namespace ZH {

class MapRenderer {
  public:
    MapRenderer(
        Vugl::Context&
      , Battlefield&
      , GFX::TextureCache&
      , const TerrainINI::Terrains& terrains
    );
    MapRenderer(const MapRenderer&) = delete;

    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
  private:
    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;
    const TerrainINI::Terrains& terrains;

    std::shared_ptr<Vugl::DescriptorSet> terrainDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> terrainPipeline;
    std::shared_ptr<Vugl::UniformBuffer> terrainUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> terrainVertices;
    std::shared_ptr<Vugl::Sampler> terrainTextureSampler;
    std::vector<std::shared_ptr<Vugl::Texture>> terrainTextures;
    std::shared_ptr<Vugl::Texture> cloudTexture;

    bool prepareTerrainPipeline(Vugl::RenderPass&, const std::vector<std::string>&);
    bool prepareTerrainVertices();
};

}

#endif
