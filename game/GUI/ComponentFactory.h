// SPDX-License-Identifier: GPL-2.0

#ifndef H_GUI_COMPONENT_FACTORY
#define H_GUI_COMPONENT_FACTORY

#include "../common.h"
#include "../StringLoader.h"
#include "../gfx/TextureLookup.h"
#include "Component.h"
#include "wnd/Layout.h"
#include "wnd/Window.h"

namespace ZH::GUI {

class ComponentFactory {
  public:
    ComponentFactory(StringLoader&, ZH::GFX::TextureLookup&);

    std::shared_ptr<Component> createComponent(const WND::Window&, OptionalCRef<WND::Layout>);
  private:
    ZH::GFX::TextureLookup& textureLookup;
    StringLoader& stringLoader;

    std::shared_ptr<Component> createButton(const WND::Window&);
    std::shared_ptr<Component> createText(const WND::Window&);
    std::shared_ptr<Component> createWindow(const WND::Window&, OptionalCRef<WND::Layout>);

    std::u16string getNAString(const std::string&);

    bool setCommonAttributes(Component&, const WND::Window&) const;
    void setDrawData(
        Component&
      , OptionalCRef<WND::DrawData>
      , std::function<void(Component&, uint8_t, const INIImage&)>
    ) const;
    bool setPositionSize(Component&, const WND::Window&) const;
    bool setName(Component&, const WND::Window&) const;

    template<typename T>
    bool setText(T& t, OptionalCRef<std::string> key) {
      if (key) {
        auto result = stringLoader.getString(*key);
        if (result) {
          t.setText(result->get().string);
          return true;
        } else {
          t.setText(getNAString(*key));
        }
      }

      return true;
    }
};

}

#endif
