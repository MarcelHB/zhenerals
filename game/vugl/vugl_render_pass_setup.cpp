#include "vugl_render_pass_setup.h"

namespace Vugl {

RenderPassSetup::RenderPassSetup (
    const VkSurfaceFormatKHR& vkSurfaceFormat
  , VkSampleCountFlagBits vkSamplingFlag
) : vkColorFormat(vkSurfaceFormat.format)
  , vkSamplingFlag(vkSamplingFlag)
  , vkColorAttachmentRef{}
  , vkDepthAttachmentRef{}
  , vkColorResolveAttachmentRef{}
  , vkAttachmentDescriptions{}
  , vkSubpassDescription{}
  , vkSubpassDependency{}
  , vkRenderPassCreateInfo{}
{
  vkAttachmentDescriptions.resize(
      VK_SAMPLE_COUNT_1_BIT == vkSamplingFlag ? 2 : 3
    , {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}
  );

  auto& vkColorAttachment = vkAttachmentDescriptions.at(0);
  vkColorAttachment.format = vkSurfaceFormat.format;
  vkColorAttachment.samples = vkSamplingFlag;
  vkColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  vkColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  vkColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  vkColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  vkColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  vkColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  if (VK_SAMPLE_COUNT_1_BIT != vkSamplingFlag) {
    vkColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  vkColorAttachmentRef.attachment = 0;
  vkColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  auto& vkDepthAttachment = vkAttachmentDescriptions.at(1);
  vkDepthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  vkDepthAttachment.samples = vkSamplingFlag;
  vkDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  vkDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  vkDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  vkDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  vkDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  vkDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  vkDepthAttachmentRef.attachment = 1;
  vkDepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  if (VK_SAMPLE_COUNT_1_BIT != vkSamplingFlag) {
    auto& vkColorAttachmentResolve = vkAttachmentDescriptions.at(2);
    vkColorAttachmentResolve.format = vkSurfaceFormat.format;
    vkColorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    vkColorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkColorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkColorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkColorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkColorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkColorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    vkColorResolveAttachmentRef.attachment = 2;
    vkColorResolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  vkSubpassDescription.colorAttachmentCount = 1;
  vkSubpassDescription.pColorAttachments = &vkColorAttachmentRef;
  vkSubpassDescription.pDepthStencilAttachment = &vkDepthAttachmentRef;
  vkSubpassDescription.pResolveAttachments = nullptr;
  if (VK_SAMPLE_COUNT_1_BIT != vkSamplingFlag) {
    vkSubpassDescription.pResolveAttachments = &vkColorResolveAttachmentRef;
  }

  vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  vkSubpassDependency.dstSubpass = 0;
  vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  vkSubpassDependency.srcAccessMask = 0;
  vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  vkRenderPassCreateInfo.attachmentCount = vkAttachmentDescriptions.size();
  vkRenderPassCreateInfo.pAttachments = vkAttachmentDescriptions.data();
  vkRenderPassCreateInfo.subpassCount = 1;
  vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
  vkRenderPassCreateInfo.dependencyCount = 1;
  vkRenderPassCreateInfo.pDependencies = &vkSubpassDependency;
}

}
