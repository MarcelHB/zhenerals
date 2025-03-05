#include "vugl_device_sampler.h"

namespace Vugl {

DeviceSampler::DeviceSampler (DeviceSampler && other)
  : allocator{other.allocator}
  , vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , vkSampler{other.vkSampler}
  , vkImage{other.vkImage}
  , vmaImageAllocation{other.vmaImageAllocation}
  , vkImageView{other.vkImageView}
{
  other.vkImage = VK_NULL_HANDLE;
  other.vmaImageAllocation = VK_NULL_HANDLE;
  other.vkImageView = VK_NULL_HANDLE;
  other.vkSampler = VK_NULL_HANDLE;
}

DeviceSampler::DeviceSampler (
    VkDevice vkDevice
  , ResourceAllocator& allocator
  , uint32_t dimension
  , VkFormat vkFormat
)
  : allocator{allocator}
  , vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , vkSampler{VK_NULL_HANDLE}
  , vkImage{VK_NULL_HANDLE}
  , vmaImageAllocation{VK_NULL_HANDLE}
  , vkImageView{VK_NULL_HANDLE}
{
  VkSamplerCreateInfo vkSamplerCreateInfo = {};
  vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
  vkSamplerCreateInfo.maxAnisotropy = 16.0f;
  vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
  vkSamplerCreateInfo.compareEnable = VK_FALSE;
  vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  vkSamplerCreateInfo.mipLodBias = 0.0f;
  vkSamplerCreateInfo.minLod = 0.0f;
  vkSamplerCreateInfo.maxLod = 0.0f;

  createImage(dimension, vkFormat);
  createSampler(vkSamplerCreateInfo);
}

DeviceSampler::~DeviceSampler () {
  destroy();
}

void DeviceSampler::createImage (uint32_t dimension, VkFormat vkFormat) {
  VkExtent2D extent{dimension, dimension};

  this->vkLastResult =
    allocator.createVkImage(
        extent
      , VK_SAMPLE_COUNT_1_BIT
      , vkFormat
      , VK_IMAGE_TILING_OPTIMAL
      , VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT| VK_IMAGE_USAGE_TRANSFER_DST_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , vkImage
      , vmaImageAllocation
    );

  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  VkImageViewCreateInfo vkImageViewCreateInfo = {};
  vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vkImageViewCreateInfo.image = vkImage;
  vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vkImageViewCreateInfo.format = vkFormat;
  vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  vkImageViewCreateInfo.subresourceRange.levelCount = 1;
  vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  vkImageViewCreateInfo.subresourceRange.layerCount = 1;

  this->vkLastResult =
    vkCreateImageView(
        vkDevice
      , &vkImageViewCreateInfo
      , nullptr
      , &(this->vkImageView)
    );
}

void DeviceSampler::createSampler (const VkSamplerCreateInfo& createInfo) {
  this->vkLastResult =
    vkCreateSampler(
        vkDevice
      , &createInfo
      , nullptr
      , &(this->vkSampler)
    );
}

void DeviceSampler::destroy () {
  vkDestroySampler(vkDevice, vkSampler, nullptr);
  this->vkSampler = VK_NULL_HANDLE;

  vkDestroyImageView(vkDevice, vkImageView, nullptr);
  this->vkImageView = VK_NULL_HANDLE;

  deleteGPUData();
}

void DeviceSampler::deleteGPUData () {
  allocator.destroyVkImage(vkImage, vmaImageAllocation);
  this->vkImage = VK_NULL_HANDLE;
  this->vmaImageAllocation = VK_NULL_HANDLE;
}

VkResult DeviceSampler::getLastResult () const {
  return vkLastResult;
}

VkSampler DeviceSampler::getVkSampler () const {
  return vkSampler;
}

VkImage DeviceSampler::getVkImage () const {
  return vkImage;
}

VkImageView DeviceSampler::getVkImageView () const {
  return vkImageView;
}

}
