// SPDX-License-Identifier: GPL-2.0

#include "Overlay.h"

namespace ZH::GUI {

Overlay::Overlay(Size size) : Component(WND::Window::Type::OVERLAY) {
  setSize(size);
}

void Overlay::frameDoneTick() {
  getBattlefield()->frameDoneTick();
}

std::shared_ptr<Battlefield> Overlay::getBattlefield() const {
  return battlefield;
}

std::unique_lock<std::recursive_mutex> Overlay::getLock() {
  return std::unique_lock {mutex};
}

bool Overlay::processEvent(const SDL_Event& event) {
  if (children.size() == 0) {
    return true;
  }

  auto accelerate = [](float v, bool y) -> float {
    auto sign = v >= 0 ? (y ? 1 : -1) : (y ? -1 : 1);
    return sign * std::min((y ? 60.0f : 100.0f), v * v) / 16.0f;
  };

  switch (event.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (event.button.button == SDL_BUTTON_RIGHT
          && cameraControlMode == CameraControlMode::NONE) {
        cameraControlMode = CameraControlMode::DIRECTIONAL;
      } else if (event.button.button == SDL_BUTTON_LEFT
          && cameraControlMode == CameraControlMode::DIRECTIONAL) {
        cameraControlMode = CameraControlMode::AXES;
      } else if (event.button.button == SDL_BUTTON_LEFT) {
        onCursorClickDown();
      }
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (event.button.button == SDL_BUTTON_LEFT
          && cameraControlMode == CameraControlMode::AXES) {
        cameraControlMode = CameraControlMode::DIRECTIONAL;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        cameraControlMode = CameraControlMode::NONE;
      } else if (event.button.button == SDL_BUTTON_LEFT
          && cameraControlMode == CameraControlMode::NONE) {
        onCursorClickUp();
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      if (cameraControlMode == CameraControlMode::AXES) {
        getBattlefield()->moveCameraAxially(
            event.motion.xrel * 2.0f
          , -event.motion.yrel * 2.0f
        );
      } else if (cameraControlMode == CameraControlMode::DIRECTIONAL) {
        getBattlefield()->moveCameraDirectionally(
            accelerate(event.motion.xrel, false)
          , accelerate(event.motion.yrel, true)
        );
      } else {
        auto childPos = (*children.begin())->getPositionOffset();
        Point pos {event.motion.x - childPos.x, event.motion.y - childPos.y};
        Point movement {event.motion.xrel, event.motion.yrel};
        onCursorOver(pos, movement);
      }
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      getBattlefield()->zoomCamera(event.wheel.y * 10);
      break;
  }

  return true;
}

void Overlay::setBattlefield(std::shared_ptr<Battlefield> battlefield) {
  this->battlefield = std::move(battlefield);
}

}
