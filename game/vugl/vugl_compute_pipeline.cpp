// SPDX-License-Identifier: GPL-2.0

#include "vugl_compute_pipeline.h"

namespace Vugl {

ComputePipeline::ComputePipeline (ComputePipeline && other)
  : vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , resourceAllocator{other.resourceAllocator}
  , vkCS{other.vkCS}
  , vkDescriptorSetLayout{other.vkDescriptorSetLayout}
  , vkPipelineLayout{other.vkPipelineLayout}
  , vkPipeline{other.vkPipeline}
  , workgroupX{other.workgroupX}
  , workgroupY{other.workgroupY}
  , workgroupZ{other.workgroupZ}
{
  other.vkCS = VK_NULL_HANDLE;
  other.vkDescriptorSetLayout = VK_NULL_HANDLE;
  other.vkPipelineLayout = VK_NULL_HANDLE;
  other.vkPipeline = VK_NULL_HANDLE;
}

ComputePipeline::ComputePipeline (
    const PipelineSetup& setup
  , VkDevice vkDevice
  , ResourceAllocator& resourceAllocator
)
  : vkDevice{vkDevice}
  , resourceAllocator{resourceAllocator}
  , vkCS{VK_NULL_HANDLE}
  , vkDescriptorSetLayout{VK_NULL_HANDLE}
  , vkPipelineLayout{VK_NULL_HANDLE}
  , vkPipeline{VK_NULL_HANDLE}
  , workgroupX{1}
  , workgroupY{1}
  , workgroupZ{1}
{
  this->vkLastResult =
    vkCreateShaderModule(
        vkDevice
      , &(setup.vkCSModuleCreateInfo)
      , nullptr
      , &(this->vkCS)
    );

  if (VK_SUCCESS != this->vkLastResult) {
    return;
  }

  VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{setup.vkPipelineLayoutCreateInfo};

  if (setup.vkDescriptorSetLayoutCreateInfo.bindingCount > 0) {
    this->vkLastResult =
      vkCreateDescriptorSetLayout(
          vkDevice
        , &(setup.vkDescriptorSetLayoutCreateInfo)
        , nullptr
        , &(this->vkDescriptorSetLayout)
      );

    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;

    if (VK_SUCCESS != vkLastResult) {
      return;
    }
  }

  this->vkLastResult =
    vkCreatePipelineLayout(
        vkDevice
      , &(vkPipelineLayoutCreateInfo)
      , nullptr
      , &(this->vkPipelineLayout)
    );
  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  VkPipelineShaderStageCreateInfo vkCSStageCreateInfo = {};
  vkCSStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vkCSStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  vkCSStageCreateInfo.module = vkCS;
  vkCSStageCreateInfo.pName = "main";

  VkComputePipelineCreateInfo vkComputePipelineCreateInfo{setup.vkComputePipelineCreateInfo};
  vkComputePipelineCreateInfo.stage = vkCSStageCreateInfo;
  vkComputePipelineCreateInfo.layout = vkPipelineLayout;

  this->vkLastResult =
    vkCreateComputePipelines(
        vkDevice
      , VK_NULL_HANDLE
      , 1
      , &vkComputePipelineCreateInfo
      , nullptr
      , &(this->vkPipeline)
    );

  if (VK_SUCCESS != vkLastResult) {
    return;
  }
}

ComputePipeline::~ComputePipeline () {
  destroy();
}

DescriptorSet ComputePipeline::createDescriptorSet () {
  return {vkDevice, vkPipelineLayout, vkDescriptorSetLayout, 1};
}

VkResult ComputePipeline::draw (VkCommandBuffer buffer, uint32_t i) {
  vkCmdDispatch(buffer, workgroupX, workgroupY, workgroupZ);

  return VK_SUCCESS;
}

VkResult ComputePipeline::recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) {
  vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);

  return VK_SUCCESS;
}

void ComputePipeline::destroy () {
  vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, nullptr);
  this->vkDescriptorSetLayout = VK_NULL_HANDLE;

  vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
  this->vkPipelineLayout = VK_NULL_HANDLE;

  vkDestroyShaderModule(vkDevice, vkCS, nullptr);
  this->vkCS = VK_NULL_HANDLE;

  vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
  this->vkPipeline = VK_NULL_HANDLE;
}

void ComputePipeline::setWorkGroupXYZ (uint32_t x, uint32_t y, uint32_t z) {
  this->workgroupX = x;
  this->workgroupY = y;
  this->workgroupZ = z;
}

}
