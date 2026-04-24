// SPDX-License-Identifier: GPL-2.0

#include "vugl_command_pool.h"

namespace Vugl {

CommandPool::CommandPool (VkDevice vkDevice, uint32_t queueFamilyIdx)
  : vkDevice(vkDevice)
{
  VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
  vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  vkCommandPoolCreateInfo.queueFamilyIndex = queueFamilyIdx;
  vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  this->vkLastResult =
    vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, nullptr, &(this->vkCommandPool));
}

CommandPool::CommandPool (CommandPool&& other)
  : vkDevice(other.vkDevice)
  , vkLastResult(VK_SUCCESS)
  , vkCommandPool(other.vkCommandPool)
{
  other.vkCommandPool = VK_NULL_HANDLE;
}

CommandPool::~CommandPool () {
  vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
  this->vkCommandPool = VK_NULL_HANDLE;
}

VkResult CommandPool::getLastResult () const {
  return vkLastResult;
}

VkCommandPool CommandPool::getVkCommandPool () const {
  return vkCommandPool;
}

}
