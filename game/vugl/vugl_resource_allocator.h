#ifndef H_VUGL_RESOURCE_ALLOCATOR
#define H_VUGL_RESOURCE_ALLOCATOR

#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace Vugl {

class ResourceAllocator {
  private:
    VkDevice vkDevice;
    VkPhysicalDevice vkPhysicalDevice;
    VmaAllocator allocator;
    VkCommandPool vkCommandPool;

  public:
    ResourceAllocator ();

    ResourceAllocator (
        VkDevice vkDevice
      , VkPhysicalDevice vkPhysicalDevice
      , VmaAllocator allocator
      , VkCommandPool vkCommandPool
    );

    VkResult allocateCommandBuffer (
        VkCommandBuffer& buffer
      , bool secondary = false
    );

    VkResult createVkBuffer (
        VkDeviceSize size
      , VkBufferUsageFlags vkBufferUsageFlags
      , VkMemoryPropertyFlags vkMemPropFlags
      , VkBuffer& vkBuffer
      , VmaAllocation& vmaAllocation
    );

    VkResult createVkImage (
        const VkExtent2D& extent
      , VkSampleCountFlagBits vkSampleCountFlags
      , VkFormat vkFormat
      , VkImageTiling vkImgTiling
      , VkImageUsageFlags vkImgUsageFlags
      , VkMemoryPropertyFlags vkMemProperties
      , VkImage& vkImage
      , VmaAllocation& vmaAllocation
    );

    void destroyVkBuffer (
        VkBuffer buffer
      , VmaAllocation allocation
    );

    void destroyVkImage (
        VkImage image
      , VmaAllocation allocation
    );

    void freeCommandBuffer(VkCommandBuffer& buffer);

    void* mapMemory (VmaAllocation allocation);
    void unmapMemory (VmaAllocation allocation);
};

}

#endif
