// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_ELEMENT_BUFFER
#define H_VUGL_ELEMENT_BUFFER

#include <memory>

#include <vulkan/vulkan.h>

#include "vugl_bindable_resource.h"
#include "vugl_uploadable_resource.h"
#include "vugl_resource_allocator.h"

namespace Vugl {

class ElementBuffer
  : public UploadableResource
  , public BindableResource
{
  private:
    VkResult vkLastResult;
    ResourceAllocator& resourceAllocator;
    uint32_t binding;
    bool bigIndex;
    size_t numVertices;
    size_t numIndices;

    VkBuffer vkStagingBuffer;
    VkBuffer vkBuffer;
    VkDeviceSize vkStagingVBSize;
    VkDeviceSize vkStagingIBSize;
    VmaAllocation vmaStagingAllocation;
    VmaAllocation vmaAllocation;

  public:
    ElementBuffer (const ElementBuffer&) = delete;
    ElementBuffer& operator= (const ElementBuffer&) = delete;
    ElementBuffer& operator= (ElementBuffer&&) = delete;

    ElementBuffer (ElementBuffer &&);
    ElementBuffer (ResourceAllocator& allocator, uint32_t binding);
    ~ElementBuffer ();

    void deleteHostData () override;
    void deleteGPUData () override;
    void destroy ();

    VkResult getLastResult () const;
    size_t getNumIndices () const;
    size_t getNumVertices () const;

    VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) override;
    VkResult recordUploadCommands (VkCommandBuffer vkCommandBuffer) override;

    void setBigIndexBuffer(bool);

    template <typename T, typename U>
    void writeData (const std::vector<T>& vertexData, const std::vector<U>& indexData) {
      this->numVertices = vertexData.size();
      this->numIndices = indexData.size();
      this->vkStagingVBSize = sizeof(T) * vertexData.size();
      this->vkStagingIBSize = sizeof(U) * indexData.size();

      if (VK_NULL_HANDLE == vkStagingBuffer) {
        this->vkLastResult =
          resourceAllocator.createVkBuffer(
              vkStagingVBSize + vkStagingIBSize
            , VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            , vkStagingBuffer
            , vmaStagingAllocation
          );
      }

      if (VK_SUCCESS != vkLastResult) {
        return;
      }

      void *mappedData = resourceAllocator.mapMemory(vmaStagingAllocation);
      std::uninitialized_copy(vertexData.cbegin(), vertexData.cend(), static_cast<T*>(mappedData));
      std::uninitialized_copy(
          indexData.cbegin()
        , indexData.cend()
        , reinterpret_cast<U*>(static_cast<T*>(mappedData) + vertexData.size())
      );

      resourceAllocator.unmapMemory(vmaStagingAllocation);
    }
};

}

#endif
