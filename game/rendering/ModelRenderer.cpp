// SPDX-License-Identifier: GPL-2.0

#include <algorithm>
#include <cassert>

#include "Geometry.h"
#include "gfx/VkExt.h"
#include "ModelRenderer.h"

namespace ZH {

ModelRenderer::ModelRenderer(
    Vugl::Context& vuglContext
  , const Config& config
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
) : vuglContext(vuglContext)
  , config(config)
  , textureCache(textureCache)
  , modelCache(modelCache)
{}

static constexpr uint32_t MAX_PIVOT_MATRICES = 40;

void ModelRenderer::beginResourceCounting() {
  for (auto& pair : renderDataMap) {
    pair.second.increaseMiss();
  }
}

void ModelRenderer::finishResourceCounting() {
  for (auto it = renderDataMap.begin(); it != renderDataMap.end();) {
    // for now try garbage collection once in 60s (if FPS isn't too low)
    if (it->second.getMisses() >= config.refreshRate.value_or(60) * 60) {
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
  pipelineSetup.addDynamicState(VK_DYNAMIC_STATE_CULL_MODE);

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  pipelineSetup.vkDescriptorSetLayoutBindings[2].descriptorCount = MAX_PIVOT_MATRICES;

  VkDescriptorBindingFlags vkDynamicDescriptorCountsFlags[] = {
      0
    , 0
    , VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
  };
  VkDescriptorSetLayoutBindingFlagsCreateInfo vkDynamicDescriptorCounts = {};
  vkDynamicDescriptorCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  vkDynamicDescriptorCounts.bindingCount = 3;
  vkDynamicDescriptorCounts.pBindingFlags = vkDynamicDescriptorCountsFlags;

  pipelineSetup.vkDescriptorSetLayoutCreateInfo.pNext = &vkDynamicDescriptorCounts;

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 32, 4, 0);

  pipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (pipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

bool ModelRenderer::prepareModel(
    uint64_t id
  , const std::string& modelName
  , std::vector<glm::mat4>&& pivotMatrices
) {
  TRACY(ZoneScoped);

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

  RenderData renderData;
  renderData.vertexKey = vertexKey;
  renderData.transformations.resize(models->size());
  renderData.numModels = models->size();
  renderData.shaderData.resize(models->size());

  renderData.drawOrder.resize(models->size());
  renderData.backfaceCulling.resize(models->size());
  renderData.boundingSpheres.resize(models->size());
  renderData.boundingSphere = (*models)[0]->boundingSphere;

  createPivotBuffer(renderData, std::move(pivotMatrices));

  if (models->size() > 1) {
    std::vector<Sphere> spheres;
    spheres.reserve(models->size());

    uint32_t i = 0;
    for (auto& model : *models) {
      Sphere sphere = model->boundingSphere;
      sphere.position =
        glm::vec3 {model->transformation * glm::vec4 {sphere.position, 1.0f}};
      spheres.push_back(sphere);
    }

    renderData.boundingSphere = getSphereFromSpheres(spheres.cbegin(), spheres.cend());
  }

  uint32_t i = 0;
  for (auto& model : *models) {
    MurmurHash3_32 hasher;
    hasher.feed(vertexKey);
    hasher.feed(i);
    auto key = hasher.getHash();

    auto& descriptorSet =
      renderData.descriptorSets.emplace_back(pipeline->createDescriptorSet());
    auto& uniformBuffer =
      renderData.uniformBuffers.emplace_back(vuglContext.createUniformBuffer(sizeof(ShaderData)));
    renderData.transformations[i] = model->transformation;
    renderData.backfaceCulling[i] = model->backfaceCulling;

    auto vertexLookup = vertexData.find(key);
    if (vertexLookup == vertexData.cend()) {
      auto modelVertices =
        std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));

      modelVertices->setBigIndexBuffer(true);
      modelVertices->writeData(model->vertexData, model->vertexIndices);

      vuglContext.uploadResource(*modelVertices);

      vertexData.emplace(std::make_pair(key, std::move(modelVertices)));
    }

    renderData.boundingSpheres[i] = {
        glm::vec3 {renderData.transformations[i] * glm::vec4 {model->boundingSphere.position, 1.0f}}
      , model->boundingSphere.radius
    };

    i += 1;

    // EVAL per-triangle texture
    // EVAL somethings without textures
    std::string textureName {"cbsandbw.dds"};
    if (!model->textures.empty()) {
      textureName = model->textures.back();
    }
    auto sampler = textureCache.getTextureSampler(textureName, true);
    if (!sampler) {
      WARN_ZH("BattlefieldRenderer", "Failed to load model texture {}", textureName);
      return false;
    }

    descriptorSet.assignUniformBuffer(uniformBuffer);
    descriptorSet.assignCombinedSampler(*sampler);
    descriptorSet.assignUniformBuffer(*renderData.pivotBuffer);
    vuglContext.uploadResource(*sampler);

    descriptorSet.updateDevice();
  }

  renderDataMap.emplace(std::make_pair(id, std::move(renderData)));

  return true;
}

void ModelRenderer::createPivotBuffer(RenderData& renderData, std::vector<glm::mat4>&& pivots) {
  assert(pivots.size() <= MAX_PIVOT_MATRICES);

  auto numPivots = pivots.size();
  renderData.pivotBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(PivotData) * numPivots, numPivots));
  renderData.pivotBuffer->setStrideSize(sizeof(PivotData));
  renderData.pivotBuffer->setVariableNumOfDescriptors(MAX_PIVOT_MATRICES);

  renderData.pivotData.resize(numPivots);
  for (size_t i = 0; i < numPivots; ++i) {
    renderData.pivotData[i].transformation = std::move(pivots[i]);
  }

  for (auto& shaderData : renderData.shaderData) {
    shaderData.pivotEnabler = 1;
  }
}

void ModelRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  commandBuffer.bindResource(*pipeline);
}

Sphere ModelRenderer::getBoundingSphere(uint64_t id) const {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return {};
  }

  return lookup->second.boundingSphere;
}

void ModelRenderer::updateModel(
    uint64_t id
  , uint32_t frameIdx
  , const glm::mat4& mvp
  , const glm::vec3& cameraPos
  , const glm::mat4& normal
  , const glm::vec3& sunlightNormal
) {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return;
  }

  auto& renderData = lookup->second;
  renderData.pivotBuffer->writeData<PivotData>(
      renderData.pivotData.cbegin()
    , renderData.pivotData.cend()
    , frameIdx
  );

  glm::mat4 axisFlip {1.0f};
  axisFlip[1][1] = 0.0f;
  axisFlip[1][2] = 1.0f;
  axisFlip[2][1] = 1.0f;
  axisFlip[2][2] = 0.0f;

  for (size_t i = 0; i < renderData.numModels; ++i) {
    glm::mat4 transformRotation = renderData.transformations[i];
    transformRotation[3] = glm::vec4 {0.0f};

    auto& shaderData = renderData.shaderData[i];
    shaderData.mvp = mvp * axisFlip * renderData.transformations[i];
    shaderData.normalMatrix =
      normal
        * axisFlip
        * transformRotation;
    shaderData.sunlight = sunlightNormal;
    renderData.uniformBuffers[i].writeData(shaderData, frameIdx);
    renderData.drawOrder[i] =
      std::make_pair(i, glm::length(cameraPos - renderData.boundingSpheres[i].position));
  }

  std::sort(
      renderData.drawOrder.begin()
    , renderData.drawOrder.end()
    , [](const OrderPair& a, const OrderPair& b) { return a.second > b.second; }
  );
}

bool ModelRenderer::renderModel(uint64_t id, Vugl::CommandBuffer& commandBuffer) {
  TRACY(ZoneScoped);

  auto renderDataLookup = renderDataMap.find(id);
  if (renderDataLookup == renderDataMap.cend()) {
    return false;
  }

  auto& renderData = renderDataLookup->second;
  renderData.decreaseMiss();

  for (auto& pair : renderData.drawOrder) {
    auto i = pair.first;
    MurmurHash3_32 hasher;
    hasher.feed(renderData.vertexKey);
    hasher.feed(i);
    auto key = hasher.getHash();

    auto elementBufferLookup = vertexData.find(key);
    if (elementBufferLookup == vertexData.cend()) {
      continue;
    }
    auto& elementBuffer = elementBufferLookup->second;

    commandBuffer.bindResource(renderData.descriptorSets[i]);
    commandBuffer.bindResource(*elementBuffer);

    auto numIndices = elementBuffer->getNumIndices();
    commandBuffer.draw([numIndices, &renderData, i](VkCommandBuffer vkCommandBuffer, uint32_t) {
      pVkCmdSetCullModeEXT(vkCommandBuffer, renderData.backfaceCulling[i] ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
      vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);

      return VK_SUCCESS;
    });
  }

  return true;
}

}
