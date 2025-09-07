#include <cstddef>

#include "common.h"

#if defined(USE_TRACY) && !defined(NO_TRACY) && defined(USE_TRACY_MEMORY)

void* operator new(size_t count) {
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);

  return ptr;
}

void operator delete(void* ptr, std::size_t sz) noexcept {
  free(ptr);

  TracyFree(ptr);
}

void operator delete(void* ptr) noexcept {
  free(ptr);

  TracyFree(ptr);
}

#endif
