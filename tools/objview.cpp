// SPDX-License-Identifier: GPL-2.0

#include "../game/Config.h"
#include "../game/gfx/Camera.h"
#include "../game/Logger.h"
#include "../game/Map.h"
#include "../game/ObjectLoader.h"
#include "../game/objects/InstanceFactory.h"
#include "../game/rendering/InstanceRenderer.h"
#include "../game/Window.h"
#include "ViewHelpers.h"

class ObjectViewer {
  private:
    ZH::Window& window;
    ViewHelpers viewHelpers;
    std::string objectName;

    std::shared_ptr<ZH::ResourceLoader> iniLoader;
    std::shared_ptr<ZH::ObjectLoader> objectLoader;
    std::shared_ptr<ZH::Objects::InstanceFactory> instanceFactory;
    std::shared_ptr<ZH::Objects::Instance> instance;

    std::shared_ptr<ZH::InstanceRenderer> instanceRenderer;
    ZH::GFX::Camera camera;

  public:
    ObjectViewer (ZH::Window& window) : window(window) {}

    bool init(ZH::Config& config, std::string&& objectName) {
      viewHelpers = createViewHelpers(config, window.getVuglContext());

      iniLoader =
        std::shared_ptr<ZH::ResourceLoader>(new ZH::ResourceLoader {{"INIZH.big"}, config.baseDir});
      objectLoader = std::make_shared<ZH::ObjectLoader>(*iniLoader);
      if (!objectLoader->init()) {
        ERROR_ZH("Game", "Could not load objects list");
      }
      instanceFactory = std::make_shared<ZH::Objects::InstanceFactory>(*objectLoader);

      ZH::MapObject mapObject;
      this->objectName = objectName;
      mapObject.name = std::move(objectName);

      instance = instanceFactory->getInstance(mapObject);
      if (!instance) {
        ERROR_ZH("Main", "Unable to load object {}", this->objectName);
        return false;
      }

      instanceRenderer =
        std::make_shared<ZH::InstanceRenderer>(
            window.getVuglContext()
          , config
          , *viewHelpers.textureCache
          , *viewHelpers.modelCache
        );

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
      glm::mat4 modelMatrix {1.0f};
      auto vp = vuglContext.getViewport();

      ZH::GFX::Camera::Settings settings;
      settings.near = 0.1f;
      settings.far = 1000.0f;
      settings.fovDeg = 90.0f;
      settings.width = vp.width * 1.0f;
      settings.height = vp.height * 1.0f;

      camera.setPerspectiveProjection(settings);

      Vugl::RenderPassSetup renderPassSetup {vuglContext.getVkSurfaceFormat(), vuglContext.getVkSamplingFlag()};
      auto renderPass = vuglContext.createRenderPass(renderPassSetup);

      instanceRenderer->preparePipeline(renderPass);
      if (!instanceRenderer->prepareInstance(*instance)) {
        WARN_ZH("W3DView", "Failed to load model of object {}", objectName);
        return;
      }

      auto boundingSphere = instanceRenderer->getBoundingSphere(*instance);

      glm::vec3 camPosition = {
          boundingSphere.radius * 2.0f
        , boundingSphere.radius
        , 0.0f
      };

      camera.reposition(
          camPosition
        , glm::vec3 {0.0f,  0.0f, 0.0f}
        , glm::vec3 {0.0f, -1.0f, 0.0f}
      );

      viewHelpers.preparePipeline(renderPass);

      uint64_t frameIdxSet = 0;

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
          }
        }

        auto& frame = vuglContext.getNextFrame();
        auto frameIndex = frame.getImageIndex();

        if (updateMatrices || (frameIdxSet & (1 << frameIndex)) == 0) {
          if (updateMatrices) {
            frameIdxSet = 0;
            mvp =
              camera.getProjectionMatrix()
                * camera.getCameraMatrix()
                * modelMatrix;
          }

          viewHelpers.setAxesMatrix(camera.getProjectionMatrix() * camera.getCameraMatrix());

          instanceRenderer->updateInstance(
              *instance
            , frameIndex
            , mvp
            , camera.getPosition()
            , modelMatrix
            , camera.getDirectionVector()
          );
          frameIdxSet |= (1 << frameIndex);

          updateMatrices = false;
        }

        Vugl::CommandBuffer primary {vuglContext.createCommandBuffer(frameIndex)};
        primary.beginRendering(renderPass, clearColors);
        Vugl::CommandBuffer secondary {vuglContext.createCommandBuffer(frameIndex, true)};
        secondary.beginRendering(renderPass, clearColors);

        instanceRenderer->bindPipeline(secondary);
        instanceRenderer->renderInstance(*instance, secondary);

        viewHelpers.lineRenderer->bindPipeline(secondary);
        viewHelpers.renderAxes(frameIndex, secondary);

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
    ERROR_ZH("Main", "Please supply an object name.");
    return 1;
  }

  ZH::Config config;
  ZH::Window window;

  if (!window.init(config)) {
    ERROR_ZH("Main", "Viewer setup failed, terminating.");
    return 1;
  }

  ObjectViewer viewer {window};
  if (!viewer.init(config, std::string {argv[1]})) {
    return 1;
  }

  viewer.loop();

  return 0;
}
