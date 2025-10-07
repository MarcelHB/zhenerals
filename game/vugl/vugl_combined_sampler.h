// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_COMBINED_SAMPLER
#define H_VUGL_COMBINED_SAMPLER

#include "vugl_sampler.h"
#include "vugl_uploadable_resource.h"

namespace Vugl {

class CombinedSampler : public Sampler, public UploadableResource {
  private:
    Vugl::Texture texture;
    VkExtent2D extent;

  public:
    CombinedSampler (const CombinedSampler&) = delete;
    CombinedSampler& operator= (const CombinedSampler&) = delete;
    CombinedSampler& operator= (CombinedSampler&&) = delete;

    CombinedSampler (CombinedSampler &&);
    CombinedSampler (VkDevice vkDevice, ResourceAllocator& allocator);
    CombinedSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, ResourceAllocator& allocator);
    ~CombinedSampler();

    void destroy ();
    void deleteGPUData () override;
    void deleteHostData () override;

    VkExtent2D getExtent () const;
    VkImage getVkImage () const;
    VkImageView getVkImageView () const;

    VkResult recordUploadCommands (VkCommandBuffer vkCommandBuffer) override;

    template <typename T>
    void createTexture (
        const std::vector<T>& data
      , const VkExtent2D& extent
      , VkFormat format
    ) {
      texture.createTexture(data, extent, format);
      vkLastResult = texture.getLastResult();
    }

    template<typename T>
    void updateTexture (const std::vector<T>& data) {
      texture.updateTexture<T>(data);
      vkLastResult = texture.getLastResult();
    }
};

}

#endif
