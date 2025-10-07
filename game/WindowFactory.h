// SPDX-License-Identifier: GPL-2.0

#ifndef H_WINDOW_FACTORY
#define H_WINDOW_FACTORY

#include "common.h"
#include "Config.h"
#include "ResourceLoader.h"
#include "GUI/wnd/WindowAndLayout.h"

namespace ZH {

class WindowFactory {
  public:
    WindowFactory(const Config& config);

    std::shared_ptr<GUI::WND::WindowAndLayout> getWND(const std::string&);
  private:
    ResourceLoader loader;
};

}

#endif
