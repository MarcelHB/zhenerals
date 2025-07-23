#ifndef H_GAME_GFX_FRAME_DISPOSABLE
#define H_GAME_GFX_FRAME_DISPOSABLE

#include <cstdint>

namespace ZH::GFX {

class FrameDisposable {
  public:
    uint8_t getMisses() const;

    void increaseMiss();
    void decreaseMiss();

  private:
    uint8_t misses = 0;
};

}

#endif
