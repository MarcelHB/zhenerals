#include "Component.h"

namespace ZH::GUI {

uint64_t Component::nextId = 0;

Component::Component(ComponentType type)
  : id(nextId++)
  , type(type)
{}

bool Component::operator==(const Component& other) const {
  return id == other.id;
}

bool Component::operator!=(const Component& other) const {
  return !operator==(other);
}

void Component::addClickListener(ClickListener&& listener) {
  clickListeners.emplace_back(std::move(listener));
}

std::shared_ptr<Component> Component::findByName(const std::string& name) const {
  for (auto& child : children) {
    if (child->getName() == name) {
      return child;
    } else {
      auto result = child->findByName(name);
      if (result) {
        return result;
      }
    }
  }

  return {};
}

void Component::fireClickEvent() {
  for (auto& listener : clickListeners) {
    listener(*this);
  }
}

Component::ChildrenContainer& Component::getChildren() {
  return children;
}

const Component::ImagesContainer& Component::getEnabledImages() const {
  return enabledImages;
}

const Component::ImagesContainer& Component::getHighlightImages() const {
  return highlightImages;
}

OptionalCRef<Font> Component::getFont() const {
  return optToCRef(font);
}

uint64_t Component::getID() const {
  return id;
}

const std::string& Component::getName() const {
  return name;
}

Point Component::getPosition() const {
  return position;
}

Point Component::getPositionOffset() const {
  return positionOffset;
}

Size Component::getSize() const {
  return size;
}

OptionalCRef<WND::TextColor> Component::getTextColor() const {
  return optToCRef(textColor);
}

Component::ComponentType Component::getType() const {
  return type;
}

bool Component::isHidden() const {
  return hidden;
}

bool Component::isPointInside(const Point& p) const {
  return
    p.x >= position.x && p.x <= position.x + size.w
      && p.y >= position.y && p.y <= position.y + size.h;
}

void Component::onCursorClickDown() {
  if (lastMouseOverChild) {
    lastMouseClickLockChild = lastMouseOverChild;
    lastMouseClickLockChild->onCursorClickDown();
  }
}

bool Component::onCursorClickUp() {
  if (lastMouseClickLockChild && lastMouseOverChild) {
    if (*lastMouseClickLockChild == *lastMouseOverChild) {
      if (lastMouseOverChild->onCursorClickUp()) {
        return true;
      } else {
        return onCursorClick();
      }
    }
  }

  return onCursorClick();
}

void Component::onCursorOut() {
  if (lastMouseOverChild) {
    lastMouseOverChild->onCursorOut();
    lastMouseOverChild = {};
  }
}

bool Component::onCursorOver(const Point& pos, const Point& movement) {
  for (auto it = children.begin(); it != children.end(); ++it) {
    if ((*it)->isHidden()) {
      continue;
    }

    if ((*it)->isPointInside(pos) && (*it)->onCursorOver(pos, movement)) {
      if (lastMouseOverChild && (*it)->operator!=(*lastMouseOverChild)) {
        lastMouseOverChild->onCursorOut();
      }
      lastMouseOverChild = *it;
      return true;
    }
  }

  if (lastMouseOverChild) {
    lastMouseOverChild->onCursorOut();
    lastMouseOverChild = {};
  }

  return false;
}

bool Component::needsRedraw() const {
  return drawUpdate;
}

void Component::setEnabledImage(uint8_t index, const INIImage& iniImage) {
  drawUpdate = true;
  enabledImages[index] = std::make_optional<INIImage>(iniImage);
}

void Component::setHighlightImage(uint8_t index, const INIImage& iniImage) {
  drawUpdate = true;
  highlightImages[index] = std::make_optional<INIImage>(iniImage);
}

void Component::setFont(std::optional<Font> font) {
  drawUpdate = true;
  this->font = std::move(font);
}

void Component::setHidden(bool hidden) {
  drawUpdate = true;
  this->hidden = hidden;
}

void Component::setName(std::string name) {
  this->name = std::move(name);
}

void Component::setPosition(Point p) {
  drawUpdate = true;
  position = p;
}

void Component::setPositionOffset(Point p) {
  drawUpdate = true;
  positionOffset = p;
}

void Component::setRedrawn() {
  drawUpdate = false;
}

void Component::setSize(Size s) {
  drawUpdate = true;
  size = s;
}

void Component::setTextColor(std::optional<WND::TextColor> textColor) {
  drawUpdate = true;
  this->textColor = std::move(textColor);
}

}
