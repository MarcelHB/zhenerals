#include "vugl_upload_sampler.h"

namespace Vugl {

UploadSampler::UploadSampler (UploadSampler && other)
  : allocator{other.allocator}
  , vkDevice{other.vkDevice}
  , vkLastResult{other.vkLastResult}
  , vkSampler{other.vkSampler}
  , vkStagingBuffer{other.vkStagingBuffer}
  , vmaStagingBufferAllocation{other.vmaStagingBufferAllocation}
  , vkTexture{other.vkTexture}
  , vmaTextureAllocation{other.vmaTextureAllocation}
  , vkTextureView{other.vkTextureView}
  , extent{other.extent}
{
  other.vkSampler = VK_NULL_HANDLE;
  other.vkStagingBuffer = VK_NULL_HANDLE;
  other.vmaStagingBufferAllocation = VK_NULL_HANDLE;
  other.vkTexture = VK_NULL_HANDLE;
  other.vmaTextureAllocation = VK_NULL_HANDLE;
  other.vkTextureView = VK_NULL_HANDLE;
}

UploadSampler::UploadSampler (VkDevice vkDevice, ResourceAllocator& allocator)
  : allocator{allocator}
  , vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , vkSampler{VK_NULL_HANDLE}
  , vkStagingBuffer{VK_NULL_HANDLE}
  , vmaStagingBufferAllocation{VK_NULL_HANDLE}
  , vkTexture{VK_NULL_HANDLE}
  , vmaTextureAllocation{VK_NULL_HANDLE}
  , vkTextureView{VK_NULL_HANDLE}
  , extent{}
{
  VkSamplerCreateInfo vkSamplerCreateInfo = {};
  vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
  vkSamplerCreateInfo.maxAnisotropy = 16.0f;
  vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
  vkSamplerCreateInfo.compareEnable = VK_FALSE;
  vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  vkSamplerCreateInfo.mipLodBias = 0.0f;
  vkSamplerCreateInfo.minLod = 0.0f;
  vkSamplerCreateInfo.maxLod = 0.0f;

  createSampler(vkSamplerCreateInfo);
}

UploadSampler::UploadSampler (VkDevice vkDevice, const VkSamplerCreateInfo& createInfo, ResourceAllocator& allocator)
  : allocator{allocator}
  , vkDevice{vkDevice}
  , vkLastResult{VK_SUCCESS}
  , vkSampler{VK_NULL_HANDLE}
  , vkStagingBuffer{VK_NULL_HANDLE}
  , vmaStagingBufferAllocation{VK_NULL_HANDLE}
  , vkTexture{VK_NULL_HANDLE}
  , vmaTextureAllocation{VK_NULL_HANDLE}
  , vkTextureView{VK_NULL_HANDLE}
  , extent{}
{
  createSampler(createInfo);
}

UploadSampler::~UploadSampler () {
  destroy();
}

void UploadSampler::createSampler (const VkSamplerCreateInfo& createInfo) {
  this->vkLastResult =
    vkCreateSampler(
        vkDevice
      , &createInfo
      , nullptr
      , &(this->vkSampler)
    );
}

void UploadSampler::deleteGPUData () {
  allocator.destroyVkImage(vkTexture, vmaTextureAllocation);
  this->vkTexture = VK_NULL_HANDLE;
  this->vmaTextureAllocation = VK_NULL_HANDLE;
}

void UploadSampler::deleteHostData () {
  allocator.destroyVkBuffer(vkStagingBuffer, vmaStagingBufferAllocation);
  this->vkStagingBuffer = VK_NULL_HANDLE;
  this->vmaStagingBufferAllocation = VK_NULL_HANDLE;
}

void UploadSampler::destroy () {
  vkDestroySampler(vkDevice, vkSampler, nullptr);
  this->vkSampler = VK_NULL_HANDLE;

  vkDestroyImageView(vkDevice, vkTextureView, nullptr);
  this->vkTextureView = VK_NULL_HANDLE;

  deleteGPUData();
  deleteHostData();
}

VkExtent2D UploadSampler::getExtent () const {
  return extent;
}

VkResult UploadSampler::getLastResult () const {
  return vkLastResult;
}

VkSampler UploadSampler::getVkSampler () const {
  return vkSampler;
}

VkImage UploadSampler::getVkImage () const {
  return vkTexture;
}

VkImageView UploadSampler::getVkImageView () const {
  return vkTextureView;
}

VkResult UploadSampler::recordUploadCommands (VkCommandBuffer vkCommandBuffer) {
  VkPipelineStageFlags vkSrcStageFlags;
  VkPipelineStageFlags vkDstStageFlags;

  VkImageMemoryBarrier vkImgMemBarrier = {};
  vkImgMemBarrier.srcAccessMask = 0;
  vkImgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  vkSrcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  vkDstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;

  vkImgMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  vkImgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  vkImgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  vkImgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vkImgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vkImgMemBarrier.image = vkTexture;
  vkImgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vkImgMemBarrier.subresourceRange.baseMipLevel = 0;
  vkImgMemBarrier.subresourceRange.levelCount = 1;
  vkImgMemBarrier.subresourceRange.baseArrayLayer = 0;
  vkImgMemBarrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(
      vkCommandBuffer
    , vkSrcStageFlags
    , vkDstStageFlags
    , 0
    , 0
    , nullptr
    , 0
    , nullptr
    , 1
    , &vkImgMemBarrier
  );

  VkBufferImageCopy vkBufferRgn = {};
  vkBufferRgn.bufferOffset = 0;
  vkBufferRgn.bufferRowLength = 0;
  vkBufferRgn.bufferImageHeight = 0;
  vkBufferRgn.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vkBufferRgn.imageSubresource.mipLevel = 0;
  vkBufferRgn.imageSubresource.baseArrayLayer = 0;
  vkBufferRgn.imageSubresource.layerCount = 1;
  vkBufferRgn.imageOffset = { 0, 0, 0 };
  vkBufferRgn.imageExtent = { extent.width, extent.height, 1 };

  vkCmdCopyBufferToImage(
      vkCommandBuffer
    , vkStagingBuffer
    , vkTexture
    , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    , 1
    , &vkBufferRgn
  );

  vkSrcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  vkDstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  vkImgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  vkImgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  vkImgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  vkImgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  vkImgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vkImgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vkImgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vkImgMemBarrier.subresourceRange.baseMipLevel = 0;
  vkImgMemBarrier.subresourceRange.levelCount = 1;
  vkImgMemBarrier.subresourceRange.baseArrayLayer = 0;
  vkImgMemBarrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(
      vkCommandBuffer
    , vkSrcStageFlags
    , vkDstStageFlags
    , 0
    , 0
    , nullptr
    , 0
    , nullptr
    , 1
    , &vkImgMemBarrier
  );

  return VK_SUCCESS;
}

}
