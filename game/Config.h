// SPDX-License-Identifier: GPL-2.0

#ifndef H_OPTIONS
#define H_OPTIONS

#include <filesystem>
#include <optional>

#include "common.h"
#include "Dimensions.h"

namespace ZH {

struct Config {
  Size resolution = {1600, 900};
  std::optional<uint16_t> refreshRate;
#if WIN32
  std::filesystem::path baseDir = "D:/Games/Steam/steamapps/common/Command & Conquer Generals - Zero Hour";
#else
  std::filesystem::path baseDir = "/mnt/shared/Games/Steam/steamapps/common/Command & Conquer Generals - Zero Hour";
#endif
};

}

#endif
