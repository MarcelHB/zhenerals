// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_INSTANCE_RENDERER
#define H_GAME_INSTANCE_RENDERER

#include <set>
#include <unordered_map>

#include "common.h"
#include "Config.h"
#include "gfx/TextureCache.h"
#include "ModelRenderer.h"
#include "objects/Instance.h"
#include "vugl/vugl_context.h"

namespace ZH {

// This acts as a layer between battlefield and models
// as one instance can have multiple draw assignments

class InstanceRenderer {
  public:
    InstanceRenderer(
        Vugl::Context&
      , const Config&
      , GFX::TextureCache&
      , GFX::ModelCache&
    );

    void beginResourceCounting();
    void finishResourceCounting();
    void bindPipeline(Vugl::CommandBuffer&);

    ModelRenderer::BoundingSphere getBoundingSphere(const Objects::Instance&) const;

    bool needsUpdate(const Objects::Instance&, uint32_t frameIdx) const;
    bool prepareInstance(const Objects::Instance&);
    bool preparePipeline(Vugl::RenderPass&);
    void resetFrames(const Objects::Instance&);
    bool renderInstance(const Objects::Instance&, Vugl::CommandBuffer&);
    void updateInstance(
        const Objects::Instance&
      , uint32_t frameIdx
      , const glm::mat4& mvp
      , const glm::vec3& cameraPos
      , const glm::mat4& normal
      , const glm::vec3& sunlightNormal
    );

    // when using this, prepareInstance over default state should have been called once for now
    bool useConditionState(
        const Objects::Instance&
      , size_t drawDataIndex
      , const Objects::ConditionState&
      , size_t stateIdx
    );
  private:
    struct InstanceData {
      struct DrawState {
        uint64_t modelID = 0;
        bool hidden = false;
        std::string modelName;
        std::set<Objects::ModelCondition> applicableConditions;
      };

      ModelRenderer::BoundingSphere boundingSphere;
      std::vector<DrawState> currentDrawStates;
      uint32_t frameIdxSet = 0;
      std::vector<std::vector<uint64_t>> modelRegistry;
    };

    GFX::ModelCache& modelCache;
    ModelRenderer modelRenderer;
    std::unordered_map<uint64_t, InstanceData> drawData;
    uint64_t nextModelID = 0;

    bool prepareModelDrawData(
        const Objects::Instance&
      , const std::shared_ptr<const Objects::DrawData>&
      , InstanceData&
      , size_t drawDataIdx
    );
    bool prepareTreeDrawData(
        const Objects::Instance&
      , const std::shared_ptr<const Objects::DrawData>&
      , InstanceData&
    );

    std::optional<size_t> determineModel(
        const Objects::Instance&
      , const std::shared_ptr<const Objects::ModelDrawData>&
      , InstanceData::DrawState&
    );

    std::vector<glm::mat4> getAnimationPivots(
      const std::shared_ptr<const Objects::ModelDrawData>&
    );
};

}

#endif
