#include "ModelRenderer.h"

namespace ZH {

ModelRenderer::ModelRenderer(
    Vugl::Context& vuglContext
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , modelCache(modelCache)
{}

void ModelRenderer::beginResourceCounting() {
  for (auto& pair : renderDataMap) {
    pair.second->increaseMiss();
  }
}

void ModelRenderer::finishResourceCounting() {
  for (auto it = renderDataMap.begin(); it != renderDataMap.end();) {
    if (it->second->getMisses() >= 2) {
      it = renderDataMap.erase(it);
    } else {
      it++;
    }
  }
}

bool ModelRenderer::preparePipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  pipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  pipelineSetup.setVSCode(readFile("shaders/model.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/model.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  pipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (pipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

bool ModelRenderer::prepareModel(uint64_t id, const std::string& modelName) {
  auto lookup = renderDataMap.find(id);
  if (lookup != renderDataMap.cend()) {
    return true;
  }

  auto models = modelCache.getModels(modelName);
  if (!models) {
    return false;
  }

  MurmurHash3_32 hasher;
  hasher.feed(modelName);
  uint32_t vertexKey = hasher.getHash();

  auto renderData = std::make_shared<RenderData>();
  renderData->vertexKey = vertexKey;
  renderData->transformations.resize(models->size());
  renderData->numModels = models->size();
  renderData->uniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(ShaderData)));

  uint32_t i = 0;
  for (auto& model : *models) {
    MurmurHash3_32 hasher;
    hasher.feed(vertexKey);
    hasher.feed(i);
    auto key = hasher.getHash();

    auto& descriptorSet =
      renderData->descriptorSets.emplace_back(pipeline->createDescriptorSet());
    renderData->transformations[i] = model->transformation;

    auto vertexLookup = vertexData.find(key);
    if (vertexLookup == vertexData.cend()) {
      auto modelVertices =
        std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));

      modelVertices->setBigIndexBuffer(true);
      modelVertices->writeData(model->vertexData, model->vertexIndices);

      vuglContext.uploadResource(*modelVertices);

      vertexData.emplace(std::make_pair(key, std::move(modelVertices)));
    }

    i += 1;

    // EVAL per-triangle texture
    auto& textureName = model->textures[0];
    auto sampler = textureCache.getTextureSampler(textureName);
    if (!sampler) {
      WARN_ZH("BattlefieldRenderer", "Failed to load model texture {}", textureName);
      return false;
    }

    descriptorSet.assignUniformBuffer(*renderData->uniformBuffer);
    descriptorSet.assignCombinedSampler(*sampler);
    vuglContext.uploadResource(*sampler);

    descriptorSet.updateDevice();
  }

  renderDataMap.emplace(std::make_pair(id, std::move(renderData)));

  return true;
}

void ModelRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  commandBuffer.bindResource(*pipeline);
}

bool ModelRenderer::needsUpdate(uint64_t id, size_t frameIdx) const {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return false;
  }

  auto& renderData = lookup->second;

  return (renderData->frameIdxSet & (1 << frameIdx)) == 0;
}

void ModelRenderer::updateModel(
    uint64_t id
  , size_t frameIdx
  , bool newMatrices
  , const glm::mat4& mvp
  , const glm::mat4& normal
  , const glm::vec3& sunlightNormal
) {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return;
  }

  glm::mat4 axisFlip {1.0f};
  axisFlip[1][1] = 0.0f;
  axisFlip[1][2] = 1.0f;
  axisFlip[2][1] = 1.0f;
  axisFlip[2][2] = 0.0f;

  auto& renderData = lookup->second;
  // EVAL multi part models with different transformation
  glm::mat4 transformRotation = renderData->transformations[0];
  transformRotation[3] = glm::vec4 {0.0f};

  renderData->shaderData.mvp = mvp * axisFlip * renderData->transformations[0];
  renderData->shaderData.normalMatrix =
    normal
      * axisFlip
      * transformRotation;
  renderData->shaderData.sunlight = sunlightNormal;
  if (newMatrices) {
    renderData->frameIdxSet = 0;
  }
  renderData->frameIdxSet |= (1 << frameIdx);

  renderData->uniformBuffer->writeData(renderData->shaderData, frameIdx);
}

bool ModelRenderer::renderModel(uint64_t id, Vugl::CommandBuffer& commandBuffer) {
  auto renderDataLookup = renderDataMap.find(id);
  if (renderDataLookup == renderDataMap.cend()) {
    return false;
  }

  auto& renderData = renderDataLookup->second;
  renderData->decreaseMiss();

  for (size_t i = 0; i < renderData->numModels; ++i) {
    MurmurHash3_32 hasher;
    hasher.feed(renderData->vertexKey);
    hasher.feed(i);
    auto key = hasher.getHash();

    auto elementBufferLookup = vertexData.find(key);
    if (elementBufferLookup == vertexData.cend()) {
      continue;
    }
    auto& elementBuffer = elementBufferLookup->second;

    commandBuffer.bindResource(renderData->descriptorSets[i]);
    commandBuffer.bindResource(*elementBuffer);

    auto numIndices = elementBuffer->getNumIndices();
    commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
      vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);
      return VK_SUCCESS;
    });
  }

  return true;
}

}
