#include <vector>

#include <gtest/gtest.h>

#include "../MemoryViewStream.h"

namespace ZH {

TEST(MemoryViewStream, Reading) {
  std::vector<char> buffer;
  buffer.resize(8);

  for (uint8_t i = 0; i < 8; ++i) {
    buffer[i] = i;
  }

  char readBuf[8];
  MemoryViewStream s {buffer.data(), buffer.size()};
  s.read(readBuf, 8);

  EXPECT_EQ(8, s.gcount());

  s.read(readBuf, 1);
  EXPECT_EQ(0, s.gcount());

  for (uint8_t i = 0; i < 8; ++i) {
    EXPECT_EQ(i, readBuf[i]);
  }
}

TEST(MemoryViewStream, Seeking) {
  std::vector<char> buffer;
  buffer.resize(8);

  for (uint8_t i = 0; i < 8; ++i) {
    buffer[i] = i;
  }

  MemoryViewStream s {buffer.data(), buffer.size()};

  s.seekg(1, std::ios::beg);
  EXPECT_EQ(1, s.tellg());
  s.seekg(3, std::ios::cur);
  EXPECT_EQ(4, s.tellg());

  char readBuf[5];
  s.read(readBuf, 4);
  EXPECT_FALSE(s.fail());
  EXPECT_EQ(4, s.gcount());

  for (uint8_t i = 0; i < 4; ++i) {
    EXPECT_EQ(i + 4, readBuf[i]);
  }

  s.seekg(-5, std::ios::end);
  EXPECT_EQ(3, s.tellg());

  s.read(readBuf, 5);
  EXPECT_EQ(5, s.gcount());

  for (uint8_t i = 0; i < 5; ++i) {
    EXPECT_EQ(i + 3, readBuf[i]);
  }
}

TEST(MemoryViewStream, EndOfFile) {
  std::vector<char> buffer;
  buffer.resize(8);

  MemoryViewStream s {buffer.data(), buffer.size()};
  char readBuf[8];

  s.seekg(0, std::ios::end);
  EXPECT_EQ(8, s.tellg());
  s.read(readBuf, 5);
  EXPECT_EQ(0, s.gcount());
  EXPECT_TRUE(s.fail());
  EXPECT_TRUE(s.eof());
}

}
