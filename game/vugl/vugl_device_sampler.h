#ifndef H_VUGL_DEVICE_SAMPLER
#define H_VUGL_DEVICE_SAMPLER

#include <vulkan/vulkan.h>

#include "vugl_resource_allocator.h"
#include "vugl_sampler.h"

namespace Vugl {

class DeviceSampler : public Sampler {
  private:
    ResourceAllocator& allocator;
    VkDevice vkDevice;
    VkResult vkLastResult;

    VkSampler vkSampler;

    VkImage vkImage;
    VmaAllocation vmaImageAllocation;
    VkImageView vkImageView;

    void createImage (uint32_t dimension, VkFormat vkFormat);
    void createSampler (const VkSamplerCreateInfo& createInfo);
  public:
    DeviceSampler (const DeviceSampler&) = delete;
    DeviceSampler& operator= (const DeviceSampler&) = delete;
    DeviceSampler& operator= (DeviceSampler&&) = delete;

    DeviceSampler (DeviceSampler &&);
    DeviceSampler (
        VkDevice vkDevice
      , ResourceAllocator& allocator
      , uint32_t dimension
      , VkFormat vkFormat
    );

    ~DeviceSampler ();

    void deleteGPUData ();
    void destroy ();

    VkResult getLastResult () const;
    VkSampler getVkSampler () const override;
    VkImage getVkImage () const override;
    VkImageView getVkImageView () const override;
};

}

#endif
