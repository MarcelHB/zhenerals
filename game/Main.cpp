// SPDX-License-Identifier: GPL-2.0

#include <SDL3/SDL_main.h>

#include "Config.h"
#include "Game.h"
#include "Logger.h"
#include "Window.h"

int main(int /*argc*/, char ** /*argv*/) {
  ZH::Logger logger;
  logger.start();

  LOG_ZH("Main", "Starting Zhenerals");

  ZH::Config config;
  ZH::Window window;

  if (!window.init(config)) {
    ERROR_ZH("Main", "SDL/Vulkan setup failed, terminating.");
    return 1;
  }

  ZH::Game game {config, window};
  if (!game.init()) {
    return 1;
  }

  game.loop();

  LOG_ZH("Main", "Exiting Zhenerals");

  return 0;
}
