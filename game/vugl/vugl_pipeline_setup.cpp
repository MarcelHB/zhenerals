#include <algorithm>

#include "vugl_pipeline_setup.h"

namespace Vugl {

std::vector<VkDynamicState> PipelineSetup::vkDefaultDynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT
  , VK_DYNAMIC_STATE_SCISSOR
  , VK_DYNAMIC_STATE_LINE_WIDTH
  , VK_DYNAMIC_STATE_BLEND_CONSTANTS
};

PipelineSetup::PipelineSetup (
    const VkViewport& vkViewport
  , VkSampleCountFlagBits vkSamplingFlag
)
  :
    VSCode{}
  , FSCode{}
  , vkVertexInputBindingDescriptions{}
  , vkVertexInputAttributeDescriptions{}
  , vkDescriptorSetLayoutBindings{}
  , vkSamplingFlag{vkSamplingFlag}
  , vkCSModuleCreateInfo{}
  , vkFSModuleCreateInfo{}
  , vkVSModuleCreateInfo{}
  , vkPipelineVertexInputStateCreateInfo{}
  , vkVertexDescriptions{}
  , vkPipelineInputAssemblyStateCreateInfo{}
  , vkViewport{}
  , vkScissorRect{}
  , vkPipelineViewportStateCreateInfo{}
  , vkPipelineMultisampleStateCreateInfo{}
  , vkPipelineDepthStencilCreateInfo{}
  , vkPipelineRasterizationStateCreateInfo{}
  , vkPipelineColorBlendAttachmentState{}
  , vkPipelineColorBlendStateCreateInfo{}
  , vkDynamicStates{}
  , vkPipelineDynamicStateCreateInfo{}
  , vkPipelineLayoutCreateInfo{}
  , vkComputePipelineCreateInfo{}
  , vkGraphicsPipelineCreateInfo{}
  , vkPushConstantRange{}
  , vkDescriptorSetLayoutCreateInfo{}
{
  vkCSModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  vkFSModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  vkVSModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
  vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;

  vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
  vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  this->vkViewport.minDepth = 0.0f;
  this->vkViewport.maxDepth = 1.0f;

  vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vkPipelineViewportStateCreateInfo.viewportCount = 1;
  vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
  vkPipelineViewportStateCreateInfo.scissorCount = 1;
  vkPipelineViewportStateCreateInfo.pScissors = &vkScissorRect;

  vkPipelineDepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  vkPipelineDepthStencilCreateInfo.depthWriteEnable = VK_TRUE;
  vkPipelineDepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  vkPipelineDepthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
  vkPipelineDepthStencilCreateInfo.minDepthBounds = 0.0f;
  vkPipelineDepthStencilCreateInfo.maxDepthBounds = 1.0f;
  vkPipelineDepthStencilCreateInfo.stencilTestEnable = VK_FALSE;
  vkPipelineDepthStencilCreateInfo.front = {};
  vkPipelineDepthStencilCreateInfo.back = {};

  vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
  vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
  vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
  vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
  vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 1.0f;
  vkPipelineRasterizationStateCreateInfo.depthBiasClamp = VK_FALSE;
  vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

  vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  vkPipelineMultisampleStateCreateInfo.rasterizationSamples = vkSamplingFlag;

  vkPipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  vkPipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
  vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  vkPipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  vkPipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  vkPipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  vkPipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  vkPipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
  vkPipelineColorBlendStateCreateInfo.attachmentCount = 1;
  vkPipelineColorBlendStateCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;
  vkPipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
  vkPipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
  vkPipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
  vkPipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

  vkPipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  vkPipelineDynamicStateCreateInfo.dynamicStateCount = vkDefaultDynamicStates.size();
  vkPipelineDynamicStateCreateInfo.pDynamicStates = vkDefaultDynamicStates.data();

  vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  vkPipelineLayoutCreateInfo.setLayoutCount = 0;
  vkPipelineLayoutCreateInfo.pSetLayouts = nullptr;
  vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilCreateInfo;
  vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
  vkGraphicsPipelineCreateInfo.pDynamicState = &vkPipelineDynamicStateCreateInfo;
  vkGraphicsPipelineCreateInfo.subpass = 0;
  vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  vkGraphicsPipelineCreateInfo.basePipelineIndex = -1;

  vkComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

  vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  vkDescriptorSetLayoutCreateInfo.bindingCount = 0;

  adjustViewport(vkViewport);
}

void PipelineSetup::addVertexInput (
    VkFormat format
  , uint32_t offset
  , uint32_t size
  , uint32_t binding
  , bool isInstanceStream
) {
  VkVertexInputAttributeDescription vertexAttributeDescription = {};
  vertexAttributeDescription.binding = binding;
  vertexAttributeDescription.location = vkVertexInputAttributeDescriptions.size();
  vertexAttributeDescription.format = format;
  vertexAttributeDescription.offset = offset;
  vkVertexInputAttributeDescriptions.push_back(vertexAttributeDescription);

  vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions =
    vkVertexInputAttributeDescriptions.data();
  vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
    vkVertexInputAttributeDescriptions.size();

  auto vkBindingResult =
    std::find_if(
        vkVertexInputBindingDescriptions.begin()
      , vkVertexInputBindingDescriptions.end()
      , [=](VkVertexInputBindingDescription d) {
        return d.binding == binding;
      }
    );

  if (vkBindingResult == vkVertexInputBindingDescriptions.cend()) {
    VkVertexInputBindingDescription vkVertexInputBindingDescription = {};
    vkVertexInputBindingDescription.binding = binding;
    vkVertexInputBindingDescription.stride = size;
    vkVertexInputBindingDescription.inputRate = isInstanceStream ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
    vkVertexInputBindingDescriptions.push_back(vkVertexInputBindingDescription);

    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount =
      vkVertexInputBindingDescriptions.size();
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =
      vkVertexInputBindingDescriptions.data();
  } else {
    vkBindingResult->stride += size;
  }
}

void PipelineSetup::adjustViewport (const VkViewport& vkViewport) {
  this->vkViewport.x = vkViewport.x;
  this->vkViewport.y = vkViewport.y;
  this->vkViewport.width = vkViewport.width;
  this->vkViewport.height = vkViewport.height;
  this->vkViewport.minDepth = 0.0f;
  this->vkViewport.maxDepth = 1.0f;

  vkScissorRect.offset.x = 0;
  vkScissorRect.offset.y = 0;
  vkScissorRect.extent.width = vkViewport.width;
  vkScissorRect.extent.height = vkViewport.height;
}

void PipelineSetup::enablePushConstants (VkShaderStageFlags shaderStageFlags, size_t size) {
  vkPushConstantRange.stageFlags = shaderStageFlags;
  vkPushConstantRange.offset = 0;
  vkPushConstantRange.size = size;

  vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange;
}

void PipelineSetup::setCSCode (std::vector<char>&& code) {
  this->CSCode = code;
  vkCSModuleCreateInfo.codeSize = CSCode.size();
  vkCSModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(CSCode.data());
}

void PipelineSetup::setFSCode (std::vector<char>&& code) {
  this->FSCode = code;
  vkFSModuleCreateInfo.codeSize = FSCode.size();
  vkFSModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(FSCode.data());
}

void PipelineSetup::setVSCode (std::vector<char>&& code) {
  this->VSCode = code;
  vkVSModuleCreateInfo.codeSize = VSCode.size();
  vkVSModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(VSCode.data());
}

void PipelineSetup::reserveSampler (VkShaderStageFlags shaderStageFlags) {
  reserveDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shaderStageFlags);
}

void PipelineSetup::reserveStorageImage (VkShaderStageFlags shaderStageFlags) {
  reserveDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, shaderStageFlags);
}

void PipelineSetup::reserveUniformBuffer (VkShaderStageFlags shaderStageFlags, bool dynamic) {
  if (dynamic) {
    reserveDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, shaderStageFlags);
  } else {
    reserveDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderStageFlags);
  }
}

void PipelineSetup::reserveDescriptor (VkDescriptorType type, VkShaderStageFlags shaderStageFlags) {
  VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding = {};
  vkDescriptorSetLayoutBinding.binding = vkDescriptorSetLayoutBindings.size();
  vkDescriptorSetLayoutBinding.descriptorType = type;
  vkDescriptorSetLayoutBinding.descriptorCount = 1;
  vkDescriptorSetLayoutBinding.stageFlags = shaderStageFlags;
  vkDescriptorSetLayoutBindings.emplace_back(vkDescriptorSetLayoutBinding);

  vkDescriptorSetLayoutCreateInfo.bindingCount = vkDescriptorSetLayoutBindings.size();
  vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBindings.data();
}

};
