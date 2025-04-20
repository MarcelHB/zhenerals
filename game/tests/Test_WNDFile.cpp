#include <fstream>

#include <gtest/gtest.h>

#include "../formats/WNDFile.h"

namespace ZH {

static void verifyDrawData(const GUI::WND::DrawData& drawData) {
  for (uint8_t i = 0; i < 9; ++i) {
    auto& entry = drawData.getEntries()[i];

    if (i == 0) {
      auto c = entry.color;
      EXPECT_EQ(250, c.r); EXPECT_EQ(250, c.g); EXPECT_EQ(250, c.b); EXPECT_EQ(250, c.a);
      c = entry.borderColor;
      EXPECT_EQ(240, c.r); EXPECT_EQ(240, c.g); EXPECT_EQ(240, c.b); EXPECT_EQ(240, c.a);
    } else {
      EXPECT_FALSE(entry.image);
      auto c = entry.color;
      EXPECT_EQ(251, c.r); EXPECT_EQ(252, c.g); EXPECT_EQ(255, c.b); EXPECT_EQ(254, c.a);
      c = entry.borderColor;
      EXPECT_EQ(255, c.r); EXPECT_EQ(255, c.g); EXPECT_EQ(253, c.b); EXPECT_EQ(255, c.a);
    }
  }
}

TEST(WNDFileTest, parsing) {
  std::ifstream stream {"tests/resources/WNDFile/v2.wnd", std::ios::binary};
  WNDFile unit {stream};

  auto gui = unit.parse();
  ASSERT_TRUE(gui);
  auto layout = gui->getLayout();
  ASSERT_TRUE(layout);

  EXPECT_EQ("abc", layout->get().getInitFunction()->get());
  EXPECT_EQ("123", layout->get().getUpdateFunction()->get());
  EXPECT_EQ("test_this", layout->get().getShutdownFunction()->get());

  auto& window = gui->getWindow();
  EXPECT_EQ(GUI::WND::Window::Type::USER, window.getType());
  EXPECT_EQ("v2.wnd:Test", window.getName()->get());
  EXPECT_EQ(BitField(GUI::WND::Window::Status::ENABLED) | GUI::WND::Window::Status::IMAGE, window.getStatus());
  EXPECT_EQ(GUI::WND::Window::Style::USER, window.getStyle());
  EXPECT_EQ("sys_callback", window.getSystemCallback()->get());
  EXPECT_EQ("input", window.getInputCallback()->get());
  EXPECT_FALSE(window.getTooltipCallback());
  EXPECT_EQ("3D 3D", window.getDrawCallback()->get());
  EXPECT_EQ("GUI:click_this", window.getText()->get());
  EXPECT_FALSE(window.getHeaderTemplate());
  EXPECT_FALSE(window.getTooltipDelay());

  auto screenRect = window.getScreenRect();
  ASSERT_TRUE(screenRect);
  auto p = screenRect->get().getUpperLeft();
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
  p = screenRect->get().getBottomRight();
  EXPECT_EQ(3, p.x);
  EXPECT_EQ(4, p.y);
  auto size = screenRect->get().getResolution();
  EXPECT_EQ(120, size.x);
  EXPECT_EQ(600, size.y);

  auto font = window.getFont();
  ASSERT_TRUE(font);
  EXPECT_EQ("Red Blue", font->get().getName());
  EXPECT_EQ(14, font->get().getSize());
  EXPECT_FALSE(font->get().isBold());

  auto textColor = window.getTextColor();
  ASSERT_TRUE(textColor);
  auto c = *textColor->get().getEnabled();
  EXPECT_EQ(251, c.r); EXPECT_EQ(255, c.g); EXPECT_EQ(255, c.b); EXPECT_EQ(255, c.a);
  c = *textColor->get().getEnabledBorder();
  EXPECT_EQ(255, c.r); EXPECT_EQ(252, c.g); EXPECT_EQ(255, c.b); EXPECT_EQ(255, c.a);
  c = *textColor->get().getDisabled();
  EXPECT_EQ(255, c.r); EXPECT_EQ(255, c.g); EXPECT_EQ(253, c.b); EXPECT_EQ(255, c.a);
  c = *textColor->get().getDisabledBorder();
  EXPECT_EQ(255, c.r); EXPECT_EQ(255, c.g); EXPECT_EQ(255, c.b); EXPECT_EQ(254, c.a);
  c = *textColor->get().getHighlight();
  EXPECT_EQ(255, c.r); EXPECT_EQ(251, c.g); EXPECT_EQ(255, c.b); EXPECT_EQ(255, c.a);
  c = *textColor->get().getHighlightBorder();
  EXPECT_EQ(255, c.r); EXPECT_EQ(255, c.g); EXPECT_EQ(252, c.b); EXPECT_EQ(255, c.a);

  EXPECT_EQ("GUI:click_this", window.getText()->get());

  auto drawData = window.getEnabledDrawData();
  ASSERT_TRUE(drawData);
  EXPECT_EQ("Image1", drawData->get().getEntries()[0].image);
  verifyDrawData(*drawData);

  drawData = window.getDisabledDrawData();
  ASSERT_TRUE(drawData);
  EXPECT_EQ("Image2", drawData->get().getEntries()[0].image);
  verifyDrawData(*drawData);

  drawData = window.getHighlightDrawData();
  ASSERT_TRUE(drawData);
  EXPECT_EQ("Image3", drawData->get().getEntries()[0].image);
  verifyDrawData(*drawData);

  auto& children = window.getChildren();
  EXPECT_EQ(2, children.size());
  EXPECT_EQ("child1", children[0]->getName()->get());
  EXPECT_EQ("child2", children[1]->getName()->get());

  auto& subChildren = children[0]->getChildren();
  EXPECT_EQ("child1_1", subChildren[0]->getName()->get());
}

}
