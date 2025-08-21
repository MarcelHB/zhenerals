#ifndef H_GFX_TEXTURE_CACHE
#define H_GFX_TEXTURE_CACHE

#include <functional>
#include <map>
#include <queue>
#include <vector>

#include "../common.h"
#include "../Cache.h"
#include "../vugl/vugl_context.h"
#include "HostTexture.h"
#include "TextureCache.h"
#include "TextureLoader.h"
#include "font/FontManager.h"

namespace ZH::GFX {

class TextureCache {
  public:
    TextureCache(
        Vugl::Context& vuglContext
      , TextureLoader& textureLoader
      , Font::FontManager& fontManager
    );

    std::shared_ptr<Vugl::CombinedSampler> getFontTextureSampler(uint8_t, bool bold = false);
    // not cached right now, to be done by the user
    std::shared_ptr<Vugl::Texture> getTexture(const std::string& key);
    std::shared_ptr<Vugl::CombinedSampler> getTextureSampler(const std::string&);

  private:
    Cache<Vugl::CombinedSampler> textureCache;
    std::unordered_map<Font::FontKey, std::shared_ptr<Vugl::CombinedSampler>> fontTextures;
    Vugl::Context& vuglContext;
    TextureLoader& textureLoader;
    Font::FontManager& fontManager;

    VkFormat mappedFormat(HostTexture::Format format);
};

}

#endif
