#include "vugl_sampler.h"

namespace Vugl {

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
  VkSamplerCreateInfo vkSamplerCreateInfo = {};
  vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.anisotropyEnable = VK_TRUE;
  vkSamplerCreateInfo.maxAnisotropy = 4.0f;
  vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
  vkSamplerCreateInfo.compareEnable = VK_FALSE;
  vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  vkSamplerCreateInfo.mipLodBias = 0.0f;
  vkSamplerCreateInfo.minLod = 0.0f;
  vkSamplerCreateInfo.maxLod = 0.0f;

  createSampler(vkSamplerCreateInfo);
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

}
