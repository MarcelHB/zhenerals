// SPDX-License-Identifier: GPL-2.0

#ifndef H_GUI_BUTTON
#define H_GUI_BUTTON

#include "../common.h"
#include "Component.h"

namespace ZH::GUI {

class Button : public Component {
  public:
    Button();

    const std::u16string& getText() const;
    bool isHighlighted() const;
    bool isLocked() const;

    bool onCursorClick() override;
    void onCursorClickDown() override;
    void onCursorOut() override;
    bool onCursorOver(const Point&, const Point&) override;

    void setText(std::u16string);
  private:
    std::u16string text;
    bool highlighted = false;
    bool locked = false;
};

}

#endif
