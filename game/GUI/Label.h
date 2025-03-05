#ifndef H_GUI_LABEL
#define H_GUI_LABEL

#include "Component.h"

namespace ZH::GUI {

class Label : public Component {
  public:
    Label();

    const std::u16string& getText() const;

    void setText(std::u16string);
  private:
    std::u16string text;
};

}

#endif
