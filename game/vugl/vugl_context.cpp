#include <iostream>
#include <cmath>
#include <set>
#include <vector>

#include "vugl_context.h"

namespace VuglDynamic {

PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;

}

namespace Vugl {

Context::Context () :
  Context{
    std::vector<const char*>()
  , std::vector<const char*>()
  , std::vector<const char*>()
  }
{}

Context::Context (
    const std::vector<const char*>& vkInstanceExtensionsList
  , const std::vector<const char*>& vkInstanceLayersList
  , const std::vector<const char*>& vkDeviceExtensionsList
) :
    state{State::NEW}
  , error{Error::NO_CTX_ERROR}
  , vkLastResult{VK_SUCCESS}
  , debuggingAllowed{false}
  , vkInstance{VK_NULL_HANDLE}
  , vkPhysicalDevice{VK_NULL_HANDLE}
  , vkPhysicalDeviceProperties{}
  , vkDevice{VK_NULL_HANDLE}
  , allocator{VK_NULL_HANDLE}
  , resourceAllocator{}
  , gfxQueueFamilyIndex{0}
  , presenterQueueFamilyIndex{0}
  , vkGFXQueue{VK_NULL_HANDLE}
  , vkPresenterQueue{VK_NULL_HANDLE}
  , vkCommandPool{VK_NULL_HANDLE}
  , vkSurface{VK_NULL_HANDLE}
  , vkSurfaceCapabilities{}
  , vkSurfaceFormat{}
  , vkPresentMode{}
  , vkViewport{}
  , vkSwapchainExtent{}
  , vkSwapchain{VK_NULL_HANDLE}
  , vkSwapchainImages{}
  , vkSwapchainImageViews{}
  , frames{}
  , vkSamplingFlag{VK_SAMPLE_COUNT_1_BIT}
  , currentFrame{0}
{
  VkApplicationInfo vkApplicationInfo = {};
  vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  vkApplicationInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

  VkInstanceCreateInfo vkInstanceCreateInfo = {};
  vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vkInstanceCreateInfo.enabledExtensionCount = vkInstanceExtensionsList.size();
  vkInstanceCreateInfo.ppEnabledExtensionNames = vkInstanceExtensionsList.data();
  vkInstanceCreateInfo.enabledLayerCount = vkInstanceLayersList.size();
  vkInstanceCreateInfo.ppEnabledLayerNames = vkInstanceLayersList.data();
  vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;

  this->vkLastResult =
    vkCreateInstance(
        &vkInstanceCreateInfo
      , nullptr
      , &(this->vkInstance)
    );
  CHECK_VK_RESULT(vkLastResult);

  for (auto& instanceExtension : vkInstanceExtensionsList) {
    if (std::string {instanceExtension} == VK_EXT_DEBUG_UTILS_EXTENSION_NAME) {
      debuggingAllowed = true;
      VuglDynamic::vkCmdBeginDebugUtilsLabelEXT =
        reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vkInstance, "vkCmdBeginDebugUtilsLabelEXT"));
      VuglDynamic::vkCmdEndDebugUtilsLabelEXT =
        reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vkInstance, "vkCmdEndDebugUtilsLabelEXT"));
      break;
    }
  }

  uint32_t physicalDevicesCount = 0;
  vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, nullptr);

  if (0 == physicalDevicesCount) {
    std::cerr << "No Vulkan capable device found!" << std::endl;
    std::exit(1);
  }

  std::vector<VkPhysicalDevice> vkPhysicalDevices{physicalDevicesCount};
  vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, vkPhysicalDevices.data());

  this->vkPhysicalDevice = pickSuitableVkPhysicalDevice(vkPhysicalDevices, vkDeviceExtensionsList);

  if (VK_NULL_HANDLE == vkPhysicalDevice) {
    std::cerr << "No suitable Vulkan device found!" << std::endl;
    std::exit(1);
  }

  auto gfxQueueFamilyIndexOption = findGFXQueueFamilyIndex(vkPhysicalDevice);
  if (!gfxQueueFamilyIndexOption) {
    std::cerr << "No suitable Vulkan queue family found!" << std::endl;
    std::exit(1);
  }

  this->gfxQueueFamilyIndex = gfxQueueFamilyIndexOption.value();
  vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);

  auto msaaBits =
    vkPhysicalDeviceProperties.limits.framebufferColorSampleCounts
    & vkPhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

  this->vkSamplingFlag = VK_SAMPLE_COUNT_1_BIT;
  if (msaaBits & VK_SAMPLE_COUNT_64_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_64_BIT;
  } else if (msaaBits & VK_SAMPLE_COUNT_32_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_32_BIT;
  } else if (msaaBits & VK_SAMPLE_COUNT_16_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_16_BIT;
  } else if (msaaBits & VK_SAMPLE_COUNT_8_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_8_BIT;
  } else if (msaaBits & VK_SAMPLE_COUNT_4_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_4_BIT;
  } else if (msaaBits & VK_SAMPLE_COUNT_2_BIT) {
    this->vkSamplingFlag = VK_SAMPLE_COUNT_2_BIT;
  }

  this->state = State::PHYSICAL_DEVICE_READY;
}

void Context::setSurface (
    VkSurfaceKHR vkSurface
  , const VkViewport& vkViewport
  , const std::vector<const char*>& vkDeviceExtensionsList
  , const VkPhysicalDeviceFeatures& enabledFeatures
  , void *deviceFeaturesNext
) {
  if (State::PHYSICAL_DEVICE_READY != this->state) {
    this->error = Error::INVALID_STATE;
    this->vkLastResult = VK_ERROR_UNKNOWN;

    return;
  }

  auto presenterQueueFamilyIndexOption = findPresenterQueueFamilyIndex(vkPhysicalDevice, vkSurface);
  if (!presenterQueueFamilyIndexOption) {
    this->error = Error::BAD_SURFACE;
    this->vkLastResult = VK_ERROR_UNKNOWN;

    return;
  }

  this->presenterQueueFamilyIndex = presenterQueueFamilyIndexOption.value();
  this->vkSurface = vkSurface;
  this->vkViewport = vkViewport;

  std::vector<VkDeviceQueueCreateInfo> vkDeviceQueueCreateInfos;
  std::set<uint32_t> queueFamilyIndices = { gfxQueueFamilyIndex, presenterQueueFamilyIndex };

  float queuePriority = 1.0f;
  for (uint32_t familyIndex : queueFamilyIndices) {
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo = {};
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.queueFamilyIndex = familyIndex;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    vkDeviceQueueCreateInfos.push_back(vkDeviceQueueCreateInfo);
  }

  VkDeviceCreateInfo vkDeviceCreateInfo = {};
  vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  vkDeviceCreateInfo.queueCreateInfoCount = vkDeviceQueueCreateInfos.size();
  vkDeviceCreateInfo.pQueueCreateInfos = vkDeviceQueueCreateInfos.data();
  if (nullptr != deviceFeaturesNext) { // only related to DeviceFeatures2
    vkDeviceCreateInfo.pEnabledFeatures = nullptr;
  } else {
    vkDeviceCreateInfo.pEnabledFeatures = &enabledFeatures;
  }
  vkDeviceCreateInfo.enabledLayerCount = 0;
  vkDeviceCreateInfo.ppEnabledLayerNames = nullptr;
  vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vkDeviceExtensionsList.size());
  vkDeviceCreateInfo.ppEnabledExtensionNames = vkDeviceExtensionsList.data();

  if (deviceFeaturesNext != nullptr) {
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = enabledFeatures;
    physicalDeviceFeatures2.pNext = deviceFeaturesNext;
    vkDeviceCreateInfo.pNext = &physicalDeviceFeatures2;
  }

  this->vkLastResult =
    vkCreateDevice(
        vkPhysicalDevice
      , &vkDeviceCreateInfo
      , nullptr
      , &(this->vkDevice)
    );

  if (this->vkLastResult != VK_SUCCESS) {
    return;
  }

  VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
  vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  vkCommandPoolCreateInfo.queueFamilyIndex = gfxQueueFamilyIndex;
  vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  this->vkLastResult =
    vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, nullptr, &(this->vkCommandPool));

  if (this->vkLastResult != VK_SUCCESS) {
    return;
  }

  VmaAllocatorCreateInfo vmaAllocatorCreateInfo = {};
  vmaAllocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
  vmaAllocatorCreateInfo.physicalDevice = vkPhysicalDevice;
  vmaAllocatorCreateInfo.device = vkDevice;
  vmaAllocatorCreateInfo.instance = vkInstance;
  this->vkLastResult =
    vmaCreateAllocator(&vmaAllocatorCreateInfo, &(this->allocator));

  if (this->vkLastResult != VK_SUCCESS) {
    return;
  }
  this->resourceAllocator = ResourceAllocator{vkDevice, vkPhysicalDevice, allocator, vkCommandPool};

  vkGetDeviceQueue(vkDevice, gfxQueueFamilyIndex, 0, &(this->vkGFXQueue));
  vkGetDeviceQueue(vkDevice, presenterQueueFamilyIndex, 0, &(this->vkPresenterQueue));

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &(this->vkSurfaceCapabilities));

  uint32_t formatsCount = 0;
  std::vector<VkSurfaceFormatKHR> supportedFormats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, nullptr);
  supportedFormats.resize(formatsCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, supportedFormats.data());

  uint32_t presentModesCount = 0;
  std::vector<VkPresentModeKHR> supportedPresentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, nullptr);
  supportedPresentModes.resize(presentModesCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, supportedPresentModes.data());

  if (supportedPresentModes.empty() || supportedFormats.empty()) {
    this->error = Error::BAD_SURFACE;
    this->vkLastResult = VK_ERROR_UNKNOWN;
    return;
  }

  this->vkSurfaceFormat = supportedFormats.at(0);
  for (auto& format : supportedFormats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM) {
      this->vkSurfaceFormat = format;
      break;
    }
  }

  this->vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  this->createSwapchainResources();

  if (VK_SUCCESS != this->vkLastResult) {
    return;
  }

  VkSemaphoreCreateInfo vkSemaphoreCreateInfo = {};
  vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo vkFenceCreateInfo = {};
  vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkImageAvailableSemaphores.resize(vkSwapchainImageViews.size(), VK_NULL_HANDLE);
  vkSubmitFences.resize(vkSwapchainImageViews.size(), VK_NULL_HANDLE);

  for (decltype(vkSwapchainImageViews)::size_type i = 0; i < vkSwapchainImageViews.size(); ++i) {
    this->vkLastResult =
      vkCreateSemaphore(
          vkDevice
        , &vkSemaphoreCreateInfo
        , nullptr
        , &(vkImageAvailableSemaphores[i])
      );

    if (VK_SUCCESS != this->vkLastResult) {
      return;
    }

    this->vkLastResult =
      vkCreateFence(
          vkDevice
        , &vkFenceCreateInfo
        , nullptr
        , &(vkSubmitFences[i])
      );

    if (VK_SUCCESS != this->vkLastResult) {
      return;
    }
  }

  this->error = Error::NO_CTX_ERROR;
  this->state = State::SURFACE_READY;
}

void Context::recreateSwapchainResources (const VkViewport& viewport) {
  this->vkViewport = viewport;
  vkDeviceWaitIdle(this->vkDevice);

  this->destroySwapchainResources();

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &(this->vkSurfaceCapabilities));
  this->createSwapchainResources();
}

void Context::createSwapchainResources () {
  if (vkSurfaceCapabilities.currentExtent.width != UINT32_MAX) {
    this->vkSwapchainExtent = vkSurfaceCapabilities.currentExtent;
  } else {
    vkSwapchainExtent.width =
      std::max(
          vkSurfaceCapabilities.minImageExtent.width
        , std::min(
              vkSurfaceCapabilities.maxImageExtent.width
            , static_cast<uint32_t>(std::ceil(vkViewport.width))
          )
      );
    vkSwapchainExtent.height =
      std::max(
          vkSurfaceCapabilities.minImageExtent.height
        , std::min(
              vkSurfaceCapabilities.maxImageExtent.height
            , static_cast<uint32_t>(std::ceil(vkViewport.height))
          )
      );
  }

  uint32_t imagesCount = vkSurfaceCapabilities.minImageCount + 1;
  if (vkSurfaceCapabilities.maxImageCount > 0
      && vkSurfaceCapabilities.maxImageCount < imagesCount
  ) {
    imagesCount = vkSurfaceCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR vkSwapchainCreateInfo = {};
  vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  vkSwapchainCreateInfo.surface = vkSurface;
  vkSwapchainCreateInfo.minImageCount = imagesCount;
  vkSwapchainCreateInfo.imageFormat = vkSurfaceFormat.format;
  vkSwapchainCreateInfo.imageColorSpace = vkSurfaceFormat.colorSpace;
  vkSwapchainCreateInfo.imageArrayLayers = 1;
  vkSwapchainCreateInfo.imageExtent = vkSwapchainExtent;
  vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  vkSwapchainCreateInfo.preTransform = vkSurfaceCapabilities.currentTransform;
  vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  vkSwapchainCreateInfo.presentMode = vkPresentMode;
  vkSwapchainCreateInfo.clipped = VK_TRUE;
  vkSwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (gfxQueueFamilyIndex != presenterQueueFamilyIndex) {
    uint32_t queueFamilyIndicesArray[] = { gfxQueueFamilyIndex, presenterQueueFamilyIndex };

    vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    vkSwapchainCreateInfo.queueFamilyIndexCount = 2;
    vkSwapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
  } else {
    vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfo.queueFamilyIndexCount = 0;
  }

  this->vkLastResult =
    vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfo, nullptr, &(this->vkSwapchain));
  if (VK_SUCCESS != vkLastResult) {
    return;
  }

  vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imagesCount, nullptr);
  if (frames.size() > 0) {
    for (auto& frame : frames) {
      frame.setSwapchain(vkSwapchain);
    }
  } else {
    for (decltype(frames)::size_type i = 0; i < imagesCount; ++i) {
      frames.emplace_back(vkDevice, vkGFXQueue, vkPresenterQueue, vkSwapchain, i);
    }
  }

  vkSwapchainImages.resize(imagesCount);
  vkSwapchainImageViews.resize(imagesCount);
  vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imagesCount, vkSwapchainImages.data());

  for (decltype(vkSwapchainImages)::size_type i = 0; i < vkSwapchainImages.size(); ++i) {
    VkImageViewCreateInfo vkImageViewCreateInfo = {};
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image = vkSwapchainImages[i];
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = vkSurfaceFormat.format;
    vkImageViewCreateInfo.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY
      , VK_COMPONENT_SWIZZLE_IDENTITY
      , VK_COMPONENT_SWIZZLE_IDENTITY
      , VK_COMPONENT_SWIZZLE_IDENTITY
    };
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;

    this->vkLastResult =
      vkCreateImageView(
          vkDevice
        , &vkImageViewCreateInfo
        , nullptr
        , &vkSwapchainImageViews[i]
      );
    if (VK_SUCCESS != vkLastResult) {
      break;
    }
  }

  if (VK_SUCCESS != vkLastResult) {
    return;
  }
}

Context::~Context () {
  destroy();
}

void Context::destroy () {
  this->state = State::DESTROYED;
  if (VK_NULL_HANDLE == vkInstance) {
    return;
  }

  for (decltype(vkSwapchainImageViews)::size_type i = 0; i < vkSwapchainImageViews.size(); ++i) {
    vkDestroySemaphore(vkDevice, vkImageAvailableSemaphores[i], nullptr);
    vkDestroyFence(vkDevice, vkSubmitFences[i], nullptr);
    frames[i].destroy();
  }
  vkImageAvailableSemaphores.clear();
  vkSubmitFences.clear();
  frames.clear();

  destroySwapchainResources();

  vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
  this->vkSurface = VK_NULL_HANDLE;

  vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
  this->vkCommandPool = VK_NULL_HANDLE;

  vmaDestroyAllocator(allocator);
  this->allocator = VK_NULL_HANDLE;

  vkDestroyDevice(vkDevice, nullptr);
  this->vkDevice = VK_NULL_HANDLE;

  vkDestroyInstance(vkInstance, nullptr);
  this->vkInstance = VK_NULL_HANDLE;
}

void Context::destroySwapchainResources () {
  for (auto view : vkSwapchainImageViews) {
    vkDestroyImageView(vkDevice, view, nullptr);
  }
  vkSwapchainImageViews.clear();

  vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
  this->vkSwapchain = VK_NULL_HANDLE;
}

CommandBuffer Context::createCommandBuffer (size_t frameIndex, bool secondary) {
  return {resourceAllocator, frameIndex, secondary};
}

ComputePipeline Context::createComputePipeline (const PipelineSetup& setup) {
  return {setup, vkDevice, resourceAllocator};
}

ElementBuffer Context::createElementBuffer (uint32_t binding) {
  return {resourceAllocator, binding};
}

Pipeline Context::createPipeline (
    const PipelineSetup& setup
  , VkRenderPass renderPass
) {
  return {
      setup
    , vkDevice
    , resourceAllocator
    , static_cast<uint32_t>(vkSwapchainImageViews.size())
    , renderPass
  };
}

RenderPass Context::createRenderPass (
    const RenderPassSetup& setup
) {
  return {
      setup
    , vkDevice
    , resourceAllocator
    , vkSwapchainImageViews
    , vkSwapchainExtent
  };
}

CombinedSampler Context::createCombinedSampler () {
  return CombinedSampler{vkDevice, resourceAllocator};
}

CombinedSampler Context::createCombinedSampler (const VkSamplerCreateInfo& createInfo) {
  return {vkDevice, createInfo, resourceAllocator};
}

Sampler Context::createSampler () {
  return Sampler{vkDevice};
}

Sampler Context::createSampler (const VkSamplerCreateInfo& createInfo) {
  return {vkDevice, createInfo};
}

Texture Context::createTexture () {
  return {vkDevice, resourceAllocator};
}

UniformBuffer Context::createUniformBuffer (
    VkDeviceSize totalSize
  , uint32_t numDescriptors
) {
  return {
      resourceAllocator
    , vkPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment
    , static_cast<uint32_t>(vkSwapchainImageViews.size())
    , totalSize
    , numDescriptors
  };
}

UniformBuffer Context::createUniformBuffer (
    VkDeviceSize totalSize
  , uint32_t numDescriptors
  , uint32_t numBuffers
) {
  return {
      resourceAllocator
    , vkPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment
    , numBuffers
    , totalSize
    , numDescriptors
  };
}

VkDevice Context::getDevice () const {
  return vkDevice;
}

Context::Error Context::getError () const {
  return error;
}

VkExtent2D Context::getExtent () const {
  return vkSwapchainExtent;
}

VkInstance Context::getInstance () const {
  return vkInstance;
}

Frame& Context::getNextFrame () {
  uint32_t imageIndex = 0;
  vkAcquireNextImageKHR(
      vkDevice
    , vkSwapchain
    , UINT64_MAX
    , vkImageAvailableSemaphores[currentFrame]
    , VK_NULL_HANDLE
    , &imageIndex
  );

  auto& frame = frames[imageIndex];
  frame.setFrameLockHandles(vkSubmitFences[imageIndex], vkImageAvailableSemaphores[currentFrame]);

  this->currentFrame = (currentFrame + 1) % vkSwapchainImageViews.size();

  return frame;
}

const std::vector<VkImage>& Context::getSwapchainImages () const {
  return vkSwapchainImages;
}

const std::vector<VkImageView>& Context::getSwapchainImageViews () const {
  return vkSwapchainImageViews;
}

std::optional<uint32_t> Context::findGFXQueueFamilyIndex (VkPhysicalDevice vkPhysicalDevice) const {
  uint32_t count = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies{count};
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilies.data());

  uint32_t i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return {i};
    }

    i += 1;
  }

  return std::nullopt;
}

const VkPhysicalDeviceProperties& Context::getVkPhysicalDeviceProperties () const {
  return vkPhysicalDeviceProperties;
}

VkSampleCountFlagBits Context::getVkSamplingFlag () const {
  return vkSamplingFlag;
}

VkViewport Context::getViewport () const {
  return vkViewport;
}

const VkSurfaceFormatKHR& Context::getVkSurfaceFormat () const {
  return vkSurfaceFormat;
}

bool Context::isDebuggingAllowed() const {
  return debuggingAllowed;
}

std::optional<uint32_t> Context::findPresenterQueueFamilyIndex (
    VkPhysicalDevice vkPhysicalDevice
  , VkSurfaceKHR vkSurface
) const {
  uint32_t count = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies{count};
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilies.data());

  uint32_t i = 0;
  for (const auto& queueFamily : queueFamilies) {
    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

    if (presentSupport) {
      return {i};
    }

    i += 1;
  }

  return std::nullopt;
}

VkPhysicalDevice Context::pickSuitableVkPhysicalDevice (
    const std::vector<VkPhysicalDevice>& vkPhysicalDevices
  , const std::vector<const char*>& vkDeviceExtensionsList
) const {
  if (vkDeviceExtensionsList.empty()) {
    return vkPhysicalDevices.at(0);
  }

  for (auto vkPhysicalDevice : vkPhysicalDevices) {
    uint32_t extensionsCount = 0;
    std::vector<VkExtensionProperties> extensions;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionsCount, nullptr);
    extensions.resize(extensionsCount);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionsCount, extensions.data());

    std::set<std::string> requiredExtensionsSet{
        vkDeviceExtensionsList.cbegin()
      , vkDeviceExtensionsList.cend()
    };

    for (const auto& extension : extensions) {
      requiredExtensionsSet.erase(extension.extensionName);
    }

    if (requiredExtensionsSet.empty()) {
      return vkPhysicalDevice;
    }
  }

  return VK_NULL_HANDLE;
}

bool Context::uploadResource (UploadableResource& resource) {
  VkCommandBuffer buffer;

  this->vkLastResult = resourceAllocator.allocateCommandBuffer(buffer);
  if (vkLastResult != VK_SUCCESS) {
    return false;
  }

  VkCommandBufferBeginInfo vkCmdBufferBeginInfo = {};
  vkCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(buffer, &vkCmdBufferBeginInfo);
  resource.recordUploadCommands(buffer);
  vkEndCommandBuffer(buffer);

  submitToGPUQueue(buffer);

  resourceAllocator.freeCommandBuffer(buffer);

  return true;
}

bool Context::submitToGPUQueue (VkCommandBuffer& buffer) {
  VkSubmitInfo vkSubmitInfo = {};
  vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  vkSubmitInfo.commandBufferCount = 1;
  vkSubmitInfo.pCommandBuffers = &buffer;

  auto result = vkQueueSubmit(vkGFXQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
  if (result != VK_SUCCESS) {
    return false;
  }

  vkQueueWaitIdle(vkGFXQueue);

  return true;
}

void Context::waitForIdle () {
  vkDeviceWaitIdle(vkDevice);
}

}
