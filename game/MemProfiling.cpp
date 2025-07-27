#include <atomic>
#include <cstddef>

#include "common.h"

#if defined(USE_TRACY) && !defined(NO_TRACY) && defined(USE_TRACY_MEMORY)

void* allocations[4096] = {0};
std::atomic<std::size_t> j;

void* operator new(size_t count) {
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);

  return ptr;
}

void operator delete(void* ptr, std::size_t sz) noexcept {
  free(ptr);

  TracyFree(ptr);
}

#endif
