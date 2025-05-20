#include <SDL3/SDL.h>

#include "EventDispatcher.h"

namespace ZH {

bool EventDispatcher::init() {
  auto offset = SDL_RegisterEvents(static_cast<int>(GameEvent::COUNT));
  if (offset == -1) {
    return false;
  }

  eventOffset = offset;

  return true;
}

void EventDispatcher::fire(GameEvent event) {
  SDL_Event sdlEvent;
  SDL_zero(sdlEvent);
  sdlEvent.type = mapEventToSDL(event);
  SDL_PushEvent(&sdlEvent);
}

void EventDispatcher::fire(GameEvent event, void* data) {
  SDL_Event sdlEvent;
  SDL_zero(sdlEvent);
  sdlEvent.type = mapEventToSDL(event);
  sdlEvent.user.data1 = data;
  SDL_PushEvent(&sdlEvent);
}

int EventDispatcher::mapEventToSDL(GameEvent event) const {
  return eventOffset + static_cast<int32_t>(event);
}

}
