// SPDX-License-Identifier: GPL-2.0

#ifndef H_GUI_WND_LAYOUT
#define H_GUI_WND_LAYOUT

#include "../../common.h"

namespace ZH::GUI::WND {

class Layout {
  public:
    struct Builder {
      OptionalString initFunction;
      OptionalString updateFunction;
      OptionalString shutdownFunction;
    };

    Layout(Builder&& builder)
      : initFunction(std::move(builder.initFunction))
      , updateFunction(std::move(builder.updateFunction))
      , shutdownFunction(std::move(builder.shutdownFunction))
    {}

    OptionalCRef<std::string> getInitFunction() const;
    OptionalCRef<std::string> getUpdateFunction() const;
    OptionalCRef<std::string> getShutdownFunction() const;
  private:
    OptionalString initFunction;
    OptionalString updateFunction;
    OptionalString shutdownFunction;
};

}

#endif
