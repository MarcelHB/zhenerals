#ifndef H_INFLATING_STREAM
#define H_INFLATING_STREAM

#include <cstdint>
#include <istream>
#include <vector>

namespace ZH {

class InflatingStream {
  public:
    InflatingStream(std::istream& stream);

    bool eof() const;
    uint32_t getInflatedSize() const;
    uint64_t read(char*, uint64_t);
    void seekg(size_t, std::ios_base::seekdir);
  private:
    enum class CompressionType {
        NONE
      , REFPACK
    };

    std::istream& stream;
    CompressionType compressionType = CompressionType::NONE;
    uint32_t inflatedSize = 0;
    bool broken = false;
    std::vector<char> inflationBuffer;
    size_t readPos = 0;

    void advanceByCompressionHeader();
    uint64_t readRefPack(char*, uint64_t);
    uint32_t decodeRefPack();
};

}

#endif
