#ifndef H_VUGL_UPLOAD_SAMPLER
#define H_VUGL_UPLOAD_SAMPLER

#include <memory>

#include <vulkan/vulkan.h>

#include "vugl_texture.h"

namespace Vugl {

class Sampler {
  protected:
    VkDevice vkDevice;
    VkResult vkLastResult;

    VkSampler vkSampler;

    void createSampler (const VkSamplerCreateInfo& createInfo);
  public:
    Sampler (const Sampler&) = delete;
    Sampler& operator= (const Sampler&) = delete;
    Sampler& operator= (Sampler&&) = delete;

    Sampler (Sampler &&);
    Sampler (VkDevice vkDevice);
    Sampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo);
    virtual ~Sampler();

    void destroy ();

    VkResult getLastResult () const;
    VkSampler getVkSampler () const;
};

}

#endif
