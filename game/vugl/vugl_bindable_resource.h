#ifndef H_VUGL_BINDABLE_RESOURCE
#define H_VUGL_BINDABLE_RESOURCE

#include <vulkan/vulkan.h>

namespace Vugl {

class BindableResource {
  public:
    virtual ~BindableResource () {}

    virtual VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) = 0;
};

};

#endif
