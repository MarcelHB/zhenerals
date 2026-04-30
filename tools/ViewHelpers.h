// SPDX-License-Identifier: GPL-2.0

#ifndef H_TOOLS_VIEW_HELPERS
#define H_TOOLS_VIEW_HELPERS

#include "../game/Config.h"
#include "../game/gfx/ModelCache.h"
#include "../game/gfx/TextureCache.h"
#include "../game/gfx/TextureLoader.h"
#include "../game/rendering/LineRenderer.h"
#include "../game/ResourceLoader.h"

struct ViewHelpers {
  std::shared_ptr<ZH::ResourceLoader> texturesResourceLoader;
  std::shared_ptr<ZH::GFX::TextureCache> textureCache;
  std::shared_ptr<ZH::GFX::TextureLoader> textureLoader;
  std::shared_ptr<ZH::ResourceLoader> modelLoader;
  std::shared_ptr<ZH::GFX::ModelCache> modelCache;
  std::shared_ptr<ZH::LineRenderer> lineRenderer;
  ZH::LineRenderer::Lines axes;

  void preparePipeline(Vugl::RenderPass&);
  void renderAxes(uint64_t frameIdx, Vugl::CommandBuffer&);
  void setAxesMatrix(const glm::mat4&);
};

ViewHelpers createViewHelpers(const ZH::Config&, Vugl::Context&);

#endif
