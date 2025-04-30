#include "vugl_render_pass.h"

namespace Vugl {

RenderPass::RenderPass(RenderPass && other)
  : vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , resourceAllocator{other.resourceAllocator}
  , vkRenderPass{other.vkRenderPass}
  , vkSwapchainExtent{other.vkSwapchainExtent}
  , vkColorFormat{other.vkColorFormat}
  , vkSamplingFlag{other.vkSamplingFlag}
  , vkDepthImage{other.vkDepthImage}
  , vkDepthImageView{other.vkDepthImageView}
  , vmaDepthImageAllocation{other.vmaDepthImageAllocation}
  , vkColorImage{other.vkColorImage}
  , vkColorImageView{other.vkColorImageView}
  , vmaColorImageAllocation{other.vmaColorImageAllocation}
  , vkFramebuffers{std::move(other.vkFramebuffers)}
{
  other.vkDepthImage = VK_NULL_HANDLE;
  other.vkDepthImageView = VK_NULL_HANDLE;
  other.vmaDepthImageAllocation = VK_NULL_HANDLE;
  other.vkColorImage = VK_NULL_HANDLE;
  other.vkColorImageView = VK_NULL_HANDLE;
  other.vmaColorImageAllocation = VK_NULL_HANDLE;
  other.vkRenderPass = VK_NULL_HANDLE;
}

RenderPass::RenderPass(
    const RenderPassSetup& setup
  , VkDevice vkDevice
  , ResourceAllocator& resourceAllocator
  , const std::vector<VkImageView>& swapchainViews
  , VkExtent2D vkSwapchainExtent
)
  : vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , resourceAllocator{resourceAllocator}
  , vkRenderPass{VK_NULL_HANDLE}
  , vkSwapchainExtent{vkSwapchainExtent}
  , vkColorFormat{setup.vkColorFormat}
  , vkSamplingFlag{setup.vkSamplingFlag}
  , vkDepthImage{VK_NULL_HANDLE}
  , vkDepthImageView{VK_NULL_HANDLE}
  , vmaDepthImageAllocation{VK_NULL_HANDLE}
  , vkColorImage{VK_NULL_HANDLE}
  , vkColorImageView{VK_NULL_HANDLE}
  , vmaColorImageAllocation{VK_NULL_HANDLE}
  , vkFramebuffers{}
{
  this->vkLastResult =
    vkCreateRenderPass(
        vkDevice
      , &(setup.vkRenderPassCreateInfo)
      , nullptr
      , &(this->vkRenderPass)
    );
  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  createViewportResources(swapchainViews);
}

RenderPass::~RenderPass() {
  destroy();
}

void RenderPass::createViewportResources (const std::vector<VkImageView>& swapchainViews) {
  this->vkLastResult =
    resourceAllocator.createVkImage(
        vkSwapchainExtent
      , vkSamplingFlag
      , VK_FORMAT_D32_SFLOAT_S8_UINT
      , VK_IMAGE_TILING_OPTIMAL
      , VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , vkDepthImage
      , vmaDepthImageAllocation
    );

  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  // Depth buffer view
  VkImageViewCreateInfo vkDepthViewCreateInfo = {};
  vkDepthViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vkDepthViewCreateInfo.image = vkDepthImage;
  vkDepthViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vkDepthViewCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  vkDepthViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  vkDepthViewCreateInfo.subresourceRange.baseMipLevel = 0;
  vkDepthViewCreateInfo.subresourceRange.levelCount = 1;
  vkDepthViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  vkDepthViewCreateInfo.subresourceRange.layerCount = 1;

  this->vkLastResult =
    vkCreateImageView(
        vkDevice
      , &vkDepthViewCreateInfo
      , nullptr
      , &(this->vkDepthImageView)
    );
  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  // Multisampling color view
  if (vkSamplingFlag != VK_SAMPLE_COUNT_1_BIT) {
    this->vkLastResult =
      resourceAllocator.createVkImage(
          vkSwapchainExtent
        , vkSamplingFlag
        , vkColorFormat
        , VK_IMAGE_TILING_OPTIMAL
        , VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , vkColorImage
        , vmaColorImageAllocation
      );

    if (VK_SUCCESS != vkLastResult) {
      return;
    }

    // Color buffer view
    VkImageViewCreateInfo vkColorViewCreateInfo = {};
    vkColorViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkColorViewCreateInfo.image = vkColorImage;
    vkColorViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkColorViewCreateInfo.format = vkColorFormat;
    vkColorViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkColorViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkColorViewCreateInfo.subresourceRange.levelCount = 1;
    vkColorViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkColorViewCreateInfo.subresourceRange.layerCount = 1;

    this->vkLastResult =
      vkCreateImageView(
          vkDevice
        , &vkColorViewCreateInfo
        , nullptr
        , &(this->vkColorImageView)
      );
    if (VK_SUCCESS != vkLastResult) {
      return;
    }
  }

  vkFramebuffers.resize(swapchainViews.size());

  for (std::remove_reference<decltype(swapchainViews)>::type::size_type i = 0; i < swapchainViews.size(); ++i) {
    std::vector<VkImageView> attachments;
    attachments.reserve(3);

    // Look at how this corresponds to VkRenderPass attachment orders,
    // validations will report mismatches, ...
    // ... so here it's the inverse order apparently
    if (VK_SAMPLE_COUNT_1_BIT != vkSamplingFlag) {
      attachments.push_back(vkColorImageView);
      attachments.push_back(vkDepthImageView);
      attachments.push_back(swapchainViews[i]);
    // but not here
    } else {
      attachments.push_back(swapchainViews[i]);
      attachments.push_back(vkDepthImageView);
    }

    VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = vkSwapchainExtent.width;
    vkFramebufferCreateInfo.height = vkSwapchainExtent.height;
    vkFramebufferCreateInfo.layers = 1;

    this->vkLastResult =
      vkCreateFramebuffer(
          vkDevice
        , &vkFramebufferCreateInfo
        , nullptr
        , &(this->vkFramebuffers[i])
      );

    if (VK_SUCCESS != this->vkLastResult) {
      return;
    }
  }
}

void RenderPass::recreateForNewViewport (
    const VkExtent2D& vkSwapchainExtent
  , const std::vector<VkImageView>& swapchainViews
) {
  this->vkSwapchainExtent = vkSwapchainExtent;

  destroyViewportResources();
  createViewportResources(swapchainViews);
}

VkRenderPass RenderPass::getVkRenderPass () const {
  return vkRenderPass;
}

VkResult RenderPass::getLastResult () const {
  return vkLastResult;
}

void RenderPass::destroy () {
  destroyViewportResources();

  vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
  this->vkRenderPass = VK_NULL_HANDLE;
}

void RenderPass::destroyViewportResources () {
  for (auto framebuffer : vkFramebuffers) {
    vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
  }

  vkFramebuffers.clear();

  vkDestroyImageView(vkDevice, vkDepthImageView, nullptr);
  this->vkDepthImageView = VK_NULL_HANDLE;

  resourceAllocator.destroyVkImage(vkDepthImage, vmaDepthImageAllocation);
  this->vkDepthImage = VK_NULL_HANDLE;
  this->vmaDepthImageAllocation = VK_NULL_HANDLE;

  if (vkColorImage != VK_NULL_HANDLE) {
    vkDestroyImageView(vkDevice, vkColorImageView, nullptr);
    this->vkColorImageView = VK_NULL_HANDLE;

    resourceAllocator.destroyVkImage(vkColorImage, vmaColorImageAllocation);
    this->vkColorImage = VK_NULL_HANDLE;
    this->vmaColorImageAllocation = VK_NULL_HANDLE;
  }
}

const VkExtent2D& RenderPass::getExtent () const {
  return vkSwapchainExtent;
};

const std::vector<VkFramebuffer>& RenderPass::getVkFramebuffers () const {
  return vkFramebuffers;
}


}
