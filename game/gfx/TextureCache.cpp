#include "../common.h"
#include "../Logging.h"
#include "../formats/TGAFile.h"
#include "HostTexture.h"
#include "TextureCache.h"

namespace ZH::GFX {

// EVAL language
static std::vector<std::string> texturePrefixes {{
    "data\\english\\art\\textures\\"
  , "art\\textures\\"
  , "art\\terrain\\"
}};

TextureCache::TextureCache(
    Vugl::Context& vuglContext
  , ResourceLoader& textureLoader
  , Font::FontManager& fontManager
  , size_t capacity
)
  : capacity(capacity)
  , vuglContext(vuglContext)
  , textureLoader(textureLoader)
  , fontManager(fontManager)
{
  auto queueOrder = [](TextureMapIt& a, TextureMapIt& b) {
    return a->second.use_count() < b->second.use_count();
  };

  usage = decltype(usage) {queueOrder};
}

std::shared_ptr<Vugl::UploadSampler> TextureCache::getFontTexture(uint8_t size, bool bold) {
  Font::FontKey key {size, bold};
  auto lookup = fontTextures.find(key);
  if (lookup != fontTextures.cend()) {
    return lookup->second;
  }

  auto font = fontManager.getFont(size, bold);
  if (!font) {
    return {};
  }

  auto& texture = font->getTexture()->get();
  auto uploadSampler = vuglContext.createUploadSampler();
  auto textureSize = texture.getSize();

  uploadSampler.createTexture(
      texture.getData()
    , VkExtent2D {textureSize.x, textureSize.y}
    , mappedFormat(texture.getFormat())
  );

  auto cachedSampler =
    std::make_shared<Vugl::UploadSampler>(std::move(uploadSampler));

  auto result = fontTextures.emplace(key, cachedSampler);

  return cachedSampler;
}

std::shared_ptr<Vugl::UploadSampler> TextureCache::getTexture(const std::string& key) {
  TRACY(ZoneScoped);

  std::optional<ResourceLoader::MemoryStream> lookup;

  for (auto& prefix : texturePrefixes) {
    std::string path {prefix};
    path.append(key);

    auto cacheLookup = textures.find(path);
    if (cacheLookup != textures.cend()) {
      return cacheLookup->second;
    }

    lookup = textureLoader.getFileStream(path, true);
    if (lookup) {
      break;
    }
  }

  if (!lookup) {
    WARN_ZH("TextureCache", "Did not find texture: {}", key);
    return {};
  }

  std::shared_ptr<HostTexture> texture;
  if (key.ends_with(".tga")) {
    auto stream = lookup->getStream();
    TGAFile tga {stream};

    texture = tga.getTexture();
  } else {
    WARN_ZH("TextureCache", "Requesting unsupported texture: {}", key);
    return {};
  }

  if (!texture) {
    WARN_ZH("TextureCache", "Failed to load texture: {}", key);
    return {};
  }

  auto uploadSampler = vuglContext.createUploadSampler();
  auto size = texture->getSize();

  uploadSampler.createTexture(
      texture->getData()
    , VkExtent2D {size.x, size.y}
    , mappedFormat(texture->getFormat())
  );

  if (uploadSampler.getLastResult() != VK_SUCCESS) {
    WARN_ZH("TextureCache", "Failed to create Vk texture: {}", key);
    return {};
  }

  auto cachedSampler =
    std::make_shared<Vugl::UploadSampler>(std::move(uploadSampler));

  if (textures.size() >= capacity) {
    tryCleanUpCache();
  }

  auto result = textures.emplace(key, cachedSampler);
  if (result.second) {
    usage.push(result.first);
  }

  return cachedSampler;
}

VkFormat TextureCache::mappedFormat(HostTexture::Format format) {
  switch (format) {
    case ZH::GFX::HostTexture::Format::BGRA8888:
      return VK_FORMAT_B8G8R8A8_UNORM;
    case ZH::GFX::HostTexture::Format::RGBA8888:
      return VK_FORMAT_R8G8B8A8_UNORM;
    default:
      WARN_ZH("TextureCache", "Unmapped format, falling back");
      return VK_FORMAT_R8G8B8A8_UNORM;
  }
}

void TextureCache::tryCleanUpCache() {
  auto& top = usage.top();
  // Drop one element that is not in use r/n
  if (top->second.use_count() == 1) {
    usage.pop();
    textures.erase(top);
  }
}

}
