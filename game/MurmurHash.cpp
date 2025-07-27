#include "MurmurHash.h"

namespace ZH {

MurmurHash::MurmurHash(uint32_t v) : value(v) {}

bool MurmurHash::operator==(const MurmurHash& other) const {
  return value == other.value;
}

bool MurmurHash::operator!=(const MurmurHash& other) const {
  return !operator==(other);
}

bool MurmurHash::operator==(uint32_t other) const {
  return value == other;
}

void MurmurHash3_32::feed(uint32_t value) {
  value *= 0xCC9E2D51;
  value = (value << 15) | (value >> 17);
  value *= 0x1B873593;

  state ^= value;
  state = (state << 13) | (state >> 19);
  state = state * 5 + 0xE6546B64;
  calls++;
}

void MurmurHash3_32::feed(const std::string& value) {
  const char* c = value.c_str();
  size_t b = value.size() / 4;

  for (size_t i = 0; i < b; ++i) {
    feed(*reinterpret_cast<const uint32_t*>(&c[i * 4]));
  }

  size_t r = value.size() % 4;
  if (r != 0) {
    uint32_t v = 0;

    for (size_t i = 0; i < r; ++i) {
      v |= c[b * 4 + i] << (i * 8);
    }

    feed(v);
  }
}

MurmurHash MurmurHash3_32::getHash() const {
  uint32_t hash = state;
  hash ^= calls * 4;
  hash ^= hash >> 16;
  hash *= 0x85EBCA6B;
  hash ^= hash >> 13;
  hash *= 0xC2B2AE35;
  hash ^= hash >> 16;

  return { hash };
}

}
