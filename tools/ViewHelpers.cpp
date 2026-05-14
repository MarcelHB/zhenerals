// SPDX-License-Identifier: GPL-2.0

#include "ViewHelpers.h"

ViewHelpers createViewHelpers(const ZH::Config& config, Vugl::Context& vuglContext) {
  ViewHelpers helpers;

  helpers.modelLoader =
    std::shared_ptr<ZH::ResourceLoader> {
      new ZH::ResourceLoader {{"W3DZH.big", "ZH_Generals/W3D.big"} , config.baseDir}
    };
  helpers.modelCache = std::make_shared<ZH::GFX::ModelCache>(*helpers.modelLoader);

  helpers.texturesResourceLoader =
    std::shared_ptr<ZH::ResourceLoader> {
      new ZH::ResourceLoader {{
          "TexturesZH.big"
        , "TerrainZH.big"
        , "MapsZH.big"
        , "EnglishZH.big"
        , "ZH_Generals/Textures.big"
        , "ZH_Generals/Terrain.big"
        , "ZH_Generals/Maps.big"
        , "ZH_Generals/English.big"
      }
      , config.baseDir
    }
  };

  helpers.fontManager = std::make_shared<ZH::GFX::Font::FontManager>();
  helpers.textureLoader =
    std::make_shared<ZH::GFX::TextureLoader>(*helpers.texturesResourceLoader);
  helpers.textureCache =
    std::make_shared<ZH::GFX::TextureCache>(
        vuglContext
      , *helpers.textureLoader
      , *helpers.fontManager
    );

  helpers.lineRenderer = std::make_shared<ZH::LineRenderer>(vuglContext);

  return helpers;
}

void ViewHelpers::preparePipeline(Vugl::RenderPass& renderPass) {
  lineRenderer->preparePipeline(renderPass);
  // RGB/XYZ axes
  axes =
    lineRenderer->createLines(
        {
            {0.0f, 0.0f, 0.0f}
          , {1.0f, 0.0f, 0.0f}
          , {0.0f, 0.0f, 0.0f}
          , {0.0f, 1.0f, 0.0f}
          , {0.0f, 0.0f, 0.0f}
          , {0.0f, 0.0f, 1.0f}
        }
      , {
            {255, 0, 0}
          , {255, 0, 0}
          , {0, 255, 0}
          , {0, 255, 0}
          , {0, 0, 255}
          , {0, 0, 255}
        }
    );
}

void ViewHelpers::setAxesMatrix(const glm::mat4& matrix) {
  axes.setMatrix(matrix);
}

void ViewHelpers::renderAxes(uint32_t frameIdx, Vugl::CommandBuffer& commandBuffer) {
  axes.writeMatrix(frameIdx);
  lineRenderer->renderLines(axes, commandBuffer);
}
