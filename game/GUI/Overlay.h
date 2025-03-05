#ifndef H_GUI_OVERLAY
#define H_GUI_OVERLAY

#include <mutex>

#include <SDL2/SDL.h>

#include "Component.h"

namespace ZH::GUI {

class Overlay : public Component {
  public:
    Overlay(Size);

    bool processEvent(const SDL_Event&);
    std::unique_lock<std::recursive_mutex> getLock();
  private:
    std::recursive_mutex mutex;
};

}

#endif
