#ifndef H_VUGL_PIPELINE
#define H_VUGL_PIPELINE

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include "vugl_bindable_resource.h"
#include "vugl_descriptor_set.h"
#include "vugl_pipeline_setup.h"
#include "vugl_resource_allocator.h"

namespace Vugl {

class Pipeline : public BindableResource {
  private:
    VkDevice vkDevice;
    VkResult vkLastResult;

    VkShaderModule vkVS;
    VkShaderModule vkFS;

    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkPipelineLayout vkPipelineLayout;
    VkPipeline vkPipeline;

    VkViewport vkViewport;
    VkRect2D vkScissorRect;

    uint32_t numSwapchainImages;
    bool hasDynamicViewport;
    bool hasDynamicScissor;

  public:
    Pipeline (const Pipeline&) = delete;
    Pipeline& operator= (const Pipeline&) = delete;
    Pipeline& operator= (Pipeline&&) = delete;

    Pipeline (Pipeline &&);
    Pipeline (
        const PipelineSetup& setup
      , VkDevice vkDevice
      , ResourceAllocator& resourceAllocator
      , uint32_t numSwapchainImages
      , VkRenderPass vkRenderPass
    );
    ~Pipeline ();

    DescriptorSet createDescriptorSet ();
    void destroy ();

    VkResult getLastResult () const;
    VkPipelineLayout getVkPipelineLayout () const;

    VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i);
    void updateViewportExtent(const VkExtent2D& extent);
};

};

#endif
