#ifndef H_MURMUR_HASH
#define H_MURMUR_HASH

#include <cstdint>

namespace ZH {

struct MurmurHash {
  uint32_t value = 0;

  MurmurHash() = default;
  MurmurHash(uint32_t value);
  bool operator==(const MurmurHash& other) const;
  bool operator!=(const MurmurHash& other) const;
  bool operator==(uint32_t value) const;
};

class MurmurHash3_32 {
public:
  MurmurHash3_32() {};

  void feed(uint32_t value);
  MurmurHash getHash() const;

private:
  uint32_t state = 0;
  uint32_t calls = 0;
};

}

#endif
