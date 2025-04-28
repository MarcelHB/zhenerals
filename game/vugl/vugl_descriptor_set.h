#ifndef H_VUGL_DESCRIPTOR_SET
#define H_VUGL_DESCRIPTOR_SET

#include <tuple>
#include <vector>

#include <vulkan/vulkan.h>

#include "vugl_bindable_resource.h"
#include "vugl_device_sampler.h"
#include "vugl_sampler.h"
#include "vugl_uniform_buffer.h"

namespace Vugl {

class DescriptorSet : public BindableResource {
  private:
    enum class DescriptorType {
        COMBINED_SAMPLER
      , STORAGE_IMAGE
      , UBO
      , DYNAMIC_UBO
      , NULL_SAMPLER
    };
    using BindingInfo = std::tuple<DescriptorType, size_t>;

    VkDevice vkDevice;
    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkPipelineLayout vkPipelineLayout;
    uint32_t numSwapchainImages;
    VkPipelineBindPoint vkPipelineBindPoint;
    VkResult vkLastResult;

    VkDescriptorPool vkDescriptorPool;
    std::vector<VkDescriptorSet> vkDescriptorSets;

    std::vector<std::reference_wrapper<const Sampler>> assignedCombinedSamplers;
    std::vector<std::reference_wrapper<const DeviceSampler>> assignedStorageImages;
    std::vector<std::reference_wrapper<const UniformBuffer>> assignedUniformBuffers;
    size_t nullSamplers;

    std::vector<BindingInfo> bindings;

  public:
    DescriptorSet (DescriptorSet &&);
    DescriptorSet (
        VkDevice vkDevice
      , VkPipelineLayout vkPipelineLayout
      , VkDescriptorSetLayout vkDescriptorSetLayout
      , uint32_t numSwapchainImages
    );
    ~DescriptorSet ();

    void assignCombinedSampler (const Sampler&);
    void assignNullSampler ();
    void assignStorageImage (const DeviceSampler&);
    void assignUniformBuffer (const UniformBuffer&, bool dynamic = false);

    void destroy ();
    VkResult getLastResult () const;
    VkDescriptorSet getVkDescriptorSet (size_t index) const;
    VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) override;
    void setPipelineBindPoint (VkPipelineBindPoint bindPoint);
    void updateDevice ();
};

};

#endif
