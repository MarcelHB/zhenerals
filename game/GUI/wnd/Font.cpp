// SPDX-License-Identifier: GPL-2.0

#include "Font.h"

namespace ZH::GUI::WND {

OptionalString Font::getName() const {
  return name;
}

std::optional<uint8_t> Font::getSize() const {
  return size;
}

bool Font::isBold() const {
  return bold;
}

}
