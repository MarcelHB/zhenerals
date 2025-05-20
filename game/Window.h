#ifndef H_WINDOW
#define H_WINDOW

#include <functional>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "common.h"
#include "Config.h"
#include "Dimensions.h"
#include "vugl/vugl_context.h"

namespace ZH {

class Window {
  public:
    Window (const Config& config);
    Window (const Window&) = delete;
    ~Window();

    Vugl::Context& getVuglContext();
    bool init();
    OptionalCRef<SDL_Event> getEvent();
  private:
    SDL_Event event;
    Size resolution;
    SDL_Window *sdlWindow = nullptr;
    std::shared_ptr<Vugl::Context> vuglContext;
};

}

#endif
