// SPDX-License-Identifier: GPL-2.0

#include "Label.h"

namespace ZH::GUI {

Label::Label() : Component(WND::Window::Type::STATICTEXT)
{}

const std::u16string& Label::getText() const {
  return text;
}

void Label::setText(std::u16string text) {
  drawUpdate = true;
  this->text = text;
}

}
