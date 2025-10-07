// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_PIPELINE_SETUP
#define H_VUGL_PIPELINE_SETUP

#include <vector>

#include <vulkan/vulkan.h>

namespace Vugl {

class PipelineSetup {
  public:
    static std::vector<VkDynamicState> vkDefaultDynamicStates;

    std::vector<char> VSCode;
    std::vector<char> FSCode;
    std::vector<char> CSCode;
    std::vector<VkVertexInputBindingDescription> vkVertexInputBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributeDescriptions;
    std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindings;

    VkSampleCountFlagBits vkSamplingFlag;
    VkShaderModuleCreateInfo vkCSModuleCreateInfo;
    VkShaderModuleCreateInfo vkFSModuleCreateInfo;
    VkShaderModuleCreateInfo vkVSModuleCreateInfo;
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    std::vector<VkVertexInputAttributeDescription> vkVertexDescriptions;
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    VkViewport vkViewport;
    VkRect2D vkScissorRect;
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilCreateInfo;
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState;
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    std::vector<VkDynamicState> vkDynamicStates;
    VkPipelineDynamicStateCreateInfo vkPipelineDynamicStateCreateInfo;
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;

    VkComputePipelineCreateInfo vkComputePipelineCreateInfo;
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    VkPushConstantRange vkPushConstantRange;

    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;

    PipelineSetup (const PipelineSetup&) = delete;
    PipelineSetup (PipelineSetup &&) = delete;
    PipelineSetup& operator= (const PipelineSetup&) = delete;
    PipelineSetup& operator= (PipelineSetup&&) = delete;

    PipelineSetup (
        const VkViewport& vkViewport
      , VkSampleCountFlagBits vkSamplingFlag = VK_SAMPLE_COUNT_1_BIT
    );

    void addVertexInput (
        VkFormat vkFormat
      , uint32_t offset
      , uint32_t size
      , uint32_t binding
      , bool isInstanceStream = false
    );
    void adjustViewport (const VkViewport& vkViewport);
    void enablePushConstants (VkShaderStageFlags shaderStageFlags, size_t size);

    void reserveCombinedSampler (VkShaderStageFlags shaderStageFlags);
    void reserveSampler (VkShaderStageFlags shaderStageFlags);
    void reserveStorageImage (VkShaderStageFlags shaderStageFlags);
    void reserveTexture (VkShaderStageFlags shaderStageFlags, size_t num = 1);
    void reserveUniformBuffer (VkShaderStageFlags shaderStageFlags, bool dynamic = false);

    void setCSCode (std::vector<char>&& code);
    void setFSCode (std::vector<char>&& code);
    void setVSCode (std::vector<char>&& code);

  private:
    void reserveDescriptor (VkDescriptorType type, VkShaderStageFlags shaderStageFlags, size_t num = 1);
};

};

#endif
