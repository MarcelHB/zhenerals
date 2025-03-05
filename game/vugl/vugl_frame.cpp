#include "vugl_frame.h"

namespace Vugl {

Frame::Frame (
    VkDevice vkDevice
  , VkQueue vkGFXQueue
  , VkQueue vkPresenterQueue
  , VkSwapchainKHR vkSwapchain
  , uint32_t swapchainImageIndex
)
  : vkDevice{vkDevice}
  , vkGFXQueue{vkGFXQueue}
  , vkPresenterQueue{vkPresenterQueue}
  , vkSwapchain{vkSwapchain}
  , vkSubmitFence{VK_NULL_HANDLE}
  , vkImageAvailableSemaphore{VK_NULL_HANDLE}
  , vkRenderDoneSemaphore{VK_NULL_HANDLE}
  , swapchainImageIndex{swapchainImageIndex}
  , lastResult{VK_SUCCESS}
{
  VkSemaphoreCreateInfo vkSemaphoreCreateInfo = {};
  vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  this->lastResult =
    vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, nullptr, &vkRenderDoneSemaphore);
}

Frame::Frame (Frame && other)
  : vkDevice{other.vkDevice}
  , vkGFXQueue{other.vkGFXQueue}
  , vkPresenterQueue{other.vkPresenterQueue}
  , vkSwapchain{other.vkSwapchain}
  , vkSubmitFence{other.vkSubmitFence}
  , vkImageAvailableSemaphore{other.vkImageAvailableSemaphore}
  , vkRenderDoneSemaphore{other.vkRenderDoneSemaphore}
  , swapchainImageIndex{other.swapchainImageIndex}
  , lastResult{other.lastResult}
{
  other.vkRenderDoneSemaphore = VK_NULL_HANDLE;
}

Frame::~Frame () {
  destroy();
}

void Frame::destroy () {
  vkDestroySemaphore(vkDevice, vkRenderDoneSemaphore, nullptr);
  this->vkRenderDoneSemaphore = VK_NULL_HANDLE;
}

uint32_t Frame::getImageIndex () const {
  return swapchainImageIndex;
}

VkResult Frame::getLastResult () const {
  return lastResult;
}

void Frame::setFrameLockHandles (VkFence submitFence, VkSemaphore imageAvailableSemaphore) {
  this->vkSubmitFence = submitFence;
  this->vkImageAvailableSemaphore = imageAvailableSemaphore;
}

void Frame::setSwapchain (VkSwapchainKHR vkSwapchain) {
  this->vkSwapchain = vkSwapchain;
}

void Frame::submitAndPresent (const CommandBuffer& commandBuffer) {
  VkPipelineStageFlags vkPipelineStageFlags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo vkSubmitInfo = {};
  vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  vkSubmitInfo.waitSemaphoreCount = 1;
  vkSubmitInfo.pWaitSemaphores = &vkImageAvailableSemaphore;
  vkSubmitInfo.pWaitDstStageMask = vkPipelineStageFlags;
  vkSubmitInfo.commandBufferCount = 1;
  vkSubmitInfo.pCommandBuffers = &commandBuffer.getVkCommandBuffer();
  vkSubmitInfo.signalSemaphoreCount = 1;
  vkSubmitInfo.pSignalSemaphores = &vkRenderDoneSemaphore;

  vkResetFences(vkDevice, 1, &vkSubmitFence);
  this->lastResult =
    vkQueueSubmit(
        vkGFXQueue
      , 1
      , &vkSubmitInfo
      , vkSubmitFence
    );
  if (VK_SUCCESS != lastResult) {
    return;
  }

  VkPresentInfoKHR vkPresentInfo = {};
  vkPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  vkPresentInfo.waitSemaphoreCount = 1;
  vkPresentInfo.pWaitSemaphores = &vkRenderDoneSemaphore;
  vkPresentInfo.swapchainCount = 1;
  vkPresentInfo.pSwapchains = &vkSwapchain;
  vkPresentInfo.pImageIndices = &swapchainImageIndex;
  vkPresentInfo.pResults = nullptr;

  vkQueuePresentKHR(vkPresenterQueue, &vkPresentInfo);

  vkWaitForFences(vkDevice, 1, &vkSubmitFence, VK_TRUE, UINT64_MAX);
}

}
