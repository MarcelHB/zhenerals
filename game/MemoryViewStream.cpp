// SPDX-License-Identifier: GPL-2.0

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
  setg(eback(), eback() + pos, egptr());

  return pos;
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
      setg(eback(), gptr() + off, egptr());
      return gptr() - eback();
    case std::ios::end:
      setg(eback(), egptr() + off, egptr());
      return egptr() - gptr();
    default:
      return -1;
  }
}

}
