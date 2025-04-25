#include "Overlay.h"

namespace ZH::GUI {

Overlay::Overlay(Size size) : Component(WND::Window::Type::OVERLAY) {
  setSize(size);
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

  switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
      onCursorClickDown();
      break;
    case SDL_MOUSEBUTTONUP:
      onCursorClickUp();
      break;
    case SDL_MOUSEMOTION:
      auto childPos = (*children.begin())->getPositionOffset();
      Point pos {event.motion.x - childPos.x, event.motion.y - childPos.y};
      Point movement {event.motion.xrel, event.motion.yrel};
      onCursorOver(pos, movement);
      break;
  }

  return true;
}

void Overlay::setBattlefield(std::shared_ptr<Battlefield> battlefield) {
  this->battlefield = std::move(battlefield);
}

}
