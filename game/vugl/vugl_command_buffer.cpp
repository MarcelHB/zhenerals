// SPDX-License-Identifier: GPL-2.0

#include "vugl_command_buffer.h"

#include <vulkan/vulkan.h>

namespace Vugl {

CommandBuffer::CommandBuffer (CommandBuffer && other)
  : allocator{other.allocator}
  , vkCommandBuffer{std::move(other.vkCommandBuffer)}
  , state{other.state}
  , frameIndex{other.frameIndex}
  , secondary{other.secondary}
{
  other.state = State::DESTROYED;
}

CommandBuffer::CommandBuffer (
    ResourceAllocator& allocator
  , size_t frameIndex
  , bool secondary
)
  : allocator{allocator}
  , vkCommandBuffer{}
  , state{State::NEW}
  , frameIndex{frameIndex}
  , secondary(secondary)
{
  allocator.allocateCommandBuffer(vkCommandBuffer, secondary);
}

VkResult CommandBuffer::beginCommands (uint32_t /*numberBuffers*/) {
  if (State::NEW != state) {
    return VK_NOT_READY;
  }

  VkCommandBufferBeginInfo vkCommandBufferBeginInfo = {};
  vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkCommandBufferBeginInfo.flags = 0;
  vkCommandBufferBeginInfo.pInheritanceInfo = nullptr;

  VkResult result = vkBeginCommandBuffer(vkCommandBuffer, &vkCommandBufferBeginInfo);
  if (result != VK_SUCCESS) {
    return result;
  }

  this->state = State::COMPUTE_OPEN;

  return VK_SUCCESS;
}

VkResult CommandBuffer::beginRendering (
    RenderPass& renderPass
  , const std::array<VkClearValue, 2>& vkClearColors
) {
  if (State::NEW != state) {
    return VK_NOT_READY;
  }

  VkCommandBufferBeginInfo vkCommandBufferBeginInfo = {};
  vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkCommandBufferBeginInfo.flags = 0;
  vkCommandBufferBeginInfo.pInheritanceInfo = nullptr;

  if (secondary) {
    vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = renderPass.getVkRenderPass();
    inheritanceInfo.framebuffer = renderPass.getVkFramebuffers()[frameIndex];

    vkCommandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
  }

  VkResult vkResult = vkBeginCommandBuffer(vkCommandBuffer, &vkCommandBufferBeginInfo);
  if (VK_SUCCESS != vkResult) {
    return vkResult;
  }

  if (!secondary) {
    VkRenderPassBeginInfo vkRenderPassBeginInfo = {};
    vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vkRenderPassBeginInfo.renderPass = renderPass.getVkRenderPass();
    vkRenderPassBeginInfo.framebuffer = renderPass.getVkFramebuffers()[frameIndex];
    vkRenderPassBeginInfo.renderArea.offset = {0, 0};
    vkRenderPassBeginInfo.renderArea.extent = renderPass.getExtent();
    vkRenderPassBeginInfo.clearValueCount = vkClearColors.size();
    vkRenderPassBeginInfo.pClearValues = vkClearColors.data();

    vkCmdBeginRenderPass(
        vkCommandBuffer
      , &vkRenderPassBeginInfo
      , VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );
  }

  this->state = State::OPEN;

  return VK_SUCCESS;
}

VkResult CommandBuffer::bindResource (BindableResource& resource) {
  if (State::OPEN != state && State::COMPUTE_OPEN != state) {
    return VK_NOT_READY;
  }

  return resource.recordBindCommands(vkCommandBuffer, frameIndex);
}

VkResult CommandBuffer::draw (Drawer& drawer) {
  if (State::OPEN != state && State::COMPUTE_OPEN != state) {
    return VK_NOT_READY;
  }

  return drawer.draw(vkCommandBuffer, frameIndex);
}

VkResult CommandBuffer::draw (std::function<VkResult(VkCommandBuffer, uint32_t)> fn) {
  if (State::OPEN != state && State::COMPUTE_OPEN != state) {
    return VK_NOT_READY;
  }

  return fn(vkCommandBuffer, frameIndex);
}

VkResult CommandBuffer::executeSecondary (const CommandBuffer& secondary) {
  vkCmdExecuteCommands(
      vkCommandBuffer
    , 1
    , &secondary.vkCommandBuffer
  );

  return VK_SUCCESS;
}

VkResult CommandBuffer::closeCommands () {
  if (State::COMPUTE_OPEN != state) {
    return VK_NOT_READY;
  }

  VkResult vkResult = vkEndCommandBuffer(vkCommandBuffer);
  if (VK_SUCCESS != vkResult) {
    return vkResult;
  }

  this->state = State::CLOSED;

  return VK_SUCCESS;
}

VkResult CommandBuffer::closeRendering () {
  if (State::OPEN != state) {
    return VK_NOT_READY;
  }

  if (!secondary) {
    vkCmdEndRenderPass(vkCommandBuffer);
  }
  VkResult vkResult = vkEndCommandBuffer(vkCommandBuffer);

  if (VK_SUCCESS != vkResult) {
    return vkResult;
  }

  this->state = State::CLOSED;

  return VK_SUCCESS;
}

VkResult CommandBuffer::beginDebugLabel (const std::string& label) {
  VkDebugUtilsLabelEXT vkLabelInfo = {};
  vkLabelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  vkLabelInfo.pNext = nullptr;
  vkLabelInfo.pLabelName = label.c_str();
  VuglDynamic::vkCmdBeginDebugUtilsLabelEXT(vkCommandBuffer, &vkLabelInfo);

  return VK_SUCCESS;
}

VkResult CommandBuffer::endDebugLabel () {
  VuglDynamic::vkCmdEndDebugUtilsLabelEXT(vkCommandBuffer);
  return VK_SUCCESS;
}

const VkCommandBuffer& CommandBuffer::getVkCommandBuffer () const {
  return vkCommandBuffer;
}

CommandBuffer::~CommandBuffer () {
  destroy();
}

void CommandBuffer::reset () {
  vkResetCommandBuffer(vkCommandBuffer, 0);

  this->state = State::NEW;
}

void CommandBuffer::destroy () {
  if (State::DESTROYED != state) {
    allocator.freeCommandBuffer(vkCommandBuffer);
  }

  this->state = State::DESTROYED;
}

}
