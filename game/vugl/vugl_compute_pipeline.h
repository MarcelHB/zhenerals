// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_COMPUTE_PIPELINE
#define H_VUGL_COMPUTE_PIPELINE

#include "vugl_bindable_resource.h"
#include "vugl_drawer.h"
#include "vugl_descriptor_set.h"
#include "vugl_pipeline_setup.h"
#include "vugl_resource_allocator.h"

namespace Vugl {

class ComputePipeline : public BindableResource, public Drawer {
  private:
    VkDevice vkDevice;
    VkResult vkLastResult;
    ResourceAllocator& resourceAllocator;

    VkShaderModule vkCS;

    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkPipelineLayout vkPipelineLayout;
    VkPipeline vkPipeline;

    uint32_t workgroupX, workgroupY, workgroupZ;
  public:
    ComputePipeline (const ComputePipeline&) = delete;
    ComputePipeline& operator= (const ComputePipeline&) = delete;
    ComputePipeline& operator= (ComputePipeline&&) = delete;

    ComputePipeline (ComputePipeline &&);
    ComputePipeline (
        const PipelineSetup& setup
      , VkDevice vkDevice
      , ResourceAllocator& resourceAllocator
    );
    ~ComputePipeline ();

    DescriptorSet createDescriptorSet ();
    void destroy ();
    void setWorkGroupXYZ (uint32_t x, uint32_t y, uint32_t z);

    VkResult draw (VkCommandBuffer vkCommandBuffer, uint32_t i) override;
    VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) override;
};

}

#endif
