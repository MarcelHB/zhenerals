#ifndef H_GAME_EVENT_DISPATCHER
#define H_GAME_EVENT_DISPATCHER

#include <cstdint>

#include "common.h"

namespace ZH {

enum class GameEvent : int32_t {
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
    int mapEventToSDL(GameEvent) const;
  private:
    uint32_t eventOffset = 0;
};

}

#endif
