#include <fstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../Map.h"
#include "../inis/WaterINI.h"

namespace ZH {

TEST(WaterINI, parsing) {
  std::ifstream stream {"tests/resources/WaterINI/water.ini", std::ios::binary};

  WaterINI unit {stream};
  auto water = unit.parse();
  ASSERT_EQ(2, water.waterSets.size());

  EXPECT_NE(water.waterSets.cend(),
      water.waterSets.find(Daytime::AFTERNOON));

  auto& waterSetting = water.waterSets.find(Daytime::EVENING)->second;
  EXPECT_EQ("orange_red.tga", waterSetting.skyTexture);
  EXPECT_EQ("purple.tga", waterSetting.waterTexture);

  auto color = waterSetting.v1color;
  EXPECT_EQ(151, color.r);
  EXPECT_EQ(150, color.g);
  EXPECT_EQ(150, color.b);
  EXPECT_EQ(255, color.a);

  color = waterSetting.v2color;
  EXPECT_EQ(150, color.r);
  EXPECT_EQ(150, color.g);
  EXPECT_EQ(153, color.b);
  EXPECT_EQ(255, color.a);

  color = waterSetting.v3color;
  EXPECT_EQ(150, color.r);
  EXPECT_EQ(152, color.g);
  EXPECT_EQ(150, color.b);
  EXPECT_EQ(255, color.a);

  color = waterSetting.v4color;
  EXPECT_EQ(150, color.r);
  EXPECT_EQ(150, color.g);
  EXPECT_EQ(154, color.b);
  EXPECT_EQ(255, color.a);

  color = waterSetting.diffuseColor;
  EXPECT_EQ(12, color.r);
  EXPECT_EQ(13, color.g);
  EXPECT_EQ(14, color.b);
  EXPECT_EQ(15, color.a);

  color = waterSetting.transparentDiffuseColor;
  EXPECT_EQ(20, color.r);
  EXPECT_EQ(30, color.g);
  EXPECT_EQ(40, color.b);
  EXPECT_EQ(50, color.a);

  EXPECT_EQ(0.125f, waterSetting.uPerMs);
  EXPECT_EQ(0.25, waterSetting.vPerMs);
  EXPECT_EQ(0.5, waterSetting.skyTexelsPerUnit);
  EXPECT_EQ(2, waterSetting.waterRepeat);

  auto& t = water.transparency;
  EXPECT_EQ(1.0, t.transparentWaterMinOpacity);
  EXPECT_EQ(2.0, t.transparentWaterDepth);
  EXPECT_EQ("swamp.tga", t.standingWaterTexture);
  EXPECT_FALSE(t.additiveBlending);

  color = t.standingWaterColor;
  EXPECT_EQ(254, color.r);
  EXPECT_EQ(253, color.g);
  EXPECT_EQ(255, color.b);
  EXPECT_EQ(255, color.a);

  color = t.radarWaterColor;
  EXPECT_EQ(120, color.r);
  EXPECT_EQ(130, color.g);
  EXPECT_EQ(255, color.b);
  EXPECT_EQ(255, color.a);
}

}
