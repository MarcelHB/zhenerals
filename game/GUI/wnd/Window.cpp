#include "Window.h"

namespace ZH::GUI::WND {

Window::Status Window::Builder::getStatusByString(const std::string_view v) {
  if (v == "ACTIVE") {
    return Status::ACTIVE;
  } else if (v == "TOGGLE") {
    return Status::TOGGLE;
  } else if (v == "DRAGABLE") {
    return Status::DRAGGABLE;
  } else if (v == "ENABLED") {
    return Status::ENABLED;
  } else if (v == "HIDDEN") {
    return Status::HIDDEN;
  } else if (v == "ABOVE") {
    return Status::ABOVE;
  } else if (v == "BELOW") {
    return Status::BELOW;
  } else if (v == "IMAGE") {
    return Status::IMAGE;
  } else if (v == "TABSTOP") {
    return Status::TABSTOP;
  } else if (v == "NOINPUT") {
    return Status::NO_INPUT;
  } else if (v == "NOFOCUS") {
    return Status::NO_FOCUS;
  } else if (v == "DESTROYED") {
    return Status::DESTROYED;
  } else if (v == "BORDER") {
    return Status::BORDER;
  } else if (v == "SMOOTH_TEXT") {
    return Status::SMOOTH_TEXT;
  } else if (v == "ONE_LINE") {
    return Status::ONE_LINE;
  } else if (v == "NO_FLUSH") {
    return Status::NO_FLUSH;
  } else if (v == "SEE_THRU") {
    return Status::SEE_THROUGH;
  } else if (v == "RIGHT_CLICK") {
    return Status::RIGHT_CLICK;
  } else if (v == "WRAP_CENTERED") {
    return Status::WRAP_CENTERED;
  } else if (v == "CHECK_LIKE") {
    return Status::CHECK_LIKE;
  } else if (v == "HOTKEY_TEXT") {
    return Status::HOTKEY_TEXT;
  } else if (v == "USE_OVERLAY_STATES") {
    return Status::USE_OVERLAY_STATES;
  } else if (v == "NOT_READY") {
    return Status::NOT_READY;
  } else if (v == "FLASHING") {
    return Status::FLASHING;
  } else if (v == "ALWAYS_COLOR") {
    return Status::ALWAYS_COLOR;
  } else if (v == "ON_MOUSE_DOWN") {
    return Status::ON_MOUSE_DOWN;
  } else {
    return Status::NOTHING;
  }
}

Window::Style Window::Builder::getStyleByString(const std::string& v) {
  if (v == "PUSHBUTTON") {
    return Style::PUSHBUTTON;
  } else if (v == "RADIOBUTTON") {
    return Style::RADIOBUTTON;
  } else if (v == "CHECKBOX") {
    return Style::CHECKBOX;
  } else if (v == "VERTSLIDER") {
    return Style::VERTSLIDER;
  } else if (v == "HORZSLIDER") {
    return Style::HORZSLIDER;
  } else if (v == "SCROLLLISTBOX") {
    return Style::SCROLLLISTBOX;
  } else if (v == "ENTRYFIELD") {
    return Style::ENTRYFIELD;
  } else if (v == "STATICTEXT") {
    return Style::STATICTEXT;
  } else if (v == "PROGRESSBAR") {
    return Style::PROGRESSBAR;
  } else if (v == "USER") {
    return Style::USER;
  } else if (v == "MOUSETRACK") {
    return Style::MOUSETRACK;
  } else if (v == "ANIMATED") {
    return Style::ANIMATED;
  } else if (v == "TABSTOP") {
    return Style::TABSTOP;
  } else if (v == "TABCONTROL") {
    return Style::TABCONTROL;
  } else if (v == "TABPANE") {
    return Style::TABPANE;
  } else if (v == "COMBOBOX") {
    return Style::COMBOBOX;
  } else {
    return Style::UNKNOWN;
  }
}

Window::Type Window::Builder::getTypeByString(const std::string& v) {
  if (v == "VERTSLIDER") {
    return Type::VERTSLIDER;
  } else if (v == "HORZSLIDER") {
    return Type::HORZSLIDER;
  } else if (v == "SCROLLLISTBOX") {
    return Type::SCROLLLISTBOX;
  } else if (v == "TABCONTROL") {
    return Type::TABCONTROL;
  } else if (v == "ENTRYFIELD") {
    return Type::ENTRYFIELD;
  } else if (v == "STATICTEXT") {
    return Type::STATICTEXT;
  } else if (v == "RADIOBUTTON") {
    return Type::STATICTEXT;
  } else if (v == "COMBOBOX") {
    return Type::COMBOBOX;
  } else if (v == "PUSHBUTTON") {
    return Type::PUSHBUTTON;
  } else {
    return Type::USER;
  }
}

Window::Window(Builder&& builder)
 : children(std::move(builder.children))
 , type(std::move(builder.type))
 , name(std::move(builder.name))
 , screenRect(std::move(builder.screenRect))
 , status(std::move(builder.status))
 , style(std::move(builder.style))
 , systemCallback(std::move(builder.systemCallback))
 , inputCallback(std::move(builder.inputCallback))
 , tooltipCallback(std::move(builder.tooltipCallback))
 , drawCallback(std::move(builder.drawCallback))
 , font(std::move(builder.font))
 , headerTemplate(std::move(builder.headerTemplate))
 , tooltipRef(std::move(builder.tooltipRef))
 , tooltipDelay(std::move(builder.tooltipDelay))
 , textRef(std::move(builder.textRef))
 , textColor(std::move(builder.textColor))
 , enabled(std::move(builder.enabled))
 , disabled(std::move(builder.disabled))
 , highlight(std::move(builder.highlight))
 , listBoxEnabledUpButton(std::move(builder.listBoxEnabledUpButton))
 , listBoxEnabledDownButton(std::move(builder.listBoxEnabledDownButton))
 , listBoxEnabledSlider(std::move(builder.listBoxEnabledSlider))
 , listBoxDisabledUpButton(std::move(builder.listBoxDisabledUpButton))
 , listBoxDisabledDownButton(std::move(builder.listBoxDisabledDownButton))
 , listBoxDisabledSlider(std::move(builder.listBoxDisabledSlider))
 , listBoxHighlightUpButton(std::move(builder.listBoxHighlightUpButton))
 , listBoxHighlightDownButton(std::move(builder.listBoxHighlightDownButton))
 , listBoxHighlightSlider(std::move(builder.listBoxHighlightSlider))
 , sliderThumbEnabled(std::move(builder.sliderThumbEnabled))
 , sliderThumbDisabled(std::move(builder.sliderThumbDisabled))
 , sliderThumbHighlight(std::move(builder.sliderThumbHighlight))
 , comboBoxDropDownButtonEnabled(std::move(builder.comboBoxDropDownButtonEnabled))
 , comboBoxDropDownButtonDisabled(std::move(builder.comboBoxDropDownButtonDisabled))
 , comboBoxDropDownButtonHighlight(std::move(builder.comboBoxDropDownButtonHighlight))
 , comboBoxEditBoxEnabled(std::move(builder.comboBoxEditBoxEnabled))
 , comboBoxEditBoxDisabled(std::move(builder.comboBoxEditBoxDisabled))
 , comboBoxEditBoxHighlight(std::move(builder.comboBoxEditBoxHighlight))
 , comboBoxListBoxEnabled(std::move(builder.comboBoxListBoxEnabled))
 , comboBoxListBoxDisabled(std::move(builder.comboBoxListBoxDisabled))
 , comboBoxListBoxHighlight(std::move(builder.comboBoxListBoxHighlight))
 , tooltip(std::move(builder.tooltip))
{}

const std::vector<std::shared_ptr<Window>>& Window::getChildren() const {
  return children;
}

OptionalCRef<DrawData> Window::getDisabledDrawData() const {
  return ptrToCOptional(disabled);
}

OptionalCRef<std::string> Window::getDrawCallback() const {
  return optToCRef(drawCallback);
}

OptionalCRef<DrawData> Window::getEnabledDrawData() const {
return ptrToCOptional(enabled);
}

OptionalCRef<Font> Window::getFont() const {
  return ptrToCOptional(font);
}

OptionalCRef<std::string> Window::getHeaderTemplate() const {
  return optToCRef(headerTemplate);
}

OptionalCRef<DrawData> Window::getHighlightDrawData() const {
return ptrToCOptional(highlight);
}

OptionalCRef<std::string> Window::getInputCallback() const {
  return optToCRef(inputCallback);
}

OptionalCRef<std::string> Window::getName() const {
  return optToCRef(name);
}

OptionalCRef<ScreenRect> Window::getScreenRect() const {
  return ptrToCOptional(screenRect);
}

BitField<Window::Status> Window::getStatus()  const {
  return status;
}

Window::Style Window::getStyle()  const {
  return style;
}

OptionalCRef<std::string> Window::getSystemCallback() const {
  return optToCRef(systemCallback);
}

OptionalCRef<std::string> Window::getText() const {
  return optToCRef(textRef);
}

OptionalCRef<TextColor> Window::getTextColor() const {
  return ptrToCOptional(textColor);
}

OptionalCRef<std::string> Window::getTooltipCallback() const {
  return optToCRef(tooltipCallback);
}

std::optional<uint32_t> Window::getTooltipDelay() const {
  return tooltipDelay;
}

Window::Type Window::getType() const {
  return type;
}

}
