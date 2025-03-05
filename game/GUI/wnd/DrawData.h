#ifndef H_GUI_WND_DRAW_DATA
#define H_GUI_WND_DRAW_DATA

#include <array>

#include "../../common.h"
#include "../../Color.h"

namespace ZH::GUI::WND {

class DrawData {
  public:
    struct Entry {
      OptionalString image;
      Color color;
      Color borderColor;
    };

    using Entries = std::array<Entry, 9>;

    DrawData(Entries&& entries) : entries(std::move(entries)) {}
    const Entries& getEntries() const;
  private:
    Entries entries;
};

}

#endif
