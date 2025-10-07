// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_TEXTURE_LOADER
#define H_GFX_TEXTURE_LOADER

#include <memory>
#include <string>

#include "../ResourceLoader.h"
#include "HostTexture.h"

namespace ZH::GFX {

class TextureLoader {
  public:
    TextureLoader(ResourceLoader& resourceLoader);
    std::shared_ptr<HostTexture> getTexture(std::string key);

  private:
    ResourceLoader& resourceLoader;
};

}

#endif
