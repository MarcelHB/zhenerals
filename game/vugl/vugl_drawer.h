// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_DRAWER
#define H_VUGL_DRAWER

#include <vulkan/vulkan.h>

namespace Vugl {

class Drawer {
  public:
    virtual ~Drawer () {}

    virtual VkResult draw (VkCommandBuffer vkCommandBuffer, uint32_t i) = 0;
};

};

#endif
