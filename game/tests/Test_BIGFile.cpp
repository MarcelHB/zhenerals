#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "../formats/BIGFile.h"

namespace ZH {

using path_t = std::filesystem::path;

class BIGFileTest : public testing::Test {
protected:
  BIGFile *unit;

public:
  void SetUp() override {
    unit = new BIGFile {"tests/resources/BIGFile/stuff.big"};
    assert(unit->open());
  }

  void TearDown() override {
    delete unit;
  }
};

TEST_F(BIGFileTest, path) {
  EXPECT_EQ("tests/resources/BIGFile/stuff.big", unit->getPath());
}

TEST_F(BIGFileTest, iteration) {
  std::vector<BIGFile::IndexT::key_type> entries;

  for (auto it = unit->cbegin(); it != unit->cend(); ++it) {
    entries.push_back(*it);
  }

  EXPECT_EQ(2, entries.size());
}

TEST_F(BIGFileTest, lookup) {
  auto it = unit->find("data\\cdKey.txt");
  EXPECT_NE(unit->cend(), it);

  auto it2 = unit->find("Data\\cdkey.txt");
  EXPECT_NE(unit->cend(), it2);

  EXPECT_EQ(it, it2);
}

TEST_F(BIGFileTest, extractionText) {
  auto it = unit->find("Data\\cdkey.txt");
  EXPECT_NE(unit->cend(), it);
  EXPECT_EQ(12, it.size());
  EXPECT_EQ("data\\cdkey.txt", it.key());

  std::array<char, 13> data1 = {0};
  unit->extract(it, data1.data(), 0, it.size());
  EXPECT_EQ(std::string {data1.data()}, std::string {"1234-5678-90"});

  std::array<char, 13> data2 = {0};
  unit->extract(it, data2.data(), 0, 4);
  unit->extract(it, data2.data() + 4, 4, 8);
  EXPECT_EQ(std::string {data2.data()}, std::string {"1234-5678-90"});
}

TEST_F(BIGFileTest, extractionBinary) {
  auto it = unit->find("no-cd-fixed.exe");
  EXPECT_NE(unit->cend(), it);
  EXPECT_EQ(6, it.size());

  std::array<char, 6> data = {0};
  std::array<char, 6> expected = {'M', 'Z', '\x1', '\xA', '\x3', '\x4'};
  unit->extract(it, data.data(), 0, it.size());
  EXPECT_EQ(data, expected);
}

}
