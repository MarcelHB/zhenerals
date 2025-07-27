#ifndef H_GUI_OVERLAY
#define H_GUI_OVERLAY

#include <mutex>

#include <SDL3/SDL.h>

#include "../common.h"
#include "../Battlefield.h"
#include "Component.h"

namespace ZH::GUI {

class Overlay : public Component {
  public:
    Overlay(Size);

    std::shared_ptr<Battlefield> getBattlefield() const;
    std::unique_lock<std::recursive_mutex> getLock();
    bool processEvent(const SDL_Event&);
    void setBattlefield(std::shared_ptr<Battlefield>);
  private:
    std::recursive_mutex mutex;
    std::shared_ptr<Battlefield> battlefield;
};

}

#endif
