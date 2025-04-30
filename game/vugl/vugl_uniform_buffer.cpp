#include "vugl_uniform_buffer.h"

namespace Vugl {

UniformBuffer::UniformBuffer (UniformBuffer && other)
  : allocator{other.allocator}
  , alignment{other.alignment}
  , alignedSize{other.alignedSize}
  , buffers{std::move(other.buffers)}
  , bufferAllocations{std::move(other.bufferAllocations)}
  , numDescriptors{other.numDescriptors}
  , strideSize{other.strideSize}
{}

UniformBuffer::UniformBuffer (
    ResourceAllocator& resourceAllocator
  , VkDeviceSize uboAlignment
  , uint32_t numBuffers
  , VkDeviceSize dataSize
  , uint32_t numDescriptors
) : allocator{resourceAllocator}
  , alignment{uboAlignment}
  , alignedSize{dataSize}
  , buffers{numBuffers, VK_NULL_HANDLE}
  , bufferAllocations{numBuffers, VK_NULL_HANDLE}
  , numDescriptors{numDescriptors}
  , strideSize{dataSize}
{
  if (dataSize % alignment > 0) {
    this->alignedSize += (alignment - (dataSize % alignment));
    this->strideSize = alignedSize;
  }

  for (uint32_t i = 0; i < numBuffers; ++i) {
    allocator.createVkBuffer(
        alignedSize
      , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
      , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      , buffers[i]
      , bufferAllocations[i]
    );
  }
}

UniformBuffer::~UniformBuffer() {
  destroy();
}

void UniformBuffer::destroy () {
  for (decltype(buffers)::size_type i = 0; i < buffers.size(); ++i) {
    allocator.destroyVkBuffer(buffers[i], bufferAllocations[i]);
  }
  buffers.resize(0);
  bufferAllocations.resize(0);
}

const std::vector<VkBuffer>& UniformBuffer::getBuffers () const {
  return buffers;
}

uint32_t UniformBuffer::getNumOfDescriptors () const {
  return numDescriptors;
}

VkDeviceSize UniformBuffer::getStrideSize () const {
  return strideSize;
}

void UniformBuffer::setStrideSize (VkDeviceSize size) {
  this->strideSize = size;

  if (size % alignment > 0) {
    this->strideSize = size + (alignment - (size % alignment));
  }
}

}
