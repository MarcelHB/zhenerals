// SPDX-License-Identifier: GPL-2.0

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../game/Config.h"
#include "../game/gfx/Camera.h"
#include "../game/gfx/font/FontManager.h"
#include "../game/gfx/ModelCache.h"
#include "../game/gfx/TextureCache.h"
#include "../game/gfx/TextureLoader.h"
#include "../game/Logger.h"
#include "../game/rendering/LineRenderer.h"
#include "../game/rendering/ModelRenderer.h"
#include "../game/ResourceLoader.h"
#include "../game/Window.h"

class Viewer {
  private:
    ZH::Window& window;
    std::shared_ptr<ZH::LineRenderer> lineRenderer;
    std::shared_ptr<ZH::GFX::ModelCache> modelCache;
    std::shared_ptr<ZH::ResourceLoader> modelLoader;
    std::shared_ptr<ZH::ModelRenderer> modelRenderer;
    std::shared_ptr<ZH::ResourceLoader> texturesResourceLoader;
    std::shared_ptr<ZH::GFX::TextureCache> textureCache;
    std::shared_ptr<ZH::GFX::TextureLoader> textureLoader;
    std::string modelName;
    std::array<glm::vec3, 2> modelExtremes;
    ZH::GFX::Camera camera;
    ZH::GFX::Camera sphereCamera;

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
          , config
          , *textureCache
          , *modelCache
        );

      lineRenderer = std::make_shared<ZH::LineRenderer>(window.getVuglContext());

      auto models = modelCache->getModels(modelName);
      if (!models) {
        ERROR_ZH("Main", "Unable to load model {}", modelName);
        return false;
      }

      size_t j = 0;
      this->modelName = std::move(modelName);
      for (auto model : *models) {
        auto extremes = model->getExtremes();

        if (j == 0) {
          modelExtremes = extremes;
          j += 1;
        }

        for (size_t i = 0; i < 3; ++i) {
          if (extremes[0][i] < modelExtremes[0][i]) {
            modelExtremes[0][i] = extremes[0][i];
          }
          if (extremes[1][i] > modelExtremes[1][i]) {
            modelExtremes[1][i] = extremes[1][i];
          }
        }
      }

      return true;
    }

    void putCameratAt(uint8_t pos) {
      glm::vec3 orientation {1.0f, 0.0f, 0.0f};
      if (pos == 1) {
        orientation = {-1.0f, 0.0f, 0.0f};
      } else if (pos == 2) {
        orientation = {0.0f, 0.0f, 1.0f};
      } else if (pos == 3) {
        orientation = {0.0f, 0.0f, -1.0f};
      }

      glm::vec3 camPosition = glm::vec3 {
          std::abs(modelExtremes[1].x - modelExtremes[0].x) * 1.2f
        , std::abs(modelExtremes[1].y - modelExtremes[0].y) * 1.2f
        , std::abs(modelExtremes[1].z - modelExtremes[0].z) * 1.2f
      } * orientation;

      camera.reposition(
          camPosition
        , glm::vec3 {0.0f,  0.0f, 0.0f}
        , glm::vec3 {0.0f, -1.0f, 0.0f}
      );

      sphereCamera.reposition(
          glm::vec3 {0.0f,  0.0f, glm::length(camPosition)}
        , glm::vec3 {0.0f,  0.0f, 0.0f}
        , glm::vec3 {0.0f, -1.0f, 0.0f}
      );
    };

    void loop() {
      std::array<VkClearValue, 2> clearColors{};
      clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
      clearColors[1].depthStencil = {1.0f, 0};

      auto& vuglContext = window.getVuglContext();

      bool updateMatrices = true;
      bool mouseDown = false;

      glm::mat4 mvp {1.0f};
      auto vp = vuglContext.getViewport();

      ZH::GFX::Camera::Settings settings;
      settings.near = 0.1f;
      settings.far = 1000.0f;
      settings.fovDeg = 90.0f;
      settings.width = vp.width * 1.0f;
      settings.height = vp.height * 1.0f;

      camera.setPerspectiveProjection(settings);
      sphereCamera.setPerspectiveProjection(settings);

      auto align = [](float max, float min) -> float {
        return -min - (max - min) * 0.5f;
      };

      glm::vec3 moveVector {
          align(modelExtremes[1].x, modelExtremes[0].x)
        , align(modelExtremes[1].z, modelExtremes[0].z)
        , align(modelExtremes[1].y, modelExtremes[0].y)
      };

      glm::vec3 camPosition = {
          std::abs(modelExtremes[1].x - modelExtremes[0].x) * 2.0f
        , std::abs(modelExtremes[1].z - modelExtremes[0].z) * 2.0f
        , std::abs(modelExtremes[1].y - modelExtremes[0].y) * 2.0f
      };

      camera.reposition(
          camPosition
        , glm::vec3 {0.0f,  0.0f, 0.0f}
        , glm::vec3 {0.0f, -1.0f, 0.0f}
      );

      sphereCamera.reposition(
          glm::vec3 {0.0f,  0.0f, glm::length(camPosition)}
        , glm::vec3 {0.0f,  0.0f, 0.0f}
        , glm::vec3 {0.0f, -1.0f, 0.0f}
      );

      auto modelMatrix =
        glm::translate(
            glm::mat4 {1.0f}
          , moveVector
        );

      glm::mat4 axisFlip {1.0f};
      axisFlip[1][1] = 0.0f;
      axisFlip[1][2] = 1.0f;
      axisFlip[2][1] = 1.0f;
      axisFlip[2][2] = 0.0f;

      Vugl::RenderPassSetup renderPassSetup {vuglContext.getVkSurfaceFormat(), vuglContext.getVkSamplingFlag()};
      auto renderPass = vuglContext.createRenderPass(renderPassSetup);

      modelRenderer->preparePipeline(renderPass);
      modelRenderer->prepareModel(1, modelName);

      lineRenderer->preparePipeline(renderPass);

      // RGB/XYZ axes
      auto axes =
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

      std::vector<glm::vec3> normalsData;
      std::vector<ZH::Color> normalsColor;

      auto models = modelCache->getModels(modelName);

      // Normals
      size_t i = 0;
      for (auto model : *models) {
        normalsData.resize(normalsData.size() + model->vertexData.size() * 2);
        normalsColor.resize(normalsColor.size() + model->vertexData.size() * 2);

        for (size_t j = 0; j < model->vertexData.size(); ++j, i += 2) {
          normalsData[i]      = glm::vec3 {model->transformation * glm::vec4 {model->vertexData[j].position, 1.0f}};
          normalsData[i + 1]  = glm::vec3 {model->transformation * glm::vec4 {model->vertexData[j].position + model->vertexData[j].normal, 1.0f}};
          normalsColor[i]     = normalsColor[i + 1] = ZH::Color {255, 255, 0};
        }
      }

      auto normals = lineRenderer->createLines(normalsData, normalsColor);

      // Bounding Sphere
      auto sphere = modelRenderer->getBoundingSphere(1);
      std::vector<glm::vec3> sphereData;
      sphereData.resize(64);
      std::vector<ZH::Color> sphereColors;
      sphereColors.resize(64);

      glm::vec3 up {0.0f, 1.0f, 0.0f};
      auto rot = glm::radians(360.0f / 32.0f);

      for (i = 0; i < 32; ++i) {
        if (i == 0) {
          sphereData[i * 2] = up;
        } else {
          sphereData[i * 2] = sphereData[i * 2 - 1];
        }

        up = glm::rotateZ(up, rot);
        sphereData[i * 2 + 1]  = up;
        sphereColors[i * 2]     = ZH::Color {255, 255, 255};
        sphereColors[i * 2 + 1] = ZH::Color {255, 255, 255};
      }

      auto sphereLines = lineRenderer->createLines(sphereData, sphereColors);
      auto sphereMatrix =
        glm::translate(
            glm::mat4 {1.0f}
          , glm::vec3 {sphere.position.x, sphere.position.z, sphere.position.y}
        ) *
        glm::scale(
            glm::mat4 {1.0f}
          , glm::vec3 {sphere.radius}
        );

      bool showNormals = false;
      bool showSphere = false;

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
              sphereCamera.zoom(event.wheel.y * 5.0f);
              updateMatrices = true;
              break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
              return;
            case SDL_EVENT_KEY_DOWN:
              switch (event.key.key) {
                case SDLK_ESCAPE: return;
                case SDLK_LEFT:
                  putCameratAt(0);
                  updateMatrices = true;
                  break;
                case SDLK_RIGHT:
                  putCameratAt(1);
                  updateMatrices = true;
                  break;
                case SDLK_DOWN:
                  putCameratAt(2);
                  updateMatrices = true;
                  break;
                case SDLK_UP:
                  putCameratAt(3);
                  updateMatrices = true;
                  break;
                case SDLK_N:
                  showNormals = !showNormals;
                  break;
                case SDLK_S:
                  showSphere = !showSphere;
                  break;
                default: break;
              }
              // fallthrough
            default: break;
          }
        }

        auto& frame = vuglContext.getNextFrame();
        auto frameIndex = frame.getImageIndex();

        if (updateMatrices || modelRenderer->needsUpdate(1, frameIndex)) {
          if (updateMatrices) {
            modelRenderer->resetFrames(1);
            mvp =
              camera.getProjectionMatrix()
                * camera.getCameraMatrix()
                * modelMatrix;
          }

          modelRenderer->updateModel(
              1
            , frameIndex
            , mvp
            , camera.getCameraMatrix()
            , modelMatrix
            , camera.getDirectionVector()
          );

          axes.setMatrix(camera.getProjectionMatrix() * camera.getCameraMatrix());
          normals.setMatrix(
              camera.getProjectionMatrix()
                * camera.getCameraMatrix()
                * modelMatrix
                * axisFlip
              );
          sphereLines.setMatrix(
              sphereCamera.getProjectionMatrix()
                * sphereCamera.getCameraMatrix()
                * modelMatrix
                * sphereMatrix
              );

          updateMatrices = false;
        }

        Vugl::CommandBuffer primary {vuglContext.createCommandBuffer(frameIndex)};
        primary.beginRendering(renderPass, clearColors);
        Vugl::CommandBuffer secondary {vuglContext.createCommandBuffer(frameIndex, true)};
        secondary.beginRendering(renderPass, clearColors);

        modelRenderer->bindPipeline(secondary);
        modelRenderer->renderModel(1, secondary);

        lineRenderer->bindPipeline(secondary);
        axes.writeMatrix(frameIndex);
        lineRenderer->renderLines(axes, secondary);

        if (showNormals) {
          normals.writeMatrix(frameIndex);
          lineRenderer->renderLines(normals, secondary);
        }
        if (showSphere) {
          sphereLines.writeMatrix(frameIndex);
          lineRenderer->renderLines(sphereLines, secondary);
        }

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
  ZH::Window window;

  if (!window.init(config)) {
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
