#ifndef H_GUI_WND_WINDOW
#define H_GUI_WND_WINDOW

#include <memory>
#include <vector>

#include "../../common.h"
#include "DrawData.h"
#include "Font.h"
#include "ScreenRect.h"
#include "TextColor.h"

namespace ZH::GUI::WND {

class Window {
  public:
    using OptionalDrawData = std::shared_ptr<DrawData>;

    enum class Type {
      COMBOBOX,
      ENTRYFIELD,
      HORZSLIDER,
      PUSHBUTTON,
      RADIOBUTTON,
      SCROLLLISTBOX,
      STATICTEXT,
      TABCONTROL,
      USER,
      VERTSLIDER,
      // custom types
      OVERLAY
    };

    enum class Status : uint32_t {
      NOTHING = 0,
      ACTIVE = 0x1,
      TOGGLE = 0x2,
      DRAGGABLE = 0x4,
      ENABLED = 0x8,
      HIDDEN = 0x10,
      ABOVE = 0x20,
      BELOW = 0x40,
      IMAGE = 0x80,
      TABSTOP = 0x100,
      NO_INPUT = 0x200,
      NO_FOCUS = 0x400,
      DESTROYED = 0x800,
      BORDER = 0x1000,
      SMOOTH_TEXT = 0x2000,
      ONE_LINE = 0x4000,
      NO_FLUSH = 0x8000,
      SEE_THROUGH = 0x10000,
      RIGHT_CLICK = 0x20000,
      WRAP_CENTERED = 0x40000,
      CHECK_LIKE = 0x80000,
      HOTKEY_TEXT = 0x100000,
      USE_OVERLAY_STATES = 0x200000,
      NOT_READY = 0x400000,
      FLASHING = 0x800000,
      ALWAYS_COLOR = 0x1000000,
      ON_MOUSE_DOWN = 0x2000000,
    };

    enum class Style {
      PUSHBUTTON,
      RADIOBUTTON,
      CHECKBOX,
      VERTSLIDER,
      HORZSLIDER,
      SCROLLLISTBOX,
      ENTRYFIELD,
      STATICTEXT,
      PROGRESSBAR,
      USER,
      MOUSETRACK,
      ANIMATED,
      TABSTOP,
      TABCONTROL,
      TABPANE,
      COMBOBOX,
      UNKNOWN
    };

    struct Builder {
      static Status getStatusByString(const std::string_view);
      static Style getStyleByString(const std::string&);
      static Type getTypeByString(const std::string&);

      std::vector<std::shared_ptr<Window>> children;

      Type type;
      std::optional<std::string> name;
      std::shared_ptr<ScreenRect> screenRect;
      BitField<Status> status;
      Style style;
      OptionalString systemCallback;
      OptionalString inputCallback;
      OptionalString tooltipCallback;
      OptionalString drawCallback;
      std::shared_ptr<Font> font;
      OptionalString headerTemplate;

      /* TODO
         std::optional<ListBox> listBox;
         std::optional<ComboBox> comboBox;
         std::optional<Slider> slider;
         std::optional<RadioButton> radioButton;
      */
      OptionalString tooltipRef;
      std::optional<uint32_t> tooltipDelay;
      OptionalString textRef;
      std::shared_ptr<TextColor> textColor;
      /* TODO
         std::optional<std::string> staticTextRef;
         std::optional<std::string> textEntryRef;
         std::optional<TabControl> tabControl;
      */
      OptionalDrawData enabled;
      OptionalDrawData disabled;
      OptionalDrawData highlight;

      OptionalDrawData listBoxEnabledUpButton;
      OptionalDrawData listBoxEnabledDownButton;
      OptionalDrawData listBoxEnabledSlider;
      OptionalDrawData listBoxDisabledUpButton;
      OptionalDrawData listBoxDisabledDownButton;
      OptionalDrawData listBoxDisabledSlider;
      OptionalDrawData listBoxHighlightUpButton;
      OptionalDrawData listBoxHighlightDownButton;
      OptionalDrawData listBoxHighlightSlider;

      OptionalDrawData sliderThumbEnabled;
      OptionalDrawData sliderThumbDisabled;
      OptionalDrawData sliderThumbHighlight;

      OptionalDrawData comboBoxDropDownButtonEnabled;
      OptionalDrawData comboBoxDropDownButtonDisabled;
      OptionalDrawData comboBoxDropDownButtonHighlight;
      OptionalDrawData comboBoxEditBoxEnabled;
      OptionalDrawData comboBoxEditBoxDisabled;
      OptionalDrawData comboBoxEditBoxHighlight;
      OptionalDrawData comboBoxListBoxEnabled;
      OptionalDrawData comboBoxListBoxDisabled;
      OptionalDrawData comboBoxListBoxHighlight;

      /* TODO:
         ImageOffset...
      */
      OptionalString tooltip;
    };

    Window(Builder&& builder);

    const std::vector<std::shared_ptr<Window>>& getChildren() const;
    OptionalCRef<DrawData> getDisabledDrawData() const;
    OptionalCRef<std::string> getDrawCallback() const;
    OptionalCRef<DrawData> getEnabledDrawData() const;
    OptionalCRef<Font> getFont() const;
    OptionalCRef<std::string> getHeaderTemplate() const;
    OptionalCRef<DrawData> getHighlightDrawData() const;
    OptionalCRef<std::string> getInputCallback() const;
    OptionalCRef<std::string> getName() const;
    OptionalCRef<ScreenRect> getScreenRect() const;
    BitField<Status> getStatus() const;
    Style getStyle() const;
    OptionalCRef<std::string> getSystemCallback() const;
    OptionalCRef<std::string> getText() const;
    OptionalCRef<TextColor> getTextColor() const;
    OptionalCRef<std::string> getTooltipCallback() const;
    std::optional<uint32_t> getTooltipDelay() const;
    Type getType() const;
  private:
    std::vector<std::shared_ptr<Window>> children;

    Type type;
    std::optional<std::string> name;
    std::shared_ptr<ScreenRect> screenRect;
    BitField<Status> status;
    Style style;
    OptionalString systemCallback;
    OptionalString inputCallback;
    OptionalString tooltipCallback;
    OptionalString drawCallback;
    std::shared_ptr<Font> font;
    OptionalString headerTemplate;

    /* TODO
       std::optional<ListBox> listBox;
       std::optional<ComboBox> comboBox;
       std::optional<Slider> slider;
       std::optional<RadioButton> radioButton;
    */
    OptionalString tooltipRef;
    std::optional<uint32_t> tooltipDelay;
    OptionalString textRef;
    std::shared_ptr<TextColor> textColor;
    /* TODO
       std::optional<std::string> staticTextRef;
       std::optional<std::string> textEntryRef;
       std::optional<TabControl> tabControl;
    */
    OptionalDrawData enabled;
    OptionalDrawData disabled;
    OptionalDrawData highlight;

    OptionalDrawData listBoxEnabledUpButton;
    OptionalDrawData listBoxEnabledDownButton;
    OptionalDrawData listBoxEnabledSlider;
    OptionalDrawData listBoxDisabledUpButton;
    OptionalDrawData listBoxDisabledDownButton;
    OptionalDrawData listBoxDisabledSlider;
    OptionalDrawData listBoxHighlightUpButton;
    OptionalDrawData listBoxHighlightDownButton;
    OptionalDrawData listBoxHighlightSlider;

    OptionalDrawData sliderThumbEnabled;
    OptionalDrawData sliderThumbDisabled;
    OptionalDrawData sliderThumbHighlight;

    OptionalDrawData comboBoxDropDownButtonEnabled;
    OptionalDrawData comboBoxDropDownButtonDisabled;
    OptionalDrawData comboBoxDropDownButtonHighlight;
    OptionalDrawData comboBoxEditBoxEnabled;
    OptionalDrawData comboBoxEditBoxDisabled;
    OptionalDrawData comboBoxEditBoxHighlight;
    OptionalDrawData comboBoxListBoxEnabled;
    OptionalDrawData comboBoxListBoxDisabled;
    OptionalDrawData comboBoxListBoxHighlight;

    /* TODO:
       ImageOffset...
    */
    OptionalString tooltip;
};

}

#endif
