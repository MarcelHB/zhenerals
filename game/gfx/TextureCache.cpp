// SPDX-License-Identifier: GPL-2.0

#include "common.h"
#include "Logging.h"
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

std::shared_ptr<Vugl::Texture> TextureCache::getTexture(const std::string& key, bool mipMaps) {
  TRACY(ZoneScoped);

  auto textureLookup = textureCache.get(key);
  if (textureLookup) {
    return textureLookup;
  }

  auto hostTexture = textureLoader.getTexture(key);
  if (!hostTexture) {
    return {};
  }

  auto texture = vuglContext.createTexture();
  auto size = hostTexture->getSize();

  uint32_t numMipMaps = 1;
  if (mipMaps) {
    numMipMaps = static_cast<uint32_t>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;
  }

  texture.createTexture(
      hostTexture->getData()
    , VkExtent2D {size.x, size.y}
    , mappedFormat(hostTexture->getFormat())
    , numMipMaps
  );

  auto cachedTexture =
    std::make_shared<Vugl::Texture>(std::move(texture));
  textureCache.put(key, cachedTexture);

  return cachedTexture;
}

std::shared_ptr<Vugl::CombinedSampler> TextureCache::getTextureSampler(const std::string& key , bool mipMaps) {
  TRACY(ZoneScoped);

  auto samplerLookup = samplerCache.get(key);
  if (samplerLookup) {
    return samplerLookup;
  }

  auto texture = getTexture(key, mipMaps);
  if (!texture) {
    return {};
  }

  auto uploadSampler = vuglContext.createCombinedSampler(texture);
  auto cachedSampler =
    std::make_shared<Vugl::CombinedSampler>(std::move(uploadSampler));
  samplerCache.put(key, cachedSampler);

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
