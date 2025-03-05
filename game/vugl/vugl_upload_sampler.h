#ifndef H_VUGL_UPLOAD_SAMPLER
#define H_VUGL_UPLOAD_SAMPLER

#include <memory>

#include <vulkan/vulkan.h>

#include "vugl_uploadable_resource.h"
#include "vugl_resource_allocator.h"
#include "vugl_sampler.h"

namespace Vugl {

class UploadSampler : public UploadableResource, public Sampler {
  private:
    ResourceAllocator& allocator;
    VkDevice vkDevice;
    VkResult vkLastResult;

    VkSampler vkSampler;

    VkBuffer vkStagingBuffer;
    VmaAllocation vmaStagingBufferAllocation;
    VkImage vkTexture;
    VmaAllocation vmaTextureAllocation;
    VkImageView vkTextureView;

    VkExtent2D extent;

    void createSampler (const VkSamplerCreateInfo& createInfo);
  public:
    UploadSampler (const UploadSampler&) = delete;
    UploadSampler& operator= (const UploadSampler&) = delete;
    UploadSampler& operator= (UploadSampler&&) = delete;

    UploadSampler (UploadSampler &&);
    UploadSampler (VkDevice vkDevice, ResourceAllocator& allocator);
    UploadSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, ResourceAllocator& allocator);
    ~UploadSampler();

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

    void deleteGPUData () override;
    void deleteHostData () override;
    void destroy ();

    VkExtent2D getExtent () const;
    VkResult getLastResult () const;
    VkSampler getVkSampler () const override;
    VkImage getVkImage () const override;
    VkImageView getVkImageView () const override;

    VkResult recordUploadCommands (VkCommandBuffer vkCommandBuffer) override;

    template<typename T>
    void updateTexture (const std::vector<T>& data) {
      void *mappedData = allocator.mapMemory(vmaStagingBufferAllocation);
      std::uninitialized_copy(data.cbegin(), data.cend(), reinterpret_cast<T*>(mappedData));
      allocator.unmapMemory(vmaStagingBufferAllocation);
    }
};

}

#endif
