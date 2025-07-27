#include <gtest/gtest.h>

#include "../MurmurHash.h"

namespace ZH {

TEST(MurmurHash, String) {
  std::string str {"abcdef"};
  MurmurHash3_32 hasher1;
  hasher1.feed(str);
  auto hash1 = hasher1.getHash();

  MurmurHash3_32 hasher2;
  hasher2.feed(0x64636261);
  hasher2.feed(0x6665);

  auto hash2 = hasher2.getHash();

  EXPECT_EQ(hash1, hash2);
};

}
