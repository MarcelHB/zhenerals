#ifndef H_VUGL_FRAME
#define H_VUGL_FRAME

#include <vulkan/vulkan.h>

#include "vugl_command_buffer.h"

namespace Vugl {

class Frame {
  private:
    VkDevice vkDevice;
    VkQueue vkGFXQueue;
    VkQueue vkPresenterQueue;
    VkFence vkSubmitFence;
    VkSwapchainKHR vkSwapchain;
    VkSemaphore vkImageAvailableSemaphore;
    VkSemaphore vkRenderDoneSemaphore;
    uint32_t swapchainImageIndex;

    VkResult lastResult;

  public:
    Frame (const Frame&) = delete;
    Frame& operator= (const Frame&) = delete;
    Frame& operator= (Frame&&) = delete;

    Frame (Frame &&);
    Frame (
        VkDevice vkDevice
      , VkQueue vkGFXQueue
      , VkQueue vkPresenterQueue
      , VkSwapchainKHR vkSwapchain
      , uint32_t swapchainImageIndex
    );
    ~Frame ();

    void destroy ();
    uint32_t getImageIndex () const;
    VkResult getLastResult () const;
    void setFrameLockHandles (VkFence submitFence, VkSemaphore imageAvailableSemaphore);
    void setSwapchain (VkSwapchainKHR vkSwapchain);
    void submitAndPresent (const CommandBuffer& commandBuffer);
};

}

#endif
