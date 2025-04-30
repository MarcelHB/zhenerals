#include "vugl_resource_allocator.h"

namespace Vugl {

ResourceAllocator::ResourceAllocator ()
  : vkDevice{VK_NULL_HANDLE}
  , vkPhysicalDevice{VK_NULL_HANDLE}
  , allocator{VK_NULL_HANDLE}
  , vkCommandPool{VK_NULL_HANDLE}
{}

ResourceAllocator::ResourceAllocator (
    VkDevice vkDevice
  , VkPhysicalDevice vkPhysicalDevice
  , VmaAllocator allocator
  , VkCommandPool vkCommandPool
) : vkDevice{vkDevice}
  , vkPhysicalDevice{vkPhysicalDevice}
  , allocator{allocator}
  , vkCommandPool{vkCommandPool}
{}

VkResult ResourceAllocator::allocateCommandBuffer (VkCommandBuffer& buffer, bool secondary) {
  VkCommandBufferAllocateInfo vkCommandBufferAllocInfo = {};
  vkCommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  vkCommandBufferAllocInfo.commandPool = vkCommandPool;
  vkCommandBufferAllocInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  vkCommandBufferAllocInfo.commandBufferCount = 1;

  return
    vkAllocateCommandBuffers(
        vkDevice
      , &vkCommandBufferAllocInfo
      , &buffer
    );
}

VkResult ResourceAllocator::createVkBuffer (
    VkDeviceSize size
  , VkBufferUsageFlags vkBufferUsageFlags
  , VkMemoryPropertyFlags vkMemProperties
  , VkBuffer& vkBuffer
  , VmaAllocation& vmaAllocation
) {
  VkBufferCreateInfo vkBufferCreateInfo = {};
  vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vkBufferCreateInfo.size = size;
  vkBufferCreateInfo.usage = vkBufferUsageFlags;
  vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocationCreateInfo = {};
  if (vkMemProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  } else {
    if ((VK_BUFFER_USAGE_TRANSFER_SRC_BIT & vkBufferUsageFlags)
        && (vkMemProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    ) {
      allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    } else {
      allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    }
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
  , VkImageUsageFlags vkImgUsageFlags
  , VkMemoryPropertyFlags vkMemProperties
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
  vkImageCreateInfo.usage = vkImgUsageFlags;
  vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkImageCreateInfo.samples = vkSampleCountFlags;
  vkImageCreateInfo.flags = 0;

  VmaAllocationCreateInfo allocationCreateInfo = {};
  if (vkMemProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  } else {
    if ((VK_BUFFER_USAGE_TRANSFER_SRC_BIT & vkImgUsageFlags)
        && (vkMemProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    ) {
      allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    } else {
      allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    }
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

void ResourceAllocator::freeCommandBuffer(VkCommandBuffer& buffer) {
  vkFreeCommandBuffers(
      vkDevice
    , vkCommandPool
    , 1
    , &buffer
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
