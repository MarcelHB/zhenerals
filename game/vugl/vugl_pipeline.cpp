// SPDX-License-Identifier: GPL-2.0

#include <algorithm>
#include <type_traits>

#include "vugl_pipeline.h"

namespace Vugl {

Pipeline::Pipeline (Pipeline && other)
  : vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , vkVS{other.vkVS}
  , vkFS{other.vkFS}
  , vkDescriptorSetLayout{other.vkDescriptorSetLayout}
  , vkPipelineLayout{other.vkPipelineLayout}
  , vkPipeline{other.vkPipeline}
  , vkViewport{other.vkViewport}
  , vkScissorRect{other.vkScissorRect}
  , numSwapchainImages{other.numSwapchainImages}
  , hasDynamicViewport{other.hasDynamicViewport}
  , hasDynamicScissor{other.hasDynamicScissor}
{
  other.vkVS = VK_NULL_HANDLE;
  other.vkFS = VK_NULL_HANDLE;
  other.vkDescriptorSetLayout = VK_NULL_HANDLE;
  other.vkPipelineLayout = VK_NULL_HANDLE;
  other.vkPipeline = VK_NULL_HANDLE;
}

Pipeline::Pipeline (
    const PipelineSetup& setup
  , VkDevice vkDevice
  , uint32_t numSwapchainImages
  , VkRenderPass vkRenderPass
)
  : vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , vkVS{VK_NULL_HANDLE}
  , vkFS{VK_NULL_HANDLE}
  , vkDescriptorSetLayout{VK_NULL_HANDLE}
  , vkPipelineLayout{VK_NULL_HANDLE}
  , vkPipeline{VK_NULL_HANDLE}
  , vkViewport{setup.vkViewport}
  , vkScissorRect{setup.vkScissorRect}
  , numSwapchainImages{numSwapchainImages}
  , hasDynamicViewport{false}
  , hasDynamicScissor{false}
{
  this->vkLastResult =
    vkCreateShaderModule(
        vkDevice
      , &(setup.vkVSModuleCreateInfo)
      , nullptr
      , &(this->vkVS)
    );
  if (VK_SUCCESS != this->vkLastResult) {
    return;
  }

  VkPipelineShaderStageCreateInfo vkVSStageCreateInfo = {};
  vkVSStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vkVSStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vkVSStageCreateInfo.module = vkVS;
  vkVSStageCreateInfo.pName = "main";

  this->vkLastResult =
    vkCreateShaderModule(
        vkDevice
      , &(setup.vkFSModuleCreateInfo)
      , nullptr
      , &(this->vkFS)
    );
  if (VK_SUCCESS != this->vkLastResult) {
    return;
  }

  VkPipelineShaderStageCreateInfo vkFSStageCreateInfo = {};
  vkFSStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vkFSStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  vkFSStageCreateInfo.module = vkFS;
  vkFSStageCreateInfo.pName = "main";

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
    vkVSStageCreateInfo, vkFSStageCreateInfo
  };

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

  if (setup.vkGraphicsPipelineCreateInfo.pDynamicState != nullptr) {
    auto dynState = setup.vkGraphicsPipelineCreateInfo.pDynamicState;

    if (dynState->dynamicStateCount > 0) {
      auto statesEnd = dynState->pDynamicStates + dynState->dynamicStateCount;
      auto result = std::find(
          dynState->pDynamicStates
        , statesEnd
        , VK_DYNAMIC_STATE_VIEWPORT
      );

      if (result != statesEnd) {
        hasDynamicViewport = true;
      }

      result = std::find(
          dynState->pDynamicStates
        , statesEnd
        , VK_DYNAMIC_STATE_SCISSOR
      );

      if (result != statesEnd) {
        hasDynamicScissor = true;
      }
    }
  }

  VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{setup.vkGraphicsPipelineCreateInfo};
  vkGraphicsPipelineCreateInfo.stageCount = shaderStages.size();
  vkGraphicsPipelineCreateInfo.pStages = shaderStages.data();
  vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
  vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;

  this->vkLastResult =
    vkCreateGraphicsPipelines(
        vkDevice
      , VK_NULL_HANDLE
      , 1
      , &vkGraphicsPipelineCreateInfo
      , nullptr
      , &(this->vkPipeline)
    );
  if (VK_SUCCESS != vkLastResult) {
    return;
  }
}

Pipeline::~Pipeline () {
  destroy();
}

DescriptorSet Pipeline::createDescriptorSet () {
  return {vkDevice, vkPipelineLayout, vkDescriptorSetLayout, numSwapchainImages};
}

void Pipeline::destroy () {
  vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, nullptr);
  this->vkDescriptorSetLayout = VK_NULL_HANDLE;

  vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
  this->vkPipelineLayout = VK_NULL_HANDLE;

  vkDestroyShaderModule(vkDevice, vkFS, nullptr);
  this->vkFS = VK_NULL_HANDLE;

  vkDestroyShaderModule(vkDevice, vkVS, nullptr);
  this->vkVS = VK_NULL_HANDLE;

  vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
  this->vkPipeline = VK_NULL_HANDLE;
}

VkResult Pipeline::getLastResult () const {
  return vkLastResult;
}

VkPipelineLayout Pipeline::getVkPipelineLayout () const {
  return vkPipelineLayout;
}

VkResult Pipeline::recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t /*i*/) {
  vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

  if (hasDynamicViewport) {
    vkCmdSetViewport(vkCommandBuffer, 0, 1, &vkViewport);
  }

  if (hasDynamicScissor) {
    vkCmdSetScissor(vkCommandBuffer, 0, 1, &vkScissorRect);
  }

  return VK_SUCCESS;
}

void Pipeline::updateViewportExtent(const VkExtent2D& extent) {
  this->vkViewport.width = extent.width;
  this->vkViewport.height = extent.height;

  vkScissorRect.extent = extent;
}

};
