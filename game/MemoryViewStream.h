#ifndef H_GAME_MEMORY_VIEW_STREAM
#define H_GAME_MEMORY_VIEW_STREAM

#include <istream>
#include <streambuf>

#include "common.h"

namespace ZH {

class MemoryStreamBuffer : public std::streambuf {
  public:
    MemoryStreamBuffer(const char* buffer, size_t size);

  protected:
    pos_type seekpos(pos_type, std::ios_base::openmode) override;
    pos_type seekoff(
        off_type off
      , std::ios_base::seekdir dir
      , std::ios_base::openmode
    ) override;
};

class MemoryViewStream : public std::istream {
  public:
    MemoryViewStream(const char* buffer, size_t size);
  private:
    MemoryStreamBuffer streamBuffer;
};

}

#endif
