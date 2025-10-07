// SPDX-License-Identifier: GPL-2.0

#include <functional>

#include "vugl_descriptor_set.h"

namespace Vugl {

DescriptorSet::DescriptorSet (
    VkDevice vkDevice
  , VkPipelineLayout vkPipelineLayout
  , VkDescriptorSetLayout vkDescriptorSetLayout
  , uint32_t numSwapchainImages
) : vkDevice{vkDevice}
  , vkPipelineLayout{vkPipelineLayout}
  , vkDescriptorSetLayout{vkDescriptorSetLayout}
  , numSwapchainImages{numSwapchainImages}
  , vkPipelineBindPoint{VK_PIPELINE_BIND_POINT_GRAPHICS}
  , vkLastResult{VK_SUCCESS}
  , vkDescriptorPool{VK_NULL_HANDLE}
  , vkDescriptorSets{}
  , assignedCombinedSamplers{}
  , assignedSamplers{}
  , assignedStorageImages{}
  , assignedTextures{}
  , assignedUniformBuffers{}
  , bindings{}
{
  vkLastResult = VK_SUCCESS;
}

DescriptorSet::DescriptorSet (DescriptorSet && other)
  : vkDevice{other.vkDevice}
  , vkPipelineLayout{other.vkPipelineLayout}
  , vkDescriptorSetLayout{other.vkDescriptorSetLayout}
  , numSwapchainImages{other.numSwapchainImages}
  , vkPipelineBindPoint{other.vkPipelineBindPoint}
  , vkLastResult{other.vkLastResult}
  , vkDescriptorPool{other.vkDescriptorPool}
  , vkDescriptorSets{std::move(other.vkDescriptorSets)}
  , assignedCombinedSamplers{std::move(other.assignedCombinedSamplers)}
  , assignedSamplers{std::move(other.assignedSamplers)}
  , assignedStorageImages{std::move(other.assignedStorageImages)}
  , assignedTextures{std::move(other.assignedTextures)}
  , assignedUniformBuffers{std::move(other.assignedUniformBuffers)}
  , bindings{std::move(other.bindings)}
{
  other.vkDescriptorPool = VK_NULL_HANDLE;
}

DescriptorSet::~DescriptorSet () {
  destroy();
}

void DescriptorSet::destroy () {
  if (VK_NULL_HANDLE != vkDescriptorPool) {
    vkResetDescriptorPool(vkDevice, vkDescriptorPool, 0);
  }

  vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, nullptr);
  this->vkDescriptorPool = VK_NULL_HANDLE;

  assignedCombinedSamplers.clear();
  bindings.resize(0);
}

void DescriptorSet::assignCombinedSampler (const CombinedSampler& sampler) {
  assignedCombinedSamplers.emplace_back(std::cref(sampler));
  bindings.emplace_back(DescriptorType::COMBINED_SAMPLER, assignedCombinedSamplers.size() - 1);
}

void DescriptorSet::assignSampler (const Sampler& sampler) {
  assignedSamplers.emplace_back(std::cref(sampler));
  bindings.emplace_back(DescriptorType::SAMPLER, assignedSamplers.size() - 1);
}

void DescriptorSet::assignStorageImage (const Texture& image) {
  assignedStorageImages.emplace_back(std::cref(image));
  bindings.emplace_back(DescriptorType::STORAGE_IMAGE, assignedStorageImages.size() - 1);
}

void DescriptorSet::assignTexture (const Texture& sampler, std::optional<uint32_t> bindingOpt) {
  assignedTextures.emplace_back(std::cref(sampler));

  if (bindingOpt) {
    auto result = textureGrouping.emplace(std::make_pair(*bindingOpt, std::vector<std::reference_wrapper<const Texture>>{}));
    result.first->second.emplace_back(std::cref(sampler));
    if (result.second) {
      bindings.emplace_back(DescriptorType::SAMPLED_IMAGE, assignedTextures.size() - 1);
    }
  } else {
    bindings.emplace_back(DescriptorType::SAMPLED_IMAGE, assignedTextures.size() - 1);
  }
}

void DescriptorSet::assignUniformBuffer (const UniformBuffer& uniformBuffer, bool dynamic) {
  assignedUniformBuffers.emplace_back(std::cref(uniformBuffer));

  if (dynamic) {
    bindings.emplace_back(DescriptorType::DYNAMIC_UBO, assignedUniformBuffers.size() - 1);
  } else {
    bindings.emplace_back(DescriptorType::UBO, assignedUniformBuffers.size() - 1);
  }
}

VkResult DescriptorSet::getLastResult () const {
  return vkLastResult;
}

VkDescriptorSet DescriptorSet::getVkDescriptorSet (size_t index) const {
  return vkDescriptorSets[index];
}

VkResult DescriptorSet::recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) {
  vkCmdBindDescriptorSets(
      vkCommandBuffer
    , vkPipelineBindPoint
    , vkPipelineLayout
    , 0
    , 1
    , &vkDescriptorSets[i]
    , 0
    , nullptr
  );

  return VK_SUCCESS;
}

void DescriptorSet::setPipelineBindPoint (VkPipelineBindPoint bindPoint) {
  this->vkPipelineBindPoint = bindPoint;
}

void DescriptorSet::updateDevice () {
  uint32_t numUBOs = 0;
  uint32_t numDynamicUBOs = 0;

  for (auto& binding : bindings) {
    if (std::get<0>(binding) == DescriptorType::UBO) {
      numUBOs += assignedUniformBuffers[std::get<1>(binding)].get().getNumOfDescriptors();
    } else if (std::get<0>(binding) == DescriptorType::DYNAMIC_UBO) {
      numDynamicUBOs += assignedUniformBuffers[std::get<1>(binding)].get().getNumOfDescriptors();
    }
  }

  if (VK_NULL_HANDLE == vkDescriptorPool) {
    std::vector<VkDescriptorPoolSize> poolSizes{};

    if (assignedCombinedSamplers.size() > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkSamplerDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkSamplerDescPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      vkSamplerDescPoolSize.descriptorCount = numSwapchainImages * (assignedCombinedSamplers.size());
    }

    if (numUBOs > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkDescPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      vkDescPoolSize.descriptorCount = numSwapchainImages * numUBOs;
    }

    if (numDynamicUBOs > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkDescPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      vkDescPoolSize.descriptorCount = numSwapchainImages * numDynamicUBOs;
    }

    if (assignedSamplers.size() > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkSamplerDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkSamplerDescPoolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
      vkSamplerDescPoolSize.descriptorCount = numSwapchainImages * (assignedSamplers.size());
    }

    if (assignedTextures.size() > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkDescPoolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      vkDescPoolSize.descriptorCount = numSwapchainImages * assignedTextures.size();
    }

    if (assignedStorageImages.size() > 0) {
      poolSizes.resize(poolSizes.size() + 1);
      auto& vkDescPoolSize = poolSizes[poolSizes.size() - 1];
      vkDescPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      vkDescPoolSize.descriptorCount = numSwapchainImages * assignedStorageImages.size();
    }

    VkDescriptorPoolCreateInfo vkDescPoolCreateInfo = {};
    vkDescPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescPoolCreateInfo.poolSizeCount = poolSizes.size();
    vkDescPoolCreateInfo.pPoolSizes = poolSizes.data();
    vkDescPoolCreateInfo.maxSets = numSwapchainImages;

    this->vkLastResult =
      vkCreateDescriptorPool(
          vkDevice
        , &vkDescPoolCreateInfo
        , nullptr
        , &(this->vkDescriptorPool)
      );
    if (VK_SUCCESS != vkLastResult) {
      return;
    }

    std::vector<VkDescriptorSetLayout> vkDescSetLayouts {
        numSwapchainImages
      , vkDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo vkDescSetAllocInfo = {};
    vkDescSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescSetAllocInfo.descriptorPool = vkDescriptorPool;
    vkDescSetAllocInfo.descriptorSetCount = numSwapchainImages;
    vkDescSetAllocInfo.pSetLayouts = vkDescSetLayouts.data();

    vkDescriptorSets.resize(numSwapchainImages);
    this->vkLastResult =
      vkAllocateDescriptorSets(
          vkDevice
        , &vkDescSetAllocInfo
        , vkDescriptorSets.data()
      );
    if (VK_SUCCESS != vkLastResult) {
      return;
    }
  }

  std::vector<VkWriteDescriptorSet> writeSets{
    numSwapchainImages * bindings.size()
  };

  std::vector<VkDescriptorImageInfo> combinedSamplerDescriptors{assignedCombinedSamplers.size()};
  std::vector<VkDescriptorImageInfo> samplerDescriptors{assignedSamplers.size()};
  std::vector<VkDescriptorImageInfo> sampledImageDescriptors{assignedTextures.size()};
  std::vector<VkDescriptorImageInfo> storageImageDescriptors{assignedStorageImages.size()};
  std::vector<VkDescriptorBufferInfo> bufferDescriptors{(numUBOs + numDynamicUBOs)};

  for (size_t i = 0; i < numSwapchainImages; ++i) {
    size_t j = 0;
    size_t iCombSamplers = 0, iSamplers = 0, iBuffers = 0, iSampledImages = 0, iStorageImages = 0;

    for (auto& binding : bindings) {
      auto& vkDescriptor = writeSets[i * bindings.size() + j];

      if (std::get<0>(binding) == DescriptorType::COMBINED_SAMPLER) {
        auto& sampler = assignedCombinedSamplers[std::get<1>(binding)].get();

        auto& vkImageInfo = combinedSamplerDescriptors[iCombSamplers];
        vkImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkImageInfo.imageView = sampler.getVkImageView();
        vkImageInfo.sampler = sampler.getVkSampler();

        vkDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkDescriptor.dstSet = vkDescriptorSets[i];
        vkDescriptor.dstBinding = j;
        vkDescriptor.dstArrayElement = 0;
        vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        vkDescriptor.descriptorCount = 1;
        vkDescriptor.pBufferInfo = VK_NULL_HANDLE;
        vkDescriptor.pImageInfo = &vkImageInfo;

        iCombSamplers += 1;
      } else if (std::get<0>(binding) == DescriptorType::SAMPLER) {
        auto& sampler = assignedSamplers[std::get<1>(binding)].get();

        auto& vkImageInfo = samplerDescriptors[iCombSamplers];
        vkImageInfo.sampler = sampler.getVkSampler();

        vkDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkDescriptor.dstSet = vkDescriptorSets[i];
        vkDescriptor.dstBinding = j;
        vkDescriptor.dstArrayElement = 0;
        vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        vkDescriptor.descriptorCount = 1;
        vkDescriptor.pBufferInfo = VK_NULL_HANDLE;
        vkDescriptor.pImageInfo = &vkImageInfo;

        iSamplers += 1;
      } else if (std::get<0>(binding) == DescriptorType::STORAGE_IMAGE) {
        auto& image = assignedStorageImages[std::get<1>(binding)].get();

        auto& vkImageInfo = storageImageDescriptors[iStorageImages];
        vkImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        vkImageInfo.imageView = image.getVkImageView();

        vkDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkDescriptor.dstSet = vkDescriptorSets[i];
        vkDescriptor.dstBinding = j;
        vkDescriptor.dstArrayElement = 0;
        vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        vkDescriptor.descriptorCount = 1;
        vkDescriptor.pBufferInfo = VK_NULL_HANDLE;
        vkDescriptor.pImageInfo = &vkImageInfo;

        iStorageImages += 1;
      } else if (std::get<0>(binding) == DescriptorType::SAMPLED_IMAGE) {
        auto lookup = textureGrouping.find(j);
        if (lookup == textureGrouping.cend()) {
          auto& texture = assignedTextures[std::get<1>(binding)].get();

          auto& vkImageInfo = sampledImageDescriptors[iSampledImages];
          vkImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          vkImageInfo.imageView = texture.getVkImageView();

          vkDescriptor.descriptorCount = 1;
          vkDescriptor.pImageInfo = &vkImageInfo;
          iSampledImages += 1;
        } else {
          auto& list = lookup->second;
          vkDescriptor.descriptorCount = list.size();
          vkDescriptor.pImageInfo = &(sampledImageDescriptors[iSampledImages]);

          for (auto& texture : list) {
            auto& vkImageInfo = sampledImageDescriptors[iSampledImages];
            vkImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vkImageInfo.imageView = texture.get().getVkImageView();
            iSampledImages += 1;
          }
        }

        vkDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkDescriptor.dstSet = vkDescriptorSets[i];
        vkDescriptor.dstBinding = j;
        vkDescriptor.dstArrayElement = 0;
        vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        vkDescriptor.pBufferInfo = VK_NULL_HANDLE;
      } else if (std::get<0>(binding) == DescriptorType::UBO
          || std::get<0>(binding) == DescriptorType::DYNAMIC_UBO) {
        auto& uniformBuffer = assignedUniformBuffers[std::get<1>(binding)].get();
        auto& buffers = uniformBuffer.getBuffers();

        size_t bufferIndex = 0;
        if (buffers.size() > 1) {
          bufferIndex = i;
        }

        for (uint32_t iDescriptors = 0; iDescriptors < uniformBuffer.getNumOfDescriptors(); iDescriptors++) {
          auto& vkDescBufferInfo = bufferDescriptors[iBuffers + iDescriptors];
          VkDeviceSize offset = iDescriptors * uniformBuffer.getStrideSize();
          vkDescBufferInfo.buffer = buffers[bufferIndex];
          vkDescBufferInfo.offset = offset;
          vkDescBufferInfo.range = uniformBuffer.getStrideSize();
        }

        vkDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkDescriptor.dstSet = vkDescriptorSets[i];
        vkDescriptor.dstBinding = j;
        vkDescriptor.dstArrayElement = 0;
        if (std::get<0>(binding) == DescriptorType::DYNAMIC_UBO) {
          vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        } else {
          vkDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        vkDescriptor.descriptorCount = uniformBuffer.getNumOfDescriptors();
        vkDescriptor.pBufferInfo = &(bufferDescriptors[iBuffers]);
        vkDescriptor.pImageInfo = VK_NULL_HANDLE;

        iBuffers += uniformBuffer.getNumOfDescriptors();
      }

      j += 1;
    }
  }

  vkUpdateDescriptorSets(
      vkDevice
    , writeSets.size()
    , writeSets.data()
    , 0
    , nullptr
  );
}

}
