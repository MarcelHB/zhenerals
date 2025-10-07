// SPDX-License-Identifier: GPL-2.0

#include <algorithm>

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
  renderData->shaderData.resize(models->size());

  renderData->orderData.resize(models->size() * 8);
  renderData->drawOrder.resize(models->size());

  uint32_t i = 0;
  for (auto& model : *models) {
    MurmurHash3_32 hasher;
    hasher.feed(vertexKey);
    hasher.feed(i);
    auto key = hasher.getHash();

    auto& descriptorSet =
      renderData->descriptorSets.emplace_back(pipeline->createDescriptorSet());
    auto& uniformBuffer =
      renderData->uniformBuffers.emplace_back(vuglContext.createUniformBuffer(sizeof(ShaderData)));
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

    calculateBoundingCorners(*model, *renderData, i);

    i += 1;

    // EVAL per-triangle texture
    auto& textureName = model->textures[0];
    auto sampler = textureCache.getTextureSampler(textureName);
    if (!sampler) {
      WARN_ZH("BattlefieldRenderer", "Failed to load model texture {}", textureName);
      return false;
    }

    descriptorSet.assignUniformBuffer(uniformBuffer);
    descriptorSet.assignCombinedSampler(*sampler);
    vuglContext.uploadResource(*sampler);

    descriptorSet.updateDevice();
  }

  renderDataMap.emplace(std::make_pair(id, std::move(renderData)));

  return true;
}

void ModelRenderer::calculateBoundingCorners(const Model& model, RenderData& renderData, size_t i) {
  auto x = model.getExtremes();

  auto& od = renderData.orderData;
  od[i * 8 + 0] = {x[0][0], x[0][1], x[0][2]};
  od[i * 8 + 1] = {x[1][0], x[0][1], x[0][2]};
  od[i * 8 + 2] = {x[0][0], x[1][1], x[0][2]};
  od[i * 8 + 3] = {x[0][0], x[0][1], x[1][2]};
  od[i * 8 + 4] = {x[1][0], x[1][1], x[1][2]};
  od[i * 8 + 5] = {x[0][0], x[1][1], x[1][2]};
  od[i * 8 + 6] = {x[1][0], x[0][1], x[1][2]};
  od[i * 8 + 7] = {x[1][0], x[1][1], x[0][2]};
}

void ModelRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  commandBuffer.bindResource(*pipeline);
}

ModelRenderer::BoundingSphere ModelRenderer::getBoundingSphere(uint64_t id) const {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return {};
  }

  auto& renderData = lookup->second;
  glm::vec3 min {std::numeric_limits<float>::max()};
  glm::vec3 max {std::numeric_limits<float>::min()};

  for (auto& corner : renderData->orderData) {
    for (uint8_t i = 0; i < 3; ++i) {
      min[i] = std::min(min[i], corner[i]);
      max[i] = std::max(max[i], corner[i]);
    }
  }

  float maxDistance = 0.0f;
  glm::vec3 center {0.0f};
  for (uint8_t i = 0; i < 3; ++i) {
    maxDistance = std::max(maxDistance, max[i] - min[i]);
    center[i] = max[i] - (max[i] - min[i]) / 2.0f;
  }

  return {std::move(center), maxDistance / 2.0f};
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
  , const glm::mat4& camera
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
  for (size_t i = 0; i < renderData->numModels; ++i) {
    glm::mat4 transformRotation = renderData->transformations[i];
    transformRotation[3] = glm::vec4 {0.0f};

    auto& shaderData = renderData->shaderData[i];
    shaderData.mvp = mvp * axisFlip * renderData->transformations[i];
    shaderData.normalMatrix =
      normal
        * axisFlip
        * transformRotation;
    shaderData.sunlight = sunlightNormal;

    if (newMatrices) {
      renderData->frameIdxSet = 0;
    }
    renderData->frameIdxSet |= (1 << frameIdx);

    renderData->uniformBuffers[i].writeData(shaderData, frameIdx);
  }

  if (renderData->numModels == 1) {
    renderData->drawOrder[0] = 0;
    return;
  }

  using Order = std::pair<size_t, float>;
  std::vector<Order> transformedOrder;
  transformedOrder.resize(renderData->orderData.size());

  for (size_t i = 0; i < renderData->orderData.size(); ++i) {
    auto& to = transformedOrder[i];
    auto& od = renderData->orderData[i];

    to.first = i / 8;
    to.second = glm::vec3 {camera * glm::vec4 {od, 1.0f}}[2];
  }

  std::sort(
      transformedOrder.begin()
    , transformedOrder.end()
    , [](const Order& a, const Order& b) { return a.second > b.second; }
  );

  for (size_t i = 0, j = 0; i < transformedOrder.size(); ++i) {
    auto v = transformedOrder[i].first;

    if (i == 0 || renderData->drawOrder[j - 1] != v) {
      renderData->drawOrder[j++] = v;
    }

    if (j == renderData->drawOrder.size()) {
      break;
    }
  }
}

bool ModelRenderer::renderModel(uint64_t id, Vugl::CommandBuffer& commandBuffer) {
  auto renderDataLookup = renderDataMap.find(id);
  if (renderDataLookup == renderDataMap.cend()) {
    return false;
  }

  auto& renderData = renderDataLookup->second;
  renderData->decreaseMiss();

  for (auto i : renderData->drawOrder) {
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
