// SPDX-License-Identifier: GPL-2.0

#include <cstddef>

#include "common.h"

#if defined(USE_TRACY) && !defined(NO_TRACY) && defined(USE_TRACY_MEMORY)
#include <atomic>
#include <mutex>

// Tracy recommends a locked mutex, but due to use of statically initialized
// containers around, a global mutex may not be initialized yet when it first
// hits `new`, so that's the workaround to have a mutex regardless.

alignas(std::mutex) std::byte mutexMem[sizeof(std::mutex)];
std::mutex* memLock;
std::atomic<uint8_t> memLockSetup;

void* operator new(size_t count) {
  if (!memLock && std::atomic_fetch_add(&memLockSetup, 1) == 0) {
    memLock = std::construct_at(reinterpret_cast<std::mutex*>(mutexMem));
  }

  {
    std::lock_guard lock {*memLock};
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);

    return ptr;
  }
}

void operator delete(void* ptr, std::size_t /*sz*/) noexcept {
  std::lock_guard lock {*memLock};
  free(ptr);

  TracyFree(ptr);
}

void operator delete(void* ptr) noexcept {
  std::lock_guard lock {*memLock};
  free(ptr);

  TracyFree(ptr);
}

#endif
