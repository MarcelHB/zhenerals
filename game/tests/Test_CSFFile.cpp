#include <fstream>

#include <gtest/gtest.h>

#include "../CSFFile.h"

namespace ZH {

TEST(CSFFile, parsing) {
  std::ifstream stream {"tests/resources/CSFFile/strings.csf", std::ios::binary};
  CSFFile unit {stream};

  auto map = unit.getStrings();
  EXPECT_EQ(3, map.size());

  auto str = u"123";
  auto it = map.find("abc");
  EXPECT_NE(map.cend(), it);
  EXPECT_EQ(u"123", it->second.string);
  EXPECT_FALSE(it->second.soundFile);

  it = map.find("def");
  EXPECT_NE(map.cend(), it);
  EXPECT_EQ(u"789", it->second.string);

  it = map.find("xyz");
  EXPECT_EQ(u"xx", it->second.string);
  EXPECT_EQ("boo", it->second.soundFile);
}

}
