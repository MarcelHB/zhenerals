// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_COMMAND_BUFFER
#define H_VUGL_COMMAND_BUFFER

#include <array>
#include <functional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "vugl_bindable_resource.h"
#include "vugl_drawer.h"
#include "vugl_dynamic.h"
#include "vugl_render_pass.h"

namespace Vugl {

class CommandBuffer {
  public:
    enum class State {
        NEW
      , OPEN
      , COMPUTE_OPEN
      , CLOSED
      , DESTROYED
    };

  private:
    ResourceAllocator& allocator;
    VkCommandBuffer vkCommandBuffer;
    State state;
    size_t frameIndex;
    bool secondary;

  public:
    CommandBuffer (const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    CommandBuffer (ResourceAllocator& allocator, size_t frameIndex, bool secondary = false);
    CommandBuffer (CommandBuffer &&);
    ~CommandBuffer ();

    VkResult beginCommands (uint32_t numberBuffers);
    VkResult beginRendering (
        RenderPass& renderPass
      , const std::array<VkClearValue, 2>& vkClearColors
    );
    VkResult bindResource (BindableResource& resource);
    VkResult draw (Drawer& drawer);
    VkResult draw (std::function<VkResult(VkCommandBuffer, uint32_t)>);
    VkResult executeSecondary (const CommandBuffer&);
    VkResult closeCommands ();
    VkResult closeRendering ();

    VkResult beginDebugLabel(const std::string&);
    VkResult endDebugLabel();

    const VkCommandBuffer& getVkCommandBuffer () const;

    void reset ();
    void destroy ();
};

}

#endif
