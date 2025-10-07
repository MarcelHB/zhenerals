// SPDX-License-Identifier: GPL-2.0

#include <filesystem>
#include <fstream>
#include <vector>

#include "Logging.h"
#include "common.h"

namespace ZH {

std::vector<char> readFile(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    WARN_ZH("readFile", "File {} does not exist.", path);
    return {};
  }

  std::vector<char> data;

  std::ifstream stream {path, std::ios::ate | std::ios::binary};
  size_t fileSize = static_cast<size_t>(stream.tellg());
  data.resize(fileSize);

  stream.seekg(0);
  stream.read(data.data(), fileSize);

  return data;
}

}
