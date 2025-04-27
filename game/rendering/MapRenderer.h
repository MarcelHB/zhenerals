#ifndef H_GAME_MAP_RENDERER
#define H_GAME_MAP_RENDERER

#include "../Battlefield.h"
#include "../vugl/vugl_context.h"
#include "../gfx/TextureCache.h"

namespace ZH {

class MapRenderer {
  public:
    MapRenderer(
        Vugl::Context&
      , Battlefield&
      , GFX::TextureCache&
    );
    MapRenderer(const MapRenderer&) = delete;

    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
  private:
    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;

    std::shared_ptr<Vugl::DescriptorSet> terrainDescriptorSet;
    std::shared_ptr<Vugl::Pipeline> terrainPipeline;
    std::shared_ptr<Vugl::UniformBuffer> terrainUniformBuffer;
    std::shared_ptr<Vugl::ElementBuffer> terrainVertices;

    bool prepareTerrainPipeline(Vugl::RenderPass&);
    bool prepareTerrainVertices();
};

}

#endif
