// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL
#define H_VUGL

#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

#include "vugl_combined_sampler.h"
#include "vugl_compute_pipeline.h"
#include "vugl_dynamic.h"
#include "vugl_element_buffer.h"
#include "vugl_frame.h"
#include "vugl_pipeline.h"
#include "vugl_resource_allocator.h"
#include "vugl_texture.h"
#include "vugl_uniform_buffer.h"
#include "vugl_uploadable_resource.h"

#define CHECK_VK_RESULT(expr) \
  if ((expr) != VK_SUCCESS) { \
    std::cerr << "VK FAILURE: " << __FILE__ << ":" << __LINE__ << ": " << expr << std::endl; \
    std::exit(1); \
  }

namespace Vugl {

class Context {
  public:
    enum class State {
      NEW, PHYSICAL_DEVICE_READY, SURFACE_READY, DESTROYED
    };

    enum class Error {
      NO_CTX_ERROR, INVALID_STATE, BAD_SURFACE
    };

    using array_index_t = uint8_t;

  private:
    enum class DynStateUpdate {
      NONE, VIEWPORT, SCISSOR, BLENDING, LINE_WIDTH
    };

    State state;
    Error error;
    VkResult vkLastResult;

    bool debuggingAllowed;
    VkInstance vkInstance;
    VkPhysicalDevice vkPhysicalDevice;
    VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
    VkDevice vkDevice;
    VmaAllocator allocator;
    ResourceAllocator resourceAllocator;

    uint32_t gfxQueueFamilyIndex;
    uint32_t presenterQueueFamilyIndex;
    VkQueue vkGFXQueue;
    VkQueue vkPresenterQueue;
    VkCommandPool vkCommandPool;

    VkSurfaceKHR vkSurface;
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    VkSurfaceFormatKHR vkSurfaceFormat;
    VkPresentModeKHR vkPresentMode;

    VkViewport vkViewport;
    VkExtent2D vkSwapchainExtent;
    VkSwapchainKHR vkSwapchain;
    std::vector<VkImage> vkSwapchainImages;
    std::vector<VkImageView> vkSwapchainImageViews;
    std::vector<Frame> frames;
    std::vector<VkSemaphore> vkImageAvailableSemaphores;
    std::vector<VkFence> vkSubmitFences;

    VkSampleCountFlagBits vkSamplingFlag;

    VkClearColorValue vkClearColorValue;
    VkClearDepthStencilValue vkClearDepthStencilValue;

    uint32_t currentFrame;

    void createSwapchainResources ();

    void destroy ();
    void destroySwapchainResources ();

    std::optional<uint32_t> findGFXQueueFamilyIndex (VkPhysicalDevice vkPhysicalDevice) const;
    std::optional<uint32_t> findPresenterQueueFamilyIndex (
        VkPhysicalDevice vkPhysicalDevice
      , VkSurfaceKHR vkSurface
    ) const;

    VkPhysicalDevice pickSuitableVkPhysicalDevice (
        const std::vector<VkPhysicalDevice>& physicalDevices
      , const std::vector<const char*>& vkDeviceExtensionsList
    ) const;
  public:
    Context ();
    Context (
        const std::vector<const char*>& vkInstanceExtensionsList
      , const std::vector<const char*>& vkInstanceLayersList
      , const std::vector<const char*>& vkDeviceExtensionsList
    );
    ~Context ();

    Context (const Context&) = delete;
    Context (Context &&) = delete;
    Context& operator= (const Context&) = delete;
    Context& operator= (Context&&) = delete;

    CommandBuffer createCommandBuffer (size_t frameIndex, bool secondary = false);
    ComputePipeline createComputePipeline (const PipelineSetup&);
    ElementBuffer createElementBuffer (uint32_t binding);
    Pipeline createPipeline (const PipelineSetup&, VkRenderPass renderPass);
    RenderPass createRenderPass (const RenderPassSetup&);
    CombinedSampler createCombinedSampler ();
    CombinedSampler createCombinedSampler (const VkSamplerCreateInfo&);
    Sampler createSampler ();
    Sampler createSampler (const VkSamplerCreateInfo&);
    Texture createTexture ();
    UniformBuffer createUniformBuffer (VkDeviceSize totalSize, uint32_t numDescriptors = 1);
    UniformBuffer createUniformBuffer (VkDeviceSize totalSize, uint32_t numDescriptors, uint32_t numBuffers);
    void recreateSwapchainResources(const VkViewport& viewport);

    VkDevice getDevice () const;
    Error getError () const;
    VkExtent2D getExtent ()  const;
    VkInstance getInstance () const;
    Frame& getNextFrame ();
    const std::vector<VkImage>& getSwapchainImages () const;
    const std::vector<VkImageView>& getSwapchainImageViews () const;
    const VkPhysicalDeviceProperties& getVkPhysicalDeviceProperties () const;
    VkSampleCountFlagBits getVkSamplingFlag () const;
    VkViewport getViewport () const;
    const VkSurfaceFormatKHR& getVkSurfaceFormat () const;

    bool isDebuggingAllowed() const;

    void setSurface (
        VkSurfaceKHR vkSurface
      , const VkViewport& vkViewport
      , const std::vector<const char*>& vkDeviceExtensionsList
      , const VkPhysicalDeviceFeatures& enabledFeatures
      , void *deviceFeaturesNext
    );

    bool uploadResource (UploadableResource& resource);
    bool submitToGPUQueue (VkCommandBuffer& buffer);
    void waitForIdle ();
};

}

#endif
