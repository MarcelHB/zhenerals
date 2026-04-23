// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_INSTANCE_RENDERER
#define H_GAME_INSTANCE_RENDERER

#include <set>
#include <unordered_map>

#include "../common.h"
#include "../Config.h"
#include "../gfx/TextureCache.h"
#include "ModelRenderer.h"
#include "../objects/Instance.h"
#include "../vugl/vugl_context.h"

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

    bool needsUpdate(const Objects::Instance&, size_t frameIdx) const;
    bool prepareInstance(const Objects::Instance&);
    bool preparePipeline(Vugl::RenderPass&);
    void resetFrames(const Objects::Instance&);

    void updateInstance(
        const Objects::Instance&
      , size_t frameIdx
      , const glm::mat4& mvp
      , const glm::mat4& camera
      , const glm::mat4& normal
      , const glm::vec3& sunlightNormal
    );

    bool renderInstance(const Objects::Instance&, Vugl::CommandBuffer&);
  private:
    struct InstanceData {
      struct DrawState {
        uint64_t modelID = 0;
        std::set<Objects::ModelCondition> applicableConditions;
      };

      ModelRenderer::BoundingSphere boundingSphere;
      std::vector<DrawState> currentDrawStates;
      uint64_t frameIdxSet = 0;
    };

    ModelRenderer modelRenderer;
    std::unordered_map<uint64_t, InstanceData> drawData;
    uint64_t nextModelID = 0;

    bool prepareModelDrawData(
        const Objects::Instance&
      , const std::shared_ptr<const Objects::DrawData>&
      , InstanceData&
    );
    bool prepareTreeDrawData(
        const Objects::Instance&
      , const std::shared_ptr<const Objects::DrawData>&
      , InstanceData&
    );
};

}

#endif
