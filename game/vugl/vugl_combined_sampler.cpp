// SPDX-License-Identifier: GPL-2.0

#include "vugl_combined_sampler.h"

namespace Vugl {

CombinedSampler::CombinedSampler (CombinedSampler && other)
  : Sampler(std::move(other))
  , texture{std::move(other.texture)}
{
  other.vkSampler = VK_NULL_HANDLE;
  other.texture.reset();
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, ResourceAllocator& allocator)
  : Sampler(vkDevice)
{
  this->texture = std::make_shared<Vugl::Texture>(vkDevice, allocator);
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, std::shared_ptr<Vugl::Texture>& texture)
  : Sampler(vkDevice)
  , texture{texture}
{
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, ResourceAllocator& allocator)
  : Sampler(vkDevice, createInfo)
{
  createSampler(createInfo);
  this->texture = std::make_shared<Vugl::Texture>(vkDevice, allocator);
}

CombinedSampler::CombinedSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, std::shared_ptr<Vugl::Texture>& texture)
  : Sampler(vkDevice, createInfo)
  , texture{texture}
{
  createSampler(createInfo);
}

void CombinedSampler::deleteGPUData () {
}

void CombinedSampler::deleteHostData () {
}

VkExtent2D CombinedSampler::getExtent () const {
  return texture->getExtent();
}

VkImage CombinedSampler::getVkImage () const {
  return texture->getVkImage();
}

VkImageView CombinedSampler::getVkImageView () const {
  return texture->getVkImageView();
}

VkResult CombinedSampler::recordUploadCommands (VkCommandBuffer vkCommandBuffer) {
  return texture->recordUploadCommands(vkCommandBuffer);
}

}
