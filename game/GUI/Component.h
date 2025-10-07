// SPDX-License-Identifier: GPL-2.0

#ifndef H_GUI_COMPONENT
#define H_GUI_COMPONENT

#include <functional>
#include <list>

#include "../common.h"
#include "../Dimensions.h"
#include "../inis/MappedImageINI.h"
#include "wnd/Window.h"

namespace ZH::GUI {

struct Font {
  uint8_t size;
  bool bold;
};

// The superclass of everything WINDOW in wnd files
class Component {
  public:
    using ImagesContainer = std::array<std::optional<INIImage>, 9>;
    using ChildrenContainer = std::list<std::shared_ptr<Component>>;
    using ClickListener = std::function<void(const Component&)>;
    using ComponentType = WND::Window::Type;

    Component(ComponentType type);
    Component(const Component&) = delete;
    virtual ~Component() = default;

    bool operator==(const Component&) const;
    bool operator!=(const Component&) const;

    void addClickListener(ClickListener&&);
    std::shared_ptr<Component> findByName(const std::string&) const;

    ChildrenContainer& getChildren();
    const ImagesContainer& getEnabledImages() const;
    const ImagesContainer& getHighlightImages() const;
    OptionalCRef<Font> getFont() const;
    uint64_t getID() const;
    const std::string& getName() const;
    Point getPosition() const;
    Point getPositionOffset() const;
    Size getSize() const;
    OptionalCRef<WND::TextColor> getTextColor() const;
    ComponentType getType() const;

    bool isDrawImage() const;
    bool isHidden() const;
    bool isPointInside(const Point&) const;

    bool needsRedraw() const;
    void setRedrawn();

    virtual bool onCursorOver(const Point&, const Point&);
    virtual void onCursorOut();
    virtual bool onCursorClick() { return false; };
    virtual void onCursorClickDown();
    virtual bool onCursorClickUp();

    void setDrawImage(bool);
    void setEnabledImage(uint8_t, const INIImage&);
    void setFont(std::optional<Font>);
    void setHighlightImage(uint8_t, const INIImage&);
    void setHidden(bool);
    void setName(std::string);
    void setPosition(Point);
    void setPositionOffset(Point);
    void setSize(Size);
    void setTextColor(std::optional<WND::TextColor>);
  protected:
    static uint64_t nextId;

    uint64_t id;
    ComponentType type;
    bool drawUpdate = true;
    ChildrenContainer children;
    ImagesContainer enabledImages;
    ImagesContainer highlightImages;
    std::optional<Font> font;
    std::string name;
    Point position;
    Point positionOffset;
    Size size;
    bool hidden = false;
    bool drawImage = true;
    std::optional<WND::TextColor> textColor;
    std::shared_ptr<Component> lastMouseClickLockChild = nullptr;
    std::shared_ptr<Component> lastMouseOverChild = nullptr;

    std::list<ClickListener> clickListeners;

    void fireClickEvent();
};

}

#endif
