#include "vugl_combined_sampler.h"

namespace Vugl {

CombinedSampler::CombinedSampler (CombinedSampler && other)
  : Sampler(std::move(other))
  , texture{std::move(other.texture)}
{
  other.vkSampler = VK_NULL_HANDLE;
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, ResourceAllocator& allocator)
  : Sampler(vkDevice)
  , texture{vkDevice, allocator}
{
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, ResourceAllocator& allocator)
  : Sampler(vkDevice, createInfo)
  , texture{vkDevice, allocator}
{
  createSampler(createInfo);
}

CombinedSampler::~CombinedSampler () {
  destroy();
}

void CombinedSampler::destroy() {
  deleteGPUData();
  deleteHostData();
}

void CombinedSampler::deleteGPUData () {
  texture.deleteGPUData();
}

void CombinedSampler::deleteHostData () {
  texture.deleteHostData();
}

VkExtent2D CombinedSampler::getExtent () const {
  return texture.getExtent();
}

VkImage CombinedSampler::getVkImage () const {
  return texture.getVkImage();
}

VkImageView CombinedSampler::getVkImageView () const {
  return texture.getVkImageView();
}

VkResult CombinedSampler::recordUploadCommands (VkCommandBuffer vkCommandBuffer) {
  return texture.recordUploadCommands(vkCommandBuffer);
}

}
