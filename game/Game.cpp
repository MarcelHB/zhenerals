#include "Game.h"
#include "Logging.h"

namespace ZH {

Game::Game(Config& config, Window& window)
  : config(config)
  , window(window)
{}

bool Game::init() {
  TRACY(ZoneScoped);
  if (!eventDispatcher.init()) {
    ERROR_ZH("Game", "Could not initialize event dispatcher.");
    return false;
  }

  iniResourceLoader =
    std::shared_ptr<ResourceLoader>(new ResourceLoader {{"INIZH.big"}, config.baseDir});

  languageResourceLoader =
    std::shared_ptr<ResourceLoader>(
      new ResourceLoader {{"EnglishZH.big", "ZH_Generals/English.big"}, config.baseDir}
    );

  audioResourceLoader =
    std::shared_ptr<ResourceLoader>(
      new ResourceLoader {{
          "AudioEnglishZH.big"
        , "AudioZH.big"
        , "SpeechEnglishZH.big"
        , "ZH_Generals/Audio.big"
        , "ZH_Generals/AudioEnglish.big"
        , "ZH_Generals/SpeechEnglish.big"
      }, config.baseDir}
    );

  stringLoader = std::make_shared<StringLoader>(*languageResourceLoader);
  if (!stringLoader->load()) {
    ERROR_ZH("Game", "Could not load strings table");
    return false;
  }

  texturesResourceLoader =
    std::shared_ptr<ResourceLoader>(
      // EVAL language
      new ResourceLoader {{
          "TexturesZH.big"
        , "TerrainZH.big"
        , "MapsZH.big"
        , "EnglishZH.big"
        , "ZH_Generals/Textures.big"
        , "ZH_Generals/Terrain.big"
        , "ZH_Generals/Maps.big"
        , "ZH_Generals/English.big"
        }
        , config.baseDir
      }
    );

  textureLookup = std::make_shared<GFX::TextureLookup>(*iniResourceLoader);
  if (!textureLookup->load()) {
    ERROR_ZH("Game", "Could not load textures list");
    return false;
  }

  componentFactory =
    std::make_shared<GUI::ComponentFactory>(*stringLoader, *textureLookup);
  fontManager = std::make_shared<GFX::Font::FontManager>();
  textureCache =
    std::make_shared<GFX::TextureCache>(
        window.getVuglContext()
      , *texturesResourceLoader
      , *fontManager
    );
  windowFactory = std::make_shared<WindowFactory>(config);

  if (!audioBackend.init()) {
    WARN_ZH("Game", "Could not initialize audio");
  }

  audioPlayback =
    std::make_shared<Audio::Playback>(
        audioBackend
      , *iniResourceLoader
      , *audioResourceLoader
    );
  if (!audioPlayback->load()) {
    WARN_ZH("Game", "Could not initialize sound library.");
  }

  overlay = std::make_shared<GUI::Overlay>(config.resolution);

  mainMenu =
    std::make_shared<GUI::Hosts::MainMenu>(
        *windowFactory
      , *componentFactory
      , eventDispatcher
      , config.resolution
    );
  if (!mainMenu->init()) {
    return false;
  }

  overlay->getChildren().push_back(mainMenu->getRootComponent());

  renderListFactory =
    std::make_shared<GUI::Drawing::RenderListFactory>(
        window.getVuglContext()
      , *overlay
      , *textureCache
      , *fontManager
    );

  drawThread = std::thread(Game::draw, this);

  return true;
}

Game::~Game() {
  terminate = true;
  drawThread.join();
}

void Game::loop() {
  while (true) {
    while (auto sdlEvent = window.getEvent()) {
      if (!processEvent(*sdlEvent)) {
        return;
      }
    }
    SDL_Delay(1);
  }
}

bool Game::processEvent(const SDL_Event& event) {
  TRACY(ZoneScoped);
  switch (event.type) {
    case SDL_WINDOWEVENT:
      switch (event.window.event) {
        case SDL_WINDOWEVENT_CLOSE:
          return false;
      }
      break;
  }

  if (event.type == eventDispatcher.mapEventToSDL(GameEvent::EXIT)) {
    return false;
  } else if (event.type == eventDispatcher.mapEventToSDL(GameEvent::PLAY_SFX)) {
    auto stringPtr = reinterpret_cast<std::string*>(event.user.data1);
    audioPlayback->playSoundEffect(*stringPtr);
    delete stringPtr;
    return true;
  }

  return overlay->processEvent(event);
}

void Game::draw(void *obj) {
  auto game = reinterpret_cast<Game*>(obj);
  auto& vuglContext = game->window.getVuglContext();

  Vugl::RenderPassSetup renderPassSetup{vuglContext.getVkSurfaceFormat(), vuglContext.getVkSamplingFlag()};
  auto renderPass = vuglContext.createRenderPass(renderPassSetup);

  if (renderPass.getLastResult() != VK_SUCCESS) {
    ERROR_ZH("Game", "Failed to create render pass!");
    return;
  }

  std::array<VkClearValue, 2> clearColors{};
  clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearColors[1].depthStencil = {1.0f, 0};

  while (true) {
    auto& frame = vuglContext.getNextFrame();
    auto frameIndex = frame.getImageIndex();

    Vugl::CommandBuffer primary {vuglContext.createCommandBuffer(frameIndex)};
    std::shared_ptr<Vugl::CommandBuffer> secondary;

    {
      auto lock = game->overlay->getLock();
      secondary = game->renderListFactory->createRenderList(frameIndex, renderPass);
    }

    if (game->terminate) {
      break;
    }

    primary.beginRendering(renderPass, clearColors);
    primary.executeSecondary(*secondary);
    primary.closeRendering();

    frame.submitAndPresent(primary);
    game->renderListFactory->onQueueSubmitted();
    TRACY(FrameMark);
  }
}

}
