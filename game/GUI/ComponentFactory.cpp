// SPDX-License-Identifier: GPL-2.0

#include "../Logging.h"
#include "ComponentFactory.h"
#include "Button.h"
#include "Label.h"
#include "Window.h"

namespace ZH::GUI {

ComponentFactory::ComponentFactory(
    StringLoader& stringLoader
  , ZH::GFX::TextureLookup& textureLookup
) : stringLoader(stringLoader)
  , textureLookup(textureLookup)
{}

std::shared_ptr<Component> ComponentFactory::createComponent(const WND::Window& wnd, OptionalCRef<WND::Layout> layout) {
  TRACY(ZoneScoped);

  switch (wnd.getType()) {
    case WND::Window::Type::PUSHBUTTON:
      return createButton(wnd);
    case WND::Window::Type::STATICTEXT:
      return createText(wnd);
    case WND::Window::Type::USER:
      return createWindow(wnd, layout);
    default:
      WARN_ZH("ComponentFactory", "Unimplemented type");
      return {};
  }
}

std::shared_ptr<Component> ComponentFactory::createButton(const WND::Window& wnd) {
  std::shared_ptr<Button> button = std::shared_ptr<Button> {new Button {}};

  setCommonAttributes(*button, wnd);
  setText(*button, wnd.getText());

  return button;
}

std::shared_ptr<Component> ComponentFactory::createText(const WND::Window& wnd) {
  std::shared_ptr<Label> label = std::shared_ptr<Label> {new Label {}};

  setCommonAttributes(*label, wnd);
  setText(*label, wnd.getText());

  return label;
}

std::shared_ptr<Component> ComponentFactory::createWindow(const WND::Window& wnd, OptionalCRef<WND::Layout> layout) {
  std::shared_ptr<Window> window = std::shared_ptr<Window> {new Window {}};

  setCommonAttributes(*window, wnd);
  for (auto child : wnd.getChildren()) {
    auto childComponent = createComponent(*child, std::nullopt);
    if (childComponent) {
      window->getChildren().push_back(std::move(childComponent));
    }
  }

  return window;
}

bool ComponentFactory::setCommonAttributes(Component& c, const WND::Window& wnd) const {
  if (!setName(c, wnd)) {
    return {};
  }

  if (!setPositionSize(c, wnd)) {
    return {};
  }

  if (wnd.getStatus() & WND::Window::Status::HIDDEN) {
    c.setHidden(true);
  }

  if (wnd.getStatus() & WND::Window::Status::IMAGE) {
    c.setDrawImage(true);
  }

  auto drawData = wnd.getEnabledDrawData();
  setDrawData(c, drawData, [](Component& c, uint8_t i, const INIImage& texture) {
    c.setEnabledImage(i, texture);
  });

  drawData = wnd.getHighlightDrawData();
  setDrawData(c, drawData, [](Component& c, uint8_t i, const INIImage& texture) {
    c.setHighlightImage(i, texture);
  });

  if (wnd.getFont()) {
    c.setFont(std::make_optional<Font>(
        (wnd.getFont()->get().getSize().value_or(12) * 3) / 2 // that's stylistic and maybe a matter of Noto
      , wnd.getFont()->get().isBold()
    ));
  }

  if (wnd.getTextColor()) {
    c.setTextColor(wnd.getTextColor());
  }

  return true;
}

void ComponentFactory::setDrawData(
    Component& component
  , OptionalCRef<WND::DrawData> drawData
  , std::function<void(Component&, uint8_t, const INIImage&)> applier
) const {
  if (drawData) {
    for (uint8_t i = 0; i < drawData->get().getEntries().size(); ++i) {
      if (!drawData->get().getEntries()[i].image) {
        continue;
      }

      auto texture = textureLookup.getTexture(*drawData->get().getEntries()[i].image);

      if (!texture) {
        WARN_ZH("ComponentFactory", "Did not find texture: {}", drawData->get().getEntries()[i].image)
      } else {
        applier(component, i, *texture);
      }
    }
  }
}

bool ComponentFactory::setName(Component& c, const WND::Window& wnd) const {
  if (!wnd.getName()) {
    WARN_ZH("ComponentFactory", "Missing name.");
    return false;
  }

  c.setName(*wnd.getName());

  return true;
}

bool ComponentFactory::setPositionSize(Component& c, const WND::Window& wnd) const {
  auto screenRect = wnd.getScreenRect();
  if (!screenRect) {
    WARN_ZH("ComponentFactory", "{}: no screen rect.", c.getName());
    return false;
  }

  auto p1 = screenRect->get().getUpperLeft();
  auto p2 = screenRect->get().getBottomRight();

  if (p1.x > p2.x || p1.y > p2.y) {
    WARN_ZH("ComponentFactory", "{}: bad coordinates.", c.getName());
    return false;
  }

  c.setPosition(p1);
  c.setSize({
      static_cast<Size::value_type>(p2.x - p1.x)
    , static_cast<Size::value_type>(p2.y - p1.y)
  });

  return true;
}

// EVAL
std::u16string ComponentFactory::getNAString(const std::string& key) {
  std::u16string na = u"n/a: ";

  std::vector<char> buffer;
  buffer.resize(key.size() * 2);
  for (size_t i = 0; i < key.size(); ++i) {
    if (static_cast<uint8_t>(key[i]) < 0x80) {
      buffer[i*2] = key[i];
    } else {
      buffer[i*2] = '_';
    }
    buffer[i*2 + 1] = '\x0';
  }

  std::u16string wkey{reinterpret_cast<char16_t*>(buffer.data()), key.size()};

  return na.append(wkey);
}

}
