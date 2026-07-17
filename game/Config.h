// SPDX-License-Identifier: GPL-2.0

#ifndef H_OPTIONS
#define H_OPTIONS

#include <filesystem>
#include <optional>

#include "common.h"
#include "Dimensions.h"
#include "vugl/vugl_context.h"

namespace ZH {

struct Config {
  Size resolution = {1600, 900};
  bool fullscreen = false;
  std::optional<uint16_t> refreshRate;
  Vugl::MSAASampling msaaSampling = Vugl::MSAASampling::EIGHT;
  float maxAnisotropy = 4.0f;
#if WIN32
  std::filesystem::path baseDir = "D:/Games/Steam/steamapps/common/Command & Conquer Generals - Zero Hour";
#else
  std::filesystem::path baseDir = "/mnt/shared/Games/Steam/steamapps/common/Command & Conquer Generals - Zero Hour";
#endif
};

}

#endif
