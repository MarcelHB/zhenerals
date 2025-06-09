#include <gtest/gtest.h>

#include "../ResourceLoader.h"

namespace ZH {

class ResourceLoaderTest : public testing::Test {
protected:
  ResourceLoader *unit;

public:
  void SetUp() override {
    unit = new ResourceLoader {{
      "tests/resources/ResourceLoader/stuff.big",
      "tests/resources/ResourceLoader/other_stuff.big",
      "tests/resources/ResourceLoader/nope.big"
    }, "."};
  }

  void TearDown() override {
    delete unit;
  }
};

TEST_F(ResourceLoaderTest, precedenceFirstInList) {
  auto result = unit->getFileStream("Data\\cdkey.txt");
  ASSERT_TRUE(result);

  std::array<char, 13> data = {0};
  result->getStream().read(data.data(), 13);
  EXPECT_EQ(std::string {data.data()}, std::string {"1234-5678-90"});
}

TEST_F(ResourceLoaderTest, oneEntry) {
  auto result = unit->getFileStream("no-keyfixed.exe");
  ASSERT_TRUE(result);

  std::array<char, 6> expected = {'M', 'Z', '\x1', '\xA', '\x3', '\x4'};
  std::array<char, 6> data = {0};
  result->getStream().read(data.data(), 6);
  EXPECT_EQ(data, expected);
}

TEST_F(ResourceLoaderTest, otherEntry) {
  auto result = unit->getFileStream("no-cd-fixed.exe");
  ASSERT_TRUE(result);

  std::array<char, 6> expected = {'M', 'Z', '\x1', '\xA', '\x3', '\x4'};
  std::array<char, 6> data = {0};
  result->getStream().read(data.data(), 6);
  EXPECT_EQ(data, expected);
}

TEST_F(ResourceLoaderTest, noEntry) {
  auto result = unit->getFileStream("doesnt-exist.mp3");
  ASSERT_FALSE(result);
}

TEST_F(ResourceLoaderTest, repeatableRead) {
  auto result = unit->getFileStream("no-keyfixed.exe");
  ASSERT_TRUE(result);

  result = unit->getFileStream("no-keyfixed.exe");
  ASSERT_TRUE(result);
  std::array<char, 6> expected = {'M', 'Z', '\x1', '\xA', '\x3', '\x4'};
  std::array<char, 6> data = {0};
  result->getStream().read(data.data(), 6);
  EXPECT_EQ(data, expected);
}

TEST_F(ResourceLoaderTest, findByPrefix) {
  auto it = unit->findByPrefix("no");

  EXPECT_EQ("no-keyfixed.exe", it.key());
  ++it;
  EXPECT_EQ("no-cd-fixed.exe", it.key());
  ++it;
  EXPECT_EQ(unit->cend(), it);
}

TEST_F(ResourceLoaderTest, findByPrefixExactly) {
  auto it = unit->findByPrefix("Data\\cdkey.txt");

  EXPECT_EQ("data\\cdkey.txt", it.key());
  ++it;
  EXPECT_EQ(unit->cend(), it);
}

}

