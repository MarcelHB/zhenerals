// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_COMMAND_POOL
#define H_VUGL_COMMAND_POOL

#include <vulkan/vulkan.h>

namespace Vugl {

class CommandPool {
  public:
    CommandPool (VkDevice, uint32_t);
    CommandPool (const CommandPool&) = delete;
    CommandPool (CommandPool&&);
    ~CommandPool ();

    CommandPool& operator= (const CommandPool&) = delete;

    VkResult getLastResult () const;
    VkCommandPool getVkCommandPool () const;
  private:
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkResult vkLastResult = VK_SUCCESS;
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;
};

}

#endif
