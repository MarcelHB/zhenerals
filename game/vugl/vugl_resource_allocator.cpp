// SPDX-License-Identifier: GPL-2.0

#include "vugl_resource_allocator.h"

namespace Vugl {

ResourceAllocator::ResourceAllocator ()
  : vkDevice{VK_NULL_HANDLE}
  , vkPhysicalDevice{VK_NULL_HANDLE}
  , allocator{VK_NULL_HANDLE}
{}

ResourceAllocator::ResourceAllocator (
    VkDevice vkDevice
  , VkPhysicalDevice vkPhysicalDevice
  , VmaAllocator allocator
) : vkDevice{vkDevice}
  , vkPhysicalDevice{vkPhysicalDevice}
  , allocator{allocator}
{}

VkResult ResourceAllocator::createVkBuffer (
    VkDeviceSize size
  , BufferType bufferType
  , VkBuffer& vkBuffer
  , VmaAllocation& vmaAllocation
) {
  VkBufferCreateInfo vkBufferCreateInfo = {};
  vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vkBufferCreateInfo.size = size;
  vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocationCreateInfo = {};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

  switch (bufferType) {
    case BufferType::TEXTURE_BUFFER_FOR_UPLOAD:
    case BufferType::VERTEX_BUFFER_FOR_UPLOAD:
      vkBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
      break;
    case BufferType::UNIFORM_BUFFER_HOST_COHERENT:
      vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
      break;
    case BufferType::VERTEX_BUFFER:
      vkBufferCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
          | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      break;
  }

  return
    vmaCreateBuffer(
        allocator
      , &vkBufferCreateInfo
      , &allocationCreateInfo
      , &vkBuffer
      , &vmaAllocation
      , nullptr
    );
}

VkResult ResourceAllocator::createVkImage (
    const VkExtent2D& extent
  , VkSampleCountFlagBits vkSampleCountFlags
  , VkFormat vkFormat
  , VkImageTiling vkImgTiling
  , ImageType imageType
  , VkImage& vkImage
  , VmaAllocation& vmaAllocation
) {
  VkImageCreateInfo vkImageCreateInfo = {};
  vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  vkImageCreateInfo.extent.width = extent.width;
  vkImageCreateInfo.extent.height = extent.height;
  vkImageCreateInfo.extent.depth = 1;
  vkImageCreateInfo.mipLevels = 1;
  vkImageCreateInfo.arrayLayers = 1;
  vkImageCreateInfo.format = vkFormat;
  vkImageCreateInfo.tiling = vkImgTiling;
  vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkImageCreateInfo.samples = vkSampleCountFlags;
  vkImageCreateInfo.flags = 0;

  VmaAllocationCreateInfo allocationCreateInfo = {};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

  switch (imageType) {
    case ImageType::MULTISAMPLER_ATTACHMENT:
      vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
      break;
    case ImageType::STENCIL_ATTACHMENT:
      vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
      break;
    case ImageType::TEXTURE:
      vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
      break;
  }

  return
    vmaCreateImage(
        allocator
      , &vkImageCreateInfo
      , &allocationCreateInfo
      , &vkImage
      , &vmaAllocation
      , nullptr
    );
}

void ResourceAllocator::destroyVkBuffer (VkBuffer vkBuffer, VmaAllocation vmaAllocation) {
  vmaDestroyBuffer(allocator, vkBuffer, vmaAllocation);
}

void ResourceAllocator::destroyVkImage (VkImage vkImage, VmaAllocation vmaAllocation) {
  vmaDestroyImage(allocator, vkImage, vmaAllocation);
}

void* ResourceAllocator::mapMemory (VmaAllocation vmaAllocation) {
  void *mappedData = nullptr;

  vmaMapMemory(allocator, vmaAllocation, &mappedData);

  return mappedData;
}

void ResourceAllocator::unmapMemory (VmaAllocation vmaAllocation) {
  vmaUnmapMemory(allocator, vmaAllocation);
}

}
