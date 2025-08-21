#include <vector>

#include "../common.h"
#include "../Logging.h"
#include "../formats/DDSFile.h"
#include "../formats/TGAFile.h"
#include "TextureLoader.h"

namespace ZH::GFX {

// EVAL language
static std::vector<std::string> texturePrefixes {{
    "data\\english\\art\\textures\\"
  , "art\\textures\\"
  , "art\\terrain\\"
}};

TextureLoader::TextureLoader(ResourceLoader& resourceLoader)
  : resourceLoader(resourceLoader)
{}

std::shared_ptr<HostTexture> TextureLoader::getTexture(std::string key) {
  TRACY(ZoneScoped);

  std::optional<ResourceLoader::MemoryStream> lookup;

  for (auto& prefix : texturePrefixes) {
    std::string path {prefix};
    path.append(key);

    lookup = resourceLoader.getFileStream(path, true);
    if (lookup) {
      break;
    }

    // Some tga files are actually dds
    if (path.ends_with(".tga")) {
      path.replace(path.size() - 3, 3, "dds");
      lookup = resourceLoader.getFileStream(path, true);

      if (lookup) {
        key.replace(key.size() - 3, 3, "dds");
        break;
      }
    }
  }

  if (!lookup) {
    WARN_ZH("TextureCache", "Did not find texture: {}", key);
    return {};
  }

  std::shared_ptr<HostTexture> hostTexture;
  auto stream = lookup->getStream();

  if (key.ends_with(".tga")) {
    TGAFile tga {stream};

    hostTexture = tga.getTexture();
  } else if (key.ends_with(".dds")) {
    DDSFile dds {stream};

    hostTexture = dds.getTexture();
  } else {
    WARN_ZH("TextureCache", "Requesting unsupported texture: {}", key);
    return {};
  }

  if (!hostTexture) {
    WARN_ZH("TextureCache", "Failed to load texture: {}", key);
    return {};
  }

  return hostTexture;
}

}
