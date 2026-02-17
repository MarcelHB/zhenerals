// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_EVENT_DISPATCHER
#define H_GAME_EVENT_DISPATCHER

#include <cstdint>

#include "common.h"

namespace ZH {

enum class GameEvent : uint32_t {
    NONE
  , EXIT
  , PLAY_SFX
  , COUNT
};

class EventDispatcher {
  public:
    bool init();
    void fire(GameEvent);
    void fire(GameEvent, void*);
    uint32_t mapEventToSDL(GameEvent) const;
  private:
    uint32_t eventOffset = 0;
};

}

#endif
