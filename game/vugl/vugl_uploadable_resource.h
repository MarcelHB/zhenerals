// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_UPLOADABLE_RESOURCE
#define H_VUGL_UPLOADABLE_RESOURCE

#include <vulkan/vulkan.h>

namespace Vugl {

class UploadableResource {
  public:
    virtual ~UploadableResource () {};

    virtual VkResult recordUploadCommands (VkCommandBuffer vkCommandBuffer) = 0;
    virtual void deleteGPUData () = 0;
    virtual void deleteHostData () = 0;
};

}

#endif
