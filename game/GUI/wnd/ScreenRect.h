// SPDX-License-Identifier: GPL-2.0

#ifndef H_GUI_WND_SCREEN_RECT
#define H_GUI_WND_SCREEN_RECT

#include "../../common.h"
#include "../../Dimensions.h"

namespace ZH::GUI::WND {

class ScreenRect {
  public:
    struct Builder {
      Point upperLeft;
      Point bottomRight;
      Size resolution;
    };

    ScreenRect(Builder&& builder)
      : upperLeft(std::move(builder.upperLeft))
      , bottomRight(std::move(builder.bottomRight))
      , resolution(std::move(builder.resolution))
    {}

    Point getUpperLeft() const;
    Point getBottomRight() const;
    Size getResolution() const;
  private:
    Point upperLeft;
    Point bottomRight;
    Size resolution; // EVAL
};

}

#endif
