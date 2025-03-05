#ifndef H_GUI_WND_WINDOW_LAYOUT
#define H_GUI_WND_WINDOW_LAYOUT

#include <memory>

#include "../../common.h"
#include "Layout.h"
#include "Window.h"

namespace ZH::GUI::WND {

class WindowAndLayout {
  public:
    WindowAndLayout(
        std::shared_ptr<Window>&& window
      , std::shared_ptr<Layout>&& layout
    ) : window(std::move(window))
      , layout(std::move(layout))
    {}

    const Window& getWindow() const;
    OptionalCRef<Layout> getLayout() const;
  private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Layout> layout;
};

}

#endif
