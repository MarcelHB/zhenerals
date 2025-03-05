#ifndef H_GUI_WND_FONT
#define H_GUI_WND_FONT

#include <cstdint>

#include "../../common.h"

namespace ZH::GUI::WND {

class Font {
  public:
    struct Builder {
      OptionalString name;
      std::optional<uint8_t> size;
      bool bold;
    };

    Font(Builder&& builder)
      : name(std::move(builder.name))
      , size(builder.size)
      , bold(builder.bold)
    {}

    OptionalString getName() const;
    std::optional<uint8_t> getSize() const;
    bool isBold() const;
  private:
    OptionalString name;
    std::optional<uint8_t> size;
    bool bold;
};

}

#endif
