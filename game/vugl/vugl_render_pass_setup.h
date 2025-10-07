// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_RENDER_PASS_SETUP
#define H_VUGL_RENDER_PASS_SETUP

#include <vector>

#include <vulkan/vulkan.h>

namespace Vugl {

struct RenderPassSetup {
  VkFormat vkColorFormat;
  VkSampleCountFlagBits vkSamplingFlag;

  VkAttachmentReference vkColorAttachmentRef;
  VkAttachmentReference vkDepthAttachmentRef;
  VkAttachmentReference vkColorResolveAttachmentRef;
  std::vector<VkAttachmentDescription> vkAttachmentDescriptions;
  VkSubpassDescription vkSubpassDescription;
  VkSubpassDependency vkSubpassDependency;
  VkRenderPassCreateInfo vkRenderPassCreateInfo;

  RenderPassSetup (
      const VkSurfaceFormatKHR& vkSurfaceFormat
    , VkSampleCountFlagBits vkSamplingFlag = VK_SAMPLE_COUNT_1_BIT
  );
};

}

#endif
