#include "MemoryViewStream.h"

namespace ZH {

MemoryViewStream::MemoryViewStream(const char* buffer, size_t size)
  : streamBuffer {buffer, size}
  , std::istream(&streamBuffer)
{}

MemoryStreamBuffer::MemoryStreamBuffer(const char* buffer, size_t size) {
  char* buffer2 = const_cast<char*>(buffer);
  setg(buffer2, buffer2, buffer2 + size);
}

MemoryStreamBuffer::pos_type MemoryStreamBuffer::seekpos(
    MemoryStreamBuffer::pos_type pos
  , std::ios_base::openmode
) {
  if (pos >= 0 && pos < (egptr() - eback())) {
    setg(eback(), eback() + pos, egptr());

    return pos;
  }

  return -1;
}

MemoryStreamBuffer::pos_type MemoryStreamBuffer::seekoff(
    off_type off
  , std::ios_base::seekdir dir
  , std::ios_base::openmode mode
) {
  switch (dir) {
    case std::ios::beg:
      return seekpos(off, mode);
    case std::ios::cur:
      if ((off <= 0 && (gptr() + off) >= eback())
          || (off >= 0 && (gptr() + off) < egptr())) {
        setg(eback(), gptr() + off, egptr());
        return gptr() - eback();
      }
      break;
    case std::ios::end:
      if (off <= 0 && (egptr() + off) >= eback()) {
        setg(eback(), egptr() + off, egptr());
        return egptr() - gptr();
      }
      break;
  }

  return -1;
}

}
