// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_TEXTURE
#define H_VUGL_TEXTURE

#include <memory>

#include <vulkan/vulkan.h>

#include "vugl_resource_allocator.h"
#include "vugl_uploadable_resource.h"

namespace Vugl {

class Texture : public UploadableResource {
  private:
    ResourceAllocator& allocator;

    VkDevice vkDevice;
    VkResult vkLastResult;

    VkExtent2D extent;

    VkBuffer vkStagingBuffer;
    VmaAllocation vmaStagingBufferAllocation;
    VkImage vkTexture;
    VmaAllocation vmaTextureAllocation;
    VkImageView vkTextureView;
  public:
    Texture (Texture &&);
    Texture (VkDevice vkDevice, ResourceAllocator& allocator);
    ~Texture();

    Texture (const Texture&) = delete;
    Texture& operator= (const Texture&) = delete;
    Texture& operator= (Texture&&) = delete;

    void deleteGPUData () override;
    void deleteHostData () override;
    void destroy ();

    VkExtent2D getExtent () const;
    VkResult getLastResult () const;
    VkImage getVkImage () const;
    VkImageView getVkImageView () const;

    VkResult recordUploadCommands (VkCommandBuffer vkCommandBuffer) override;

    template <typename T>
    void createTexture (
        const std::vector<T>& data
      , const VkExtent2D& extent
      , VkFormat vkFormat
    ) {
      if (vkTexture != VK_NULL_HANDLE) {
        this->vkLastResult = VK_ERROR_UNKNOWN;
        return;
      }

      this->extent = extent;
      VkDeviceSize vkTBSize = data.size();

      this->vkLastResult =
        allocator.createVkBuffer(
            vkTBSize
          , VK_BUFFER_USAGE_TRANSFER_SRC_BIT
          , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
          , vkStagingBuffer
          , vmaStagingBufferAllocation
        );

      if (VK_SUCCESS != vkLastResult) {
        return;
      }

      updateTexture(data);

      this->vkLastResult =
        allocator.createVkImage(
            extent
          , VK_SAMPLE_COUNT_1_BIT
          , vkFormat
          , VK_IMAGE_TILING_OPTIMAL
          , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
          , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          , vkTexture
          , vmaTextureAllocation
        );

      if (VK_SUCCESS != vkLastResult) {
        return;
      }

      VkImageViewCreateInfo vkImageViewCreateInfo = {};
      vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      vkImageViewCreateInfo.image = vkTexture;
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
          , &(this->vkTextureView)
        );
    }

    template<typename T>
    void updateTexture (const std::vector<T>& data) {
      void *mappedData = allocator.mapMemory(vmaStagingBufferAllocation);
      std::uninitialized_copy(data.cbegin(), data.cend(), reinterpret_cast<T*>(mappedData));
      allocator.unmapMemory(vmaStagingBufferAllocation);
    }
};

}

#endif
