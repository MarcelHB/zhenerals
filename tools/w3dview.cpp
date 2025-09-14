#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>

#include "../game/Config.h"
#include "../game/gfx/Camera.h"
#include "../game/gfx/font/FontManager.h"
#include "../game/gfx/ModelCache.h"
#include "../game/gfx/TextureCache.h"
#include "../game/gfx/TextureLoader.h"
#include "../game/Logger.h"
#include "../game/rendering/ModelRenderer.h"
#include "../game/ResourceLoader.h"
#include "../game/Window.h"

class Viewer {
  private:
    ZH::Window& window;
    std::shared_ptr<ZH::GFX::ModelCache> modelCache;
    std::shared_ptr<ZH::ResourceLoader> modelLoader;
    std::shared_ptr<ZH::ModelRenderer> modelRenderer;
    std::shared_ptr<ZH::ResourceLoader> texturesResourceLoader;
    std::shared_ptr<ZH::GFX::TextureCache> textureCache;
    std::shared_ptr<ZH::GFX::TextureLoader> textureLoader;
    std::string modelName;
    std::array<glm::vec3, 2> modelExtremes;
    ZH::GFX::Camera camera;

  public:
    Viewer (ZH::Window& window) : window(window) {}

    bool init(ZH::Config& config, std::string&& modelName) {
      modelLoader =
        std::shared_ptr<ZH::ResourceLoader> {
          new ZH::ResourceLoader {{"W3DZH.big", "ZH_Generals/W3D.big"} , config.baseDir}
        };
      modelCache = std::make_shared<ZH::GFX::ModelCache>(*modelLoader);

      texturesResourceLoader =
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

      ZH::GFX::Font::FontManager fontManager {};
      textureLoader =
        std::make_shared<ZH::GFX::TextureLoader>(*texturesResourceLoader);
      textureCache =
        std::make_shared<ZH::GFX::TextureCache>(
            window.getVuglContext()
          , *textureLoader
          , fontManager // will vanish, Ok
        );

      modelRenderer =
        std::make_shared<ZH::ModelRenderer>(
            window.getVuglContext()
          , *textureCache
          , *modelCache
        );

      auto models = modelCache->getModels(modelName);
      if (!models) {
        ERROR_ZH("Main", "Unable to load model {}", modelName);
        return false;
      }

      modelExtremes = (*models)[0]->getExtremes();
      this->modelName = std::move(modelName);

      return true;
    }

    void loop() {
      std::array<VkClearValue, 2> clearColors{};
      clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
      clearColors[1].depthStencil = {1.0f, 0};

      auto& vuglContext = window.getVuglContext();

      bool updateMatrices = true;
      bool mouseDown = false;

      glm::mat4 mvp {1.0f};
      auto vp = vuglContext.getViewport();
      camera.setPerspectiveProjection(
          0.1f
        , 1000.0f
        , 90
        , vp.width * 1.0f
        , vp.height * 1.0f
      );

      auto align = [](float max, float min) -> float {
        return -min - (max - min) * 0.5f;
      };

      glm::vec3 moveVector {
          align(modelExtremes[1].x, modelExtremes[0].x)
        , align(modelExtremes[1].y, modelExtremes[0].y)
        , align(modelExtremes[1].z, modelExtremes[0].z)
      };

      camera.reposition(
          glm::vec3 {
              moveVector.x * 4.0f
            , moveVector.y * 4.0f
            , moveVector.z * 4.0f
          }
        , glm::vec3 {0.0f, 0.0f, 0.0f}
        , glm::vec3 {0.0f, 1.0f, 0.0f}
      );

      auto modelMatrix =
        glm::translate(
            glm::mat4 {1.0f}
          , moveVector
        );

      Vugl::RenderPassSetup renderPassSetup{vuglContext.getVkSurfaceFormat(), vuglContext.getVkSamplingFlag()};
      auto renderPass = vuglContext.createRenderPass(renderPassSetup);

      modelRenderer->preparePipeline(renderPass);
      modelRenderer->prepareModel(1, modelName);

      while (true) {
        while (auto eventOpt = window.getEvent()) {
          if (!eventOpt) {
            break;
          }

          auto event = eventOpt->get();

          switch (event.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
              if (event.button.button == SDL_BUTTON_LEFT) {
                mouseDown = true;
              }
              break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
              if (event.button.button == SDL_BUTTON_LEFT) {
                mouseDown = false;
              }
              break;
            case SDL_EVENT_MOUSE_MOTION:
              if (mouseDown) {
                camera.moveAround(
                    event.motion.xrel / 100.0f
                  , event.motion.yrel / 80.0f
                  , glm::vec3 {0.0f}
                );
                updateMatrices = true;
              }
              break;
            case SDL_EVENT_MOUSE_WHEEL:
              camera.zoom(event.wheel.y * 5.0f);
              updateMatrices = true;
              break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
              return;
            case SDL_EVENT_KEY_DOWN:
              if (event.key.key == SDLK_ESCAPE) {
                return;
              }
              // fallthrough
            default: break;
          }
        }

        auto& frame = vuglContext.getNextFrame();
        auto frameIndex = frame.getImageIndex();

        if (updateMatrices || modelRenderer->needsUpdate(1, frameIndex)) {
          if (updateMatrices) {
            mvp =
              camera.getProjectionMatrix()
                * camera.getCameraMatrix()
                * modelMatrix;
          }

          modelRenderer->updateModel(
              1
            , frameIndex
            , updateMatrices
            , mvp
            , glm::mat4 {1.0f}
            , camera.getDirectionVector()
          );

          updateMatrices = false;
        }

        Vugl::CommandBuffer primary {vuglContext.createCommandBuffer(frameIndex)};
        primary.beginRendering(renderPass, clearColors);
        Vugl::CommandBuffer secondary {vuglContext.createCommandBuffer(frameIndex, true)};
        secondary.beginRendering(renderPass, clearColors);

        modelRenderer->bindPipeline(secondary);
        modelRenderer->renderModel(1, secondary);

        secondary.closeRendering();
        primary.executeSecondary(secondary);
        primary.closeRendering();
        frame.submitAndPresent(primary);
      }
    }
};

int main(int argc, char **argv) {
  ZH::Logger logger;
  logger.start();

  if (argc < 2) {
    ERROR_ZH("Main", "Please supply a model name.");
    return 1;
  }

  ZH::Config config;
  ZH::Window window {config};

  if (!window.init()) {
    ERROR_ZH("Main", "Viewer setup failed, terminating.");
    return 1;
  }

  Viewer viewer {window};
  if (!viewer.init(config, std::string {argv[1]})) {
    return 1;
  }

  viewer.loop();

  return 0;
}
