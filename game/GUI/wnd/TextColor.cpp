// SPDX-License-Identifier: GPL-2.0

#include "TextColor.h"

namespace ZH::GUI::WND {

OptionalColor TextColor::getDisabled() const {
  return disabled;
}

OptionalColor TextColor::getDisabledBorder() const {
  return disabledBorder;
}

OptionalColor TextColor::getEnabled() const {
  return enabled;
}

OptionalColor TextColor::getEnabledBorder() const {
  return enabledBorder;
}

OptionalColor TextColor::getHighlight() const {
  return highlight;
}

OptionalColor TextColor::getHighlightBorder() const {
  return highlightBorder;
}

}
