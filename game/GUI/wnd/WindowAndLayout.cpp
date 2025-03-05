#include "WindowAndLayout.h"

namespace ZH::GUI::WND {

OptionalCRef<Layout> WindowAndLayout::getLayout() const {
  return ptrToCOptional(layout);
}

const Window& WindowAndLayout::getWindow() const {
  return *window;
}

}
