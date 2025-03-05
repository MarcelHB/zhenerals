#ifndef H_GAME_EVENT_DISPATCHER
#define H_GAME_EVENT_DISPATCHER

namespace ZH {

enum class GameEvent : int32_t {
    NONE
  , EXIT
  , COUNT
};

class EventDispatcher {
  public:
    bool init();
    void fire(GameEvent);
    int mapEventToSDL(GameEvent) const;
  private:
    uint32_t eventOffset = 0;
};

}

#endif
