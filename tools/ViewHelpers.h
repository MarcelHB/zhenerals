// SPDX-License-Identifier: GPL-2.0

#ifndef H_TOOLS_VIEW_HELPERS
#define H_TOOLS_VIEW_HELPERS

#include "Config.h"
#include "gfx/font/FontManager.h"
#include "gfx/ModelCache.h"
#include "gfx/TextureCache.h"
#include "gfx/TextureLoader.h"
#include "rendering/LineRenderer.h"
#include "ResourceLoader.h"


struct ViewHelpers {
  std::shared_ptr<ZH::ResourceLoader> texturesResourceLoader;
  std::shared_ptr<ZH::GFX::TextureCache> textureCache;
  std::shared_ptr<ZH::GFX::TextureLoader> textureLoader;
  std::shared_ptr<ZH::ResourceLoader> modelLoader;
  std::shared_ptr<ZH::GFX::ModelCache> modelCache;
  std::shared_ptr<ZH::LineRenderer> lineRenderer;
  std::shared_ptr<ZH::GFX::Font::FontManager> fontManager;

  ZH::LineRenderer::Lines axes;

  void preparePipeline(Vugl::RenderPass&);
  void renderAxes(uint32_t frameIdx, Vugl::CommandBuffer&);
  void setAxesMatrix(const glm::mat4&);
};

ViewHelpers createViewHelpers(const ZH::Config&, Vugl::Context&);

#endif
