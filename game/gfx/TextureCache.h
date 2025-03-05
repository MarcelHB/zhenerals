#ifndef H_GFX_TEXTURE_CACHE
#define H_GFX_TEXTURE_CACHE

#include <functional>
#include <map>
#include <queue>
#include <vector>

#include "../ResourceLoader.h"
#include "../vugl/vugl_context.h"
#include "HostTexture.h"
#include "font/FontManager.h"

namespace ZH::GFX {

class TextureCache {
  public:
    TextureCache(
        Vugl::Context& vuglContext
      , ResourceLoader& textureLoader
      , Font::FontManager& fontManager
      , size_t capacity = 128
    );

    std::shared_ptr<Vugl::UploadSampler> getFontTexture(uint8_t, bool bold = false);
    std::shared_ptr<Vugl::UploadSampler> getTexture(const std::string&);

  private:
    size_t capacity;
    Vugl::Context& vuglContext;
    ResourceLoader& textureLoader;
    Font::FontManager& fontManager;

    std::map<std::string, std::shared_ptr<Vugl::UploadSampler>> textures;
    std::unordered_map<Font::FontKey, std::shared_ptr<Vugl::UploadSampler>> fontTextures;

    using TextureMapIt = decltype(textures)::const_iterator;
    std::priority_queue<
        TextureMapIt
      , std::vector<TextureMapIt>
      , std::function<bool(TextureMapIt&, TextureMapIt&)>
    > usage;

    VkFormat mappedFormat(HostTexture::Format format);
    void tryCleanUpCache();
};

}

#endif
