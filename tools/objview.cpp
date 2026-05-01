// SPDX-License-Identifier: GPL-2.0

#include <iterator>

#include <fmt/xchar.h>

#include "../game/Config.h"
#include "../game/gfx/Camera.h"
#include "../game/GUI/Component.h"
#include "../game/GUI/drawing/RenderListFactory.h"
#include "../game/Logger.h"
#include "../game/Map.h"
#include "../game/ObjectLoader.h"
#include "../game/objects/InstanceFactory.h"
#include "../game/rendering/InstanceRenderer.h"
#include "../game/Window.h"
#include "ViewHelpers.h"

std::u16string fromASCIIString(const std::string& input) {
  std::u16string output;
  output.resize(input.size() * 2);

  for (size_t i = 0; i < input.size(); ++i) {
    uint16_t w = input[i];
    output[i] = w;
  }

  return output;
}

struct RootOverlay : ZH::GUI::Component {
  RootOverlay() : ZH::GUI::Component(ZH::GUI::WND::Window::Type::OVERLAY) {}
};

class ObjectViewer {
  private:
    ZH::Window& window;
    ViewHelpers viewHelpers;
    std::string objectName;

    std::shared_ptr<ZH::ResourceLoader> iniLoader;
    std::shared_ptr<ZH::ObjectLoader> objectLoader;
    std::shared_ptr<ZH::Objects::InstanceFactory> instanceFactory;
    std::shared_ptr<ZH::Objects::Instance> instance;
    RootOverlay rootComponent;
    std::shared_ptr<ZH::GUI::Drawing::RenderListFactory> guiRenderer;

    std::shared_ptr<ZH::InstanceRenderer> instanceRenderer;
    ZH::GFX::Camera camera;

    size_t currentDrawDataIdx = 0;
    size_t currentStateIdx = 0;

    bool shiftState(bool next) {
      auto& currentDrawData = instance->getBase()->drawMetaData[currentDrawDataIdx];
      if (!currentDrawData.hasStateConditions()) {
        return false;
      }

      if (currentStateIdx == 0 && !next) {
        return false;
      }

      auto md = std::static_pointer_cast<ZH::Objects::ModelDrawData>(currentDrawData.drawData);
      if (currentStateIdx == md->conditionStates.size() && next) {
        return false;
      }

      currentStateIdx += next ? 1 : -1;

      return true;
    }

    void buildGUI() {
      auto vp = window.getVuglContext().getViewport();

      std::shared_ptr<ZH::GUI::Label> label =
        std::make_shared<ZH::GUI::Label>();
      label->setName("numDrawData");
      label->setPosition(ZH::Point {vp.width - 150, 20});
      label->setSize(ZH::Size { 140, 20 });

      rootComponent.getChildren().emplace_back(std::move(label));

      label = std::make_shared<ZH::GUI::Label>();
      label->setName("numStates");
      label->setPosition(ZH::Point {vp.width - 150, 20});
      label->setSize(ZH::Size { 140, 40 });

      rootComponent.getChildren().emplace_back(std::move(label));

      label = std::make_shared<ZH::GUI::Label>();
      label->setName("currentModel");
      label->setPosition(ZH::Point {vp.width / 2 - 100, 20});
      label->setSize(ZH::Size { 200, 60 });

      rootComponent.getChildren().emplace_back(std::move(label));
    }

    const ZH::Objects::ConditionState& getCurrentConditionState() const {
      const ZH::Objects::ConditionState *nextState = nullptr;
      auto& currentDrawData = instance->getBase()->drawMetaData[currentDrawDataIdx];
      auto md = std::static_pointer_cast<ZH::Objects::ModelDrawData>(currentDrawData.drawData);

      if (currentStateIdx == 0) {
        return md->defaultConditionState;
      } else {
        auto it = md->conditionStates.cbegin();
        std::advance(it, currentStateIdx - 1);
        return *it;
      }
    }

    void updateLabels() {
      auto label = rootComponent.findByName<ZH::GUI::Label>("numDrawData");
      label->setText(
        fmt::format(
            u"# draw data: {} ({})"
          , instance->getBase()->drawMetaData.size()
          , currentDrawDataIdx
        )
      );

      size_t numStates = 1;
      auto& currentDrawData = instance->getBase()->drawMetaData[currentDrawDataIdx];
      if (currentDrawData.hasStateConditions()) {
        // EVAL assumption for now
        auto md = std::static_pointer_cast<ZH::Objects::ModelDrawData>(currentDrawData.drawData);
        numStates = md->conditionStates.size() + 1;
      }

      label = rootComponent.findByName<ZH::GUI::Label>("numStates");
      label->setText(
        fmt::format(
            u"# states: {} ({})"
          , numStates
          , currentStateIdx
        )
      );

      auto& currentState = getCurrentConditionState();
      if (!currentState.model.empty()) {
        label = rootComponent.findByName<ZH::GUI::Label>("currentModel");
        label->setText(fromASCIIString(currentState.model));
      }
    }
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

      guiRenderer =
        std::make_shared<ZH::GUI::Drawing::RenderListFactory>(
            window.getVuglContext()
          , rootComponent
          , *viewHelpers.textureCache
          , *viewHelpers.fontManager
        );

      instanceRenderer =
        std::make_shared<ZH::InstanceRenderer>(
            window.getVuglContext()
          , config
          , *viewHelpers.textureCache
          , *viewHelpers.modelCache
        );

      buildGUI();

      return true;
    }

    void loop() {
      std::array<VkClearValue, 2> clearColors{};
      clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
      clearColors[1].depthStencil = {1.0f, 0};

      auto& vuglContext = window.getVuglContext();

      bool updateMatrices = true;
      bool updateUI = true;
      bool updateState = true;
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
            case SDL_EVENT_KEY_DOWN:
              switch (event.key.key) {
                case SDLK_S:
                  updateState = shiftState(false);
                  break;
                case SDLK_W:
                  updateState = shiftState(true);
                  break;
                default: break;
              }
              // fallthrough
            default: break;
          }
        }

        if (updateState) {
          auto& nextState = getCurrentConditionState();
          if (instanceRenderer->useConditionState(*instance, currentDrawDataIdx, nextState)) {
            updateMatrices = true;
          }
          updateUI = true;
          updateState = false;
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

        if (updateUI) {
          updateLabels();
        }

        Vugl::CommandBuffer primary {vuglContext.createCommandBuffer(frameIndex)};
        primary.beginRendering(renderPass, clearColors);
        Vugl::CommandBuffer secondary {vuglContext.createCommandBuffer(frameIndex, true)};
        secondary.beginRendering(renderPass, clearColors);

        instanceRenderer->bindPipeline(secondary);
        instanceRenderer->renderInstance(*instance, secondary);

        viewHelpers.lineRenderer->bindPipeline(secondary);
        viewHelpers.renderAxes(frameIndex, secondary);

        guiRenderer->createRenderList(secondary, frameIndex, renderPass);

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
