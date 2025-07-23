#include "FrameDisposable.h"

namespace ZH::GFX {

uint8_t FrameDisposable::getMisses() const {
  return misses;
}

void FrameDisposable::increaseMiss() {
  misses += 1;
}

void FrameDisposable::decreaseMiss() {
  if (misses > 0) {
    misses -= 1;
  }
}

}
