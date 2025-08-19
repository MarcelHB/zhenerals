#include "vugl_element_buffer.h"

namespace Vugl {

ElementBuffer::ElementBuffer (
    ResourceAllocator& allocator
  , uint32_t binding
) : resourceAllocator{allocator}
  , binding{binding}
  , bigIndex{false}
  , numVertices{0}
  , numIndices{0}
  , vkLastResult{VK_SUCCESS}
  , vkStagingBuffer{VK_NULL_HANDLE}
  , vkBuffer{VK_NULL_HANDLE}
  , vkStagingVBSize{0}
  , vkStagingIBSize{0}
  , vmaStagingAllocation{VK_NULL_HANDLE}
  , vmaAllocation{VK_NULL_HANDLE}
{}

ElementBuffer::ElementBuffer (ElementBuffer && other)
  : resourceAllocator{other.resourceAllocator}
  , binding(other.binding)
  , bigIndex{other.bigIndex}
  , numVertices{other.numVertices}
  , numIndices{other.numIndices}
  , vkLastResult{other.vkLastResult}
  , vkStagingBuffer{other.vkStagingBuffer}
  , vkBuffer{other.vkBuffer}
  , vkStagingVBSize{other.vkStagingVBSize}
  , vkStagingIBSize{other.vkStagingIBSize}
  , vmaStagingAllocation{other.vmaStagingAllocation}
  , vmaAllocation{other.vmaAllocation}
{
  other.vkStagingBuffer = VK_NULL_HANDLE;
  other.vkBuffer = VK_NULL_HANDLE;
  other.vmaStagingAllocation = VK_NULL_HANDLE;
  other.vmaAllocation = VK_NULL_HANDLE;
}

ElementBuffer::~ElementBuffer () {
  destroy();
}

void ElementBuffer::destroy () {
  deleteHostData();
  deleteGPUData();
}

void ElementBuffer::deleteGPUData () {
  resourceAllocator.destroyVkBuffer(vkBuffer, vmaAllocation);
  this->vkBuffer = VK_NULL_HANDLE;
  this->vmaAllocation = VK_NULL_HANDLE;
}

void ElementBuffer::deleteHostData () {
  resourceAllocator.destroyVkBuffer(vkStagingBuffer, vmaStagingAllocation);
  this->vkStagingBuffer = VK_NULL_HANDLE;
  this->vmaStagingAllocation = VK_NULL_HANDLE;
  this->vkStagingVBSize = 0;
  this->vkStagingIBSize = 0;
}

VkResult ElementBuffer::getLastResult () const {
  return vkLastResult;
}

size_t ElementBuffer::getNumIndices () const {
  return numIndices;
}

size_t ElementBuffer::getNumVertices () const {
  return numVertices;
}

VkResult ElementBuffer::recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) {
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(vkCommandBuffer, binding, 1, &vkBuffer, &offset);

  if (vkStagingIBSize > 0) {
    vkCmdBindIndexBuffer(
        vkCommandBuffer
      , vkBuffer
      , vkStagingVBSize
      , bigIndex ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16
    );
  }

  return VK_SUCCESS;
}

VkResult ElementBuffer::recordUploadCommands (VkCommandBuffer vkCommandBuffer) {
  if (VK_NULL_HANDLE == vkBuffer) {
    this->vkLastResult =
      resourceAllocator.createVkBuffer(
          vkStagingVBSize + vkStagingIBSize
        , VK_BUFFER_USAGE_TRANSFER_DST_BIT
          | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
          | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , vkBuffer
        , vmaAllocation
      );

    if (VK_SUCCESS != vkLastResult) {
      return vkLastResult;
    }
  }

  VkBufferCopy vkBufferCopy = {};
  vkBufferCopy.srcOffset = 0;
  vkBufferCopy.dstOffset = 0;
  vkBufferCopy.size = vkStagingVBSize + vkStagingIBSize;

  vkCmdCopyBuffer(vkCommandBuffer, vkStagingBuffer, vkBuffer, 1, &vkBufferCopy);

  return VK_SUCCESS;
}

void ElementBuffer::setBigIndexBuffer (bool big) {
  bigIndex = big;
}

};
