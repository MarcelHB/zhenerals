// SPDX-License-Identifier: GPL-2.0

#ifndef H_VUGL_DESCRIPTOR_SET
#define H_VUGL_DESCRIPTOR_SET

#include <optional>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "vugl_bindable_resource.h"
#include "vugl_combined_sampler.h"
#include "vugl_texture.h"
#include "vugl_uniform_buffer.h"

namespace Vugl {

class DescriptorSet : public BindableResource {
  private:
    enum class DescriptorType {
        SAMPLER
      , COMBINED_SAMPLER
      , SAMPLED_IMAGE
      , STORAGE_IMAGE
      , UBO
      , DYNAMIC_UBO
      , NULL_SAMPLER
    };
    using BindingInfo = std::tuple<DescriptorType, size_t>;

    VkDevice vkDevice;
    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkPipelineLayout vkPipelineLayout;
    uint32_t numSwapchainImages;
    VkPipelineBindPoint vkPipelineBindPoint;
    VkResult vkLastResult;

    VkDescriptorPool vkDescriptorPool;
    std::vector<VkDescriptorSet> vkDescriptorSets;

    std::vector<std::reference_wrapper<const CombinedSampler>> assignedCombinedSamplers;
    std::vector<std::reference_wrapper<const Sampler>> assignedSamplers;
    std::vector<std::reference_wrapper<const Texture>> assignedStorageImages;
    std::vector<std::reference_wrapper<const Texture>> assignedTextures;
    std::vector<std::reference_wrapper<const UniformBuffer>> assignedUniformBuffers;

    std::vector<BindingInfo> bindings;
    std::unordered_map<uint32_t, std::vector<std::reference_wrapper<const Texture>>> textureGrouping;

  public:
    DescriptorSet (DescriptorSet &&);
    DescriptorSet (
        VkDevice vkDevice
      , VkPipelineLayout vkPipelineLayout
      , VkDescriptorSetLayout vkDescriptorSetLayout
      , uint32_t numSwapchainImages
    );
    ~DescriptorSet ();

    void assignCombinedSampler (const CombinedSampler&);
    void assignSampler (const Sampler&);
    void assignStorageImage (const Texture&);
    void assignTexture (const Texture&, std::optional<uint32_t> binding = {});
    void assignUniformBuffer (const UniformBuffer&, bool dynamic = false);

    void destroy ();
    VkResult getLastResult () const;
    VkDescriptorSet getVkDescriptorSet (size_t index) const;
    VkResult recordBindCommands (VkCommandBuffer vkCommandBuffer, uint32_t i) override;
    void setPipelineBindPoint (VkPipelineBindPoint bindPoint);
    void updateDevice ();
};

};

#endif
