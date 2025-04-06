#ifndef H_GAME
#define H_GAME

#include <thread>

#include <SDL2/SDL.h>

#include "audio/Backend.h"
#include "common.h"
#include "Config.h"
#include "EventDispatcher.h"
#include "ResourceLoader.h"
#include "StringLoader.h"
#include "Window.h"
#include "WindowFactory.h"
#include "gfx/TextureCache.h"
#include "gfx/TextureLookup.h"
#include "gfx/font/FontManager.h"
#include "GUI/ComponentFactory.h"
#include "GUI/Overlay.h"
#include "GUI/drawing/RenderListFactory.h"
#include "GUI/hosts/MainMenu.h"

namespace ZH {

class Game {
  public:
    Game(Config&, Window&);
    Game(const Game&) = delete;
    ~Game();

    bool init();
    void loop();
  private:
    bool terminate = false;
    Config& config;

    std::thread drawThread;

    EventDispatcher eventDispatcher;
    Audio::Backend audioBackend;
    std::shared_ptr<GUI::ComponentFactory> componentFactory;
    std::shared_ptr<GFX::Font::FontManager> fontManager;
    std::shared_ptr<ResourceLoader> iniResourceLoader;
    std::shared_ptr<ResourceLoader> languageResourceLoader;
    std::shared_ptr<StringLoader> stringLoader;
    std::shared_ptr<ResourceLoader> texturesResourceLoader;
    std::shared_ptr<WindowFactory> windowFactory;
    std::shared_ptr<GFX::TextureCache> textureCache;
    std::shared_ptr<GFX::TextureLookup> textureLookup;

    std::shared_ptr<GUI::Overlay> overlay;
    std::shared_ptr<GUI::Hosts::MainMenu> mainMenu;
    std::shared_ptr<GUI::Drawing::RenderListFactory> renderListFactory;

    Window& window;

    static void draw(void*);
    bool processEvent(const SDL_Event&);
};

}

#endif
