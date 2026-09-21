// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_GFX_FRAME_DISPOSABLE
#define H_GAME_GFX_FRAME_DISPOSABLE

#include <cstdint>

#include "common.h"

namespace ZH::GFX {

class FrameDisposable {
  public:
    uint8_t getMisses() const;

    void increaseMiss();
    void decreaseMiss();

  private:
    // EVAL concept
    uint16_t misses = 0;
};

}

#endif
