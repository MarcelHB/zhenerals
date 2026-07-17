// SPDX-License-Identifier: GPL-2.0

#include "vugl_sampler.h"

namespace Vugl {

VkSamplerCreateInfo Sampler::defaultSamplerCreateInfo = {
  .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
  .pNext = nullptr,
  .flags = 0,
  .magFilter = VK_FILTER_LINEAR,
  .minFilter = VK_FILTER_LINEAR,
  .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
  .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  .mipLodBias = 0.0f,
  .anisotropyEnable = VK_TRUE,
  .maxAnisotropy = 4.0f,
  .compareEnable = VK_FALSE,
  .compareOp = VK_COMPARE_OP_ALWAYS,
  .minLod = 0.0f,
  .maxLod = VK_LOD_CLAMP_NONE,
  .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
  .unnormalizedCoordinates = VK_FALSE
};

Sampler::Sampler (Sampler && other)
  : vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , vkSampler{other.vkSampler}
{
  other.vkSampler = VK_NULL_HANDLE;
}

Sampler::Sampler (VkDevice vkDevice)
  : vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , vkSampler{VK_NULL_HANDLE}
{
  createSampler(defaultSamplerCreateInfo);
}

Sampler::Sampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo)
  : Sampler(vkDevice)
{
  createSampler(createInfo);
}

Sampler::~Sampler() {
  destroy();
}

void Sampler::createSampler (const VkSamplerCreateInfo& createInfo) {
  this->vkLastResult =
    vkCreateSampler(
        vkDevice
      , &createInfo
      , nullptr
      , &(this->vkSampler)
    );
}

void Sampler::destroy () {
  vkDestroySampler(vkDevice, vkSampler, nullptr);
  this->vkSampler = VK_NULL_HANDLE;
}

VkResult Sampler::getLastResult () const {
  return vkLastResult;
}

VkSampler Sampler::getVkSampler () const {
  return vkSampler;
}

VkSamplerCreateInfo& Sampler::getDefaultSamplerCreateInfo() {
  return defaultSamplerCreateInfo;
}

void Sampler::setDefaultMaxAnisotropy(float value) {
  defaultSamplerCreateInfo.maxAnisotropy = value;
}

}
