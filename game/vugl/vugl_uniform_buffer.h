#ifndef H_VUGL_UNIFORM_BUFFER
#define H_VUGL_UNIFORM_BUFFER

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include "vugl_resource_allocator.h"

namespace Vugl {

class UniformBuffer {
  private:
    ResourceAllocator& allocator;
    VkDeviceSize alignment;
    VkDeviceSize alignedSize;
    std::vector<VkBuffer> buffers;
    std::vector<VmaAllocation> bufferAllocations;
    uint32_t numDescriptors;
    VkDeviceSize strideSize;

  public:
    UniformBuffer (const UniformBuffer&) = delete;
    UniformBuffer& operator= (const UniformBuffer&) = delete;
    UniformBuffer& operator= (UniformBuffer&&) = delete;

    UniformBuffer (UniformBuffer &&);
    UniformBuffer (
        ResourceAllocator& allocator
      , VkDeviceSize uboAlignment
      , uint32_t numBuffers
      , VkDeviceSize totalSize
      , uint32_t numDescriptors
    );
    ~UniformBuffer ();

    void destroy ();

    const std::vector<VkBuffer>& getBuffers () const;
    uint32_t getNumOfDescriptors () const;
    VkDeviceSize getStrideSize () const;

    void setStrideSize (VkDeviceSize size);

    template <typename T>
    void writeData (const T& data, uint32_t i) {
      T *mappedData = reinterpret_cast<T*>(allocator.mapMemory(bufferAllocations[i]));
      *mappedData = data;
      allocator.unmapMemory(bufferAllocations[i]);
    }

    template <typename T, typename It>
    void writeData (It begin, It end, uint32_t i) {
      void *mappedData = allocator.mapMemory(bufferAllocations[i]);

      auto it = begin;
      for (size_t j = 0; it != end; it++, j++) {
        std::uninitialized_copy(
            it
          , it + 1
          , reinterpret_cast<T*>(static_cast<uint8_t*>(mappedData) + j * strideSize)
        );
      }

      allocator.unmapMemory(bufferAllocations[i]);
    }
};

}

#endif
