#include <vector>

#include "Logging.h"
#include "Window.h"

#define CHECK_SDL(expr) \
  if ((expr) != SDL_TRUE) { \
    LOG_ZH("Window/SDL", "error: {}", SDL_GetError());\
    return false; \
  }

#define DEBUGGING 1

namespace ZH {

const std::vector<const char*> vkDeviceExtensionsList {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  , "VK_EXT_descriptor_indexing"
};

const std::vector<const char*> vkInstanceLayersList {
#if DEBUGGING
  "VK_LAYER_KHRONOS_validation"
#endif
};

Window::Window(const Config& config)
  : resolution(config.resolution)
{}

Vugl::Context& Window::getVuglContext() {
  return *vuglContext.get();
}

bool Window::init() {
  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

  sdlWindow =
    SDL_CreateWindow(
        "Zhenerals"
      , SDL_WINDOWPOS_CENTERED
      , SDL_WINDOWPOS_CENTERED
      , resolution.x
      , resolution.y
      , SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
    );
  CHECK_SDL(sdlWindow != nullptr);
  //SDL_Delay(4000); // For attaching tracy before all

  std::vector<const char*> vkInstanceExtensionsList;
  uint32_t extCount = 0;
  CHECK_SDL(SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extCount, nullptr));
  vkInstanceExtensionsList.resize(extCount);
  CHECK_SDL(SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extCount, vkInstanceExtensionsList.data()));

#if DEBUGGING
  vkInstanceExtensionsList.resize(extCount + 1);
  vkInstanceExtensionsList[extCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

  vuglContext = std::make_shared<Vugl::Context>(
      vkInstanceExtensionsList
    , vkInstanceLayersList
    , vkDeviceExtensionsList
  );

  VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
  VkViewport viewport = {};
  viewport.width = resolution.x;
  viewport.height = resolution.y;

  VkPhysicalDeviceFeatures vkDeviceFeatures = {};
  vkDeviceFeatures.fillModeNonSolid = true;

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT nextDeviceFeatures = {};
  nextDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
  nextDeviceFeatures.runtimeDescriptorArray = VK_TRUE;
  nextDeviceFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

  CHECK_SDL(SDL_Vulkan_CreateSurface(sdlWindow, vuglContext->getInstance(), &vkSurface));
  vuglContext->setSurface(
      vkSurface
    , viewport
    , vkDeviceExtensionsList
    , vkDeviceFeatures
    , &nextDeviceFeatures
  );

  return true;
}

OptionalCRef<SDL_Event> Window::getEvent() {
  bool newEvent = SDL_PollEvent(&event) == 1;

  if (newEvent) {
    return std::make_optional<std::reference_wrapper<SDL_Event>>(event);
  } else {
    return {};
  }
}

Window::~Window() {
  vuglContext->waitForIdle();
  vuglContext.reset();

  if (sdlWindow != nullptr) {
    SDL_DestroyWindow(sdlWindow);
  }

  SDL_Quit();
}

}
