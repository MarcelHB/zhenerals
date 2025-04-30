#ifndef H_VUGL_RENDER_PASS
#define H_VUGL_RENDER_PASS

#include <vector>

#include <vulkan/vulkan.h>

#include "vugl_render_pass_setup.h"
#include "vugl_resource_allocator.h"

namespace Vugl {

class RenderPass {
  private:
    VkDevice vkDevice;
    VkResult vkLastResult;
    ResourceAllocator& resourceAllocator;

    VkRenderPass vkRenderPass;
    VkExtent2D vkSwapchainExtent;
    VkFormat vkColorFormat;
    VkSampleCountFlagBits vkSamplingFlag;

    VkImage vkDepthImage;
    VkImageView vkDepthImageView;
    VmaAllocation vmaDepthImageAllocation;

    VkImage vkColorImage;
    VkImageView vkColorImageView;
    VmaAllocation vmaColorImageAllocation;

    std::vector<VkFramebuffer> vkFramebuffers;

  public:
    RenderPass(RenderPass && other);
    RenderPass(
        const RenderPassSetup& renderPassSetup
      , VkDevice vkDevice
      , ResourceAllocator& resourceAllocator
      , const std::vector<VkImageView>& swapchainViews
      , VkExtent2D vkSwapchainExtent
    );
    ~RenderPass();
    RenderPass (const RenderPass&) = delete;
    RenderPass& operator= (const RenderPass&) = delete;
    RenderPass& operator= (RenderPass&&) = delete;


    void createViewportResources (const std::vector<VkImageView>& swapchainViews);
    void destroy ();
    void destroyViewportResources ();

    VkResult getLastResult () const;

    const VkExtent2D& getExtent () const;
    const std::vector<VkFramebuffer>& getVkFramebuffers () const;
    VkRenderPass getVkRenderPass () const;

    void recreateForNewViewport (
        const VkExtent2D& vkSwapchainExtent
      , const std::vector<VkImageView>& swapchainViews
    );
};

}

#endif
