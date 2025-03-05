#ifndef H_GUI_WND_TEXT_COLOR
#define H_GUI_WND_TEXT_COLOR

#include <optional>

#include "../../Color.h"

namespace ZH::GUI::WND {

using OptionalColor = std::optional<Color>;

class TextColor {
  public:
    struct Builder {
      OptionalColor disabled;
      OptionalColor disabledBorder;
      OptionalColor enabled;
      OptionalColor enabledBorder;
      OptionalColor highlight;
      OptionalColor highlightBorder;
    };

    TextColor(Builder&& builder)
      : disabled(std::move(builder.disabled))
      , disabledBorder(std::move(builder.disabledBorder))
      , enabled(std::move(builder.enabled))
      , enabledBorder(std::move(builder.enabledBorder))
      , highlight(std::move(builder.highlight))
      , highlightBorder(std::move(builder.highlightBorder))
    {}

    OptionalColor getDisabled() const;
    OptionalColor getDisabledBorder() const;
    OptionalColor getEnabled() const;
    OptionalColor getEnabledBorder() const;
    OptionalColor getHighlight() const;
    OptionalColor getHighlightBorder() const;
  private:
    OptionalColor disabled;
    OptionalColor disabledBorder;
    OptionalColor enabled;
    OptionalColor enabledBorder;
    OptionalColor highlight;
    OptionalColor highlightBorder;
};

}

#endif
