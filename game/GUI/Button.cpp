// SPDX-License-Identifier: GPL-2.0

#include "Button.h"

namespace ZH::GUI {

Button::Button() : Component(WND::Window::Type::PUSHBUTTON)
{}

const std::u16string& Button::getText() const {
  return text;
}

bool Button::isHighlighted() const {
  return highlighted;
}

bool Button::isLocked() const {
  return locked;
}

bool Button::onCursorClick() {
  locked = false;
  fireClickEvent();
  return true;
}

void Button::onCursorClickDown() {
  locked = true;
}

void Button::onCursorOut() {
  highlighted = false;
  locked = false;
  drawUpdate = true;
}

bool Button::onCursorOver(const Point&, const Point&) {
  drawUpdate = true;
  highlighted = true;
  return true;
}

void Button::setText(std::u16string text) {
  drawUpdate = true;
  this->text = text;
}

}
