#include "../common.h"
#include "../Logging.h"
#include "HostTexture.h"
#include "TextureCache.h"
#include "TextureLoader.h"

namespace ZH::GFX {

// EVAL split into lookup and Vk, since repeating, and for testing
TextureCache::TextureCache(
    Vugl::Context& vuglContext
  , TextureLoader& textureLoader
  , Font::FontManager& fontManager
) : vuglContext(vuglContext)
  , textureLoader(textureLoader)
  , fontManager(fontManager)
{ }

std::shared_ptr<Vugl::CombinedSampler> TextureCache::getFontTextureSampler(uint8_t size, bool bold) {
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
  auto uploadSampler = vuglContext.createCombinedSampler();
  auto textureSize = texture.getSize();

  uploadSampler.createTexture(
      texture.getData()
    , VkExtent2D {textureSize.x, textureSize.y}
    , mappedFormat(texture.getFormat())
  );

  auto cachedSampler =
    std::make_shared<Vugl::CombinedSampler>(std::move(uploadSampler));

  auto result = fontTextures.emplace(key, cachedSampler);

  return cachedSampler;
}

std::shared_ptr<Vugl::Texture> TextureCache::getTexture(const std::string& key) {
  TRACY(ZoneScoped);

  auto hostTexture = textureLoader.getTexture(key);
  if (!hostTexture) {
    return {};
  }

  auto texture = vuglContext.createTexture();
  auto size = hostTexture->getSize();

  texture.createTexture(
      hostTexture->getData()
    , VkExtent2D {size.x, size.y}
    , mappedFormat(hostTexture->getFormat())
  );

  if (texture.getLastResult() != VK_SUCCESS) {
    WARN_ZH("TextureCache", "Failed to create Vk texture: {}", key);
    return {};
  }

  return std::make_shared<Vugl::Texture>(std::move(texture));
}

std::shared_ptr<Vugl::CombinedSampler> TextureCache::getTextureSampler(const std::string& key) {
  TRACY(ZoneScoped);

  auto lookup = textureCache.get(key);
  if (lookup) {
    return lookup;
  }

  auto hostTexture = textureLoader.getTexture(key);
  if (!hostTexture) {
    return {};
  }

  auto uploadSampler = vuglContext.createCombinedSampler();
  auto size = hostTexture->getSize();

  uploadSampler.createTexture(
      hostTexture->getData()
    , VkExtent2D {size.x, size.y}
    , mappedFormat(hostTexture->getFormat())
  );

  if (uploadSampler.getLastResult() != VK_SUCCESS) {
    WARN_ZH("TextureCache", "Failed to create Vk texture: {}", key);
    return {};
  }

  auto cachedSampler =
    std::make_shared<Vugl::CombinedSampler>(std::move(uploadSampler));
  textureCache.put(key, cachedSampler);

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

}
