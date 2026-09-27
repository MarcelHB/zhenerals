// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_PATCH_RENDERER
#define H_GAME_PATCH_RENDERER

#include "Battlefield.h"
#include "Config.h"
#include "gfx/FrameDisposable.h"
#include "gfx/TextureCache.h"
#include "inis/RoadsBridgesINI.h"
#include "RoadResolution.h"
#include "vugl/vugl_context.h"

namespace ZH {

class PatchRenderer {
  public:
    PatchRenderer(
        Vugl::Context&
      , const Config& config
      , Battlefield&
      , GFX::TextureCache&
      , const RoadsBridgesINI::Roads& roads
    );

    PatchRenderer (const PatchRenderer&) = delete;

    void beginResourceCounting();
    void finishResourceCounting();

    bool init(Vugl::RenderPass&);
    void renderPatches(Vugl::CommandBuffer&, uint32_t frameIdx, bool);
    void renderRoads(Vugl::CommandBuffer&, uint32_t frameIdx, bool);
    void renderScorches(Vugl::CommandBuffer&, uint32_t frameIdx, bool);
  private:
    Vugl::Context& vuglContext;
    const Config& config;
    GFX::TextureCache& textureCache;
    Battlefield& battlefield;
    RoadResolution roadResolution;

    const RoadsBridgesINI::Roads& roadsINI;

    struct RoadData : public GFX::FrameDisposable {
      uint32_t textureKey = 0;
      glm::mat4 mvp;
      glm::mat4 uv;
      std::shared_ptr<Vugl::DescriptorSet> descriptorSet;
      std::shared_ptr<Vugl::UniformBuffer> uniformBuffer;
      bool prepared = false;
      uint64_t frameIdxSet = 0;
    };

    struct ScorchUBData {
      alignas(16) glm::mat4 mvp;
      alignas(16) glm::mat4 uv;
      alignas(16) glm::vec3 sunlight;
      alignas(4)  float heightOffset;
    };

    struct ScorchData : public GFX::FrameDisposable {
      glm::vec3 position;
      float radius = 1.0f;
      glm::mat4 uv;
      std::shared_ptr<Vugl::DescriptorSet> descriptorSet;
      std::shared_ptr<Vugl::UniformBuffer> uniformBuffer;
      std::shared_ptr<Vugl::ElementBuffer> vertices;
    };

    struct ScorchOrderData {
      ScorchData *scorch = nullptr;
      bool draw = true;
      float dist = 0.0f;
      uint64_t frameIdxSet = 0;
    };

    std::shared_ptr<Vugl::Pipeline> patchPipeline;

    std::vector<RoadData> roadData;
    std::shared_ptr<Vugl::ElementBuffer> roadDefaultVertices;
    std::unordered_map<
        uint32_t
      , std::shared_ptr<Vugl::CombinedSampler>
    > roadTextures;

    std::shared_ptr<Vugl::CombinedSampler> scorchTextureSampler;
    std::unordered_map<uint64_t, ScorchData> scorchData;
    std::vector<ScorchOrderData> scorchOrderData;

    bool preparePatches(Vugl::RenderPass&);
    bool prepareRoads();
    void prepareRoadData(RoadData&);
    bool prepareScorches();
    void prepareScorchData(const Battlefield::ScorchData&);
};

}

#endif
