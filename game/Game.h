// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME
#define H_GAME

#include <thread>

#include <SDL3/SDL.h>

#include "common.h"
#include "audio/Backend.h"
#include "audio/Playback.h"
#include "BattlefieldFactory.h"
#include "common.h"
#include "Config.h"
#include "EventDispatcher.h"
#include "inis/TerrainINI.h"
#include "inis/WaterINI.h"
#include "ObjectLoader.h"
#include "objects/InstanceFactory.h"
#include "ResourceLoader.h"
#include "StringLoader.h"
#include "Window.h"
#include "WindowFactory.h"
#include "gfx/ModelCache.h"
#include "gfx/TextureCache.h"
#include "gfx/TextureLoader.h"
#include "gfx/TextureLookup.h"
#include "gfx/font/FontManager.h"
#include "GUI/ComponentFactory.h"
#include "GUI/Overlay.h"
#include "GUI/drawing/RenderListFactory.h"
#include "GUI/hosts/MainMenu.h"
#include "rendering/BattlefieldRenderer.h"

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
    std::shared_ptr<BattlefieldFactory> battlefieldFactory;
    std::shared_ptr<GUI::ComponentFactory> componentFactory;
    std::shared_ptr<GFX::Font::FontManager> fontManager;
    std::shared_ptr<ResourceLoader> audioResourceLoader;
    std::shared_ptr<Audio::Playback> audioPlayback;
    std::shared_ptr<ResourceLoader> iniResourceLoader;
    std::shared_ptr<Objects::InstanceFactory> instanceFactory;
    std::shared_ptr<ResourceLoader> languageResourceLoader;
    std::shared_ptr<ResourceLoader> mapsLoader;
    std::shared_ptr<BattlefieldRenderer> mapRenderer;
    std::shared_ptr<GFX::ModelCache> modelCache;
    std::shared_ptr<ResourceLoader> modelLoader;
    std::shared_ptr<ObjectLoader> objectLoader;
    std::shared_ptr<StringLoader> stringLoader;
    std::shared_ptr<ResourceLoader> texturesResourceLoader;
    std::shared_ptr<WindowFactory> windowFactory;
    std::shared_ptr<GFX::TextureCache> textureCache;
    std::shared_ptr<GFX::TextureLoader> textureLoader;
    std::shared_ptr<GFX::TextureLookup> textureLookup;

    std::shared_ptr<GUI::Overlay> overlay;
    std::shared_ptr<GUI::Hosts::MainMenu> mainMenu;
    std::shared_ptr<GUI::Drawing::RenderListFactory> renderListFactory;

    TerrainINI::Terrains terrains;
    WaterINI::WaterSettings waterSettings;

    Window& window;

    static void draw(void*);
    bool processEvent(const SDL_Event&);
};

}

#endif
