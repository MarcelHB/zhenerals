#include <fstream>
#include <unordered_map>

#include <gtest/gtest.h>

#include "../InflatingStream.h"
#include "../formats/Dict.h"

namespace ZH {

TEST(Dict, parsing) {
  std::ifstream stream {"tests/resources/Dict/some.dict", std::ios::binary};
  InflatingStream infStream {stream};

  std::unordered_map<uint32_t, std::string> chunks;
  chunks.emplace(1, std::string {"some_bool"});
  chunks.emplace(2, std::string {"some_int"});
  chunks.emplace(3, std::string {"some_float"});
  chunks.emplace(4, std::string {"some_string"});
  chunks.emplace(5, std::string {"some_wstring"});

  Dict dict;
  EXPECT_EQ(0x3A, dict.parse(chunks, infStream));
  EXPECT_EQ(5, dict.size());

  EXPECT_FALSE(dict.getBool("no_bool"));
  EXPECT_TRUE(dict.getBool("some_bool"));
  EXPECT_TRUE(*dict.getBool("some_bool"));

  EXPECT_FALSE(dict.getInt("no_int"));
  EXPECT_TRUE(dict.getInt("some_int"));
  EXPECT_EQ(266, *dict.getInt("some_int"));

  EXPECT_FALSE(dict.getFloat("no_float"));
  EXPECT_TRUE(dict.getFloat("some_float"));
  EXPECT_EQ(1.25, *dict.getFloat("some_float"));

  EXPECT_FALSE(dict.getString("no_string"));
  EXPECT_TRUE(dict.getString("some_string"));
  EXPECT_EQ("abcde", dict.getString("some_string")->get());

  EXPECT_FALSE(dict.getU16String("no_wstring"));
  EXPECT_TRUE(dict.getU16String("some_wstring"));
  EXPECT_EQ(u"fghij", dict.getU16String("some_wstring")->get());
}

TEST(Dict, iteration) {
  std::ifstream stream {"tests/resources/Dict/some.dict", std::ios::binary};
  InflatingStream infStream {stream};

  std::unordered_map<uint32_t, std::string> chunks;
  chunks.emplace(1, std::string {"some_bool"});
  chunks.emplace(2, std::string {"some_int"});
  chunks.emplace(3, std::string {"some_float"});
  chunks.emplace(4, std::string {"some_string"});
  chunks.emplace(5, std::string {"some_wstring"});

  bool sawBool = false, sawInt = false, sawFloat = false, sawString = false, sawWString = false;

  Dict dict;
  dict.parse(chunks, infStream);

  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    if (it.key() == "some_bool") {
      sawBool = true;
      EXPECT_EQ(Dict::DictType::BOOL, it.type());
    } else if (it.key() == "some_int") {
      sawInt = true;
      EXPECT_EQ(Dict::DictType::INT, it.type());
    } else if (it.key() == "some_float") {
      sawFloat = true;
      EXPECT_EQ(Dict::DictType::FLOAT, it.type());
    } else if (it.key() == "some_string") {
      sawString = true;
      EXPECT_EQ(Dict::DictType::STRING, it.type());
    } else if (it.key() == "some_wstring") {
      sawWString = true;
      EXPECT_EQ(Dict::DictType::U16STRING, it.type());
    }

  }

  EXPECT_TRUE(sawBool && sawInt && sawFloat && sawString && sawWString);
}

}
