#include "ScreenRect.h"

namespace ZH::GUI::WND {

Point ScreenRect::getUpperLeft() const {
  return upperLeft;
}

Point ScreenRect::getBottomRight() const {
  return bottomRight;
}

Size ScreenRect::getResolution() const {
  return resolution;
}

}
