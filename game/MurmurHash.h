#ifndef H_MURMUR_HASH
#define H_MURMUR_HASH

#include <cstdint>
#include <string>

namespace ZH {

struct MurmurHash {
  uint32_t value = 0;

  MurmurHash() = default;
  MurmurHash(uint32_t value);
  bool operator==(const MurmurHash& other) const;
  bool operator!=(const MurmurHash& other) const;
  bool operator==(uint32_t value) const;

  operator uint32_t() const { return value; }
};

class MurmurHash3_32 {
public:
  MurmurHash3_32() {};

  void feed(uint32_t value);
  void feed(const std::string& value);
  MurmurHash getHash() const;

private:
  uint32_t state = 0;
  uint32_t calls = 0;
};

}

#endif
