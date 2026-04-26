// SPDX-License-Identifier: GPL-2.0

#include <algorithm>

#include "../gfx/VkExt.h"
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

void ModelRenderer::beginResourceCounting() {
  for (auto& pair : renderDataMap) {
    pair.second->increaseMiss();
  }
}

void ModelRenderer::finishResourceCounting() {
  for (auto it = renderDataMap.begin(); it != renderDataMap.end();) {
    // for now try garbage collection once in 60s (if FPS isn't too low)
    if (it->second->getMisses() >= config.refreshRate.value_or(60) * 60) {
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

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  pipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (pipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

bool ModelRenderer::prepareModel(uint64_t id, const std::string& modelName) {
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

  auto renderData = std::make_shared<RenderData>();
  renderData->vertexKey = vertexKey;
  renderData->transformations.resize(models->size());
  renderData->numModels = models->size();
  renderData->shaderData.resize(models->size());

  renderData->drawOrder.resize(models->size());
  renderData->backfaceCulling.resize(models->size());
  renderData->boundingSpheres.resize(models->size());

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
    renderData->backfaceCulling[i] = model->backfaceCulling;

    auto vertexLookup = vertexData.find(key);
    if (vertexLookup == vertexData.cend()) {
      auto modelVertices =
        std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));

      modelVertices->setBigIndexBuffer(true);
      modelVertices->writeData(model->vertexData, model->vertexIndices);

      vuglContext.uploadResource(*modelVertices);

      vertexData.emplace(std::make_pair(key, std::move(modelVertices)));
    }

    // Assume the biggest radius is the one to use for everything
    if (model->boundingSphereRadius > renderData->boundingSphere.radius) {
      renderData->boundingSphere.radius = model->boundingSphereRadius;
      renderData->boundingSphere.position =
        glm::vec3 {renderData->transformations[i] * glm::vec4 {model->boundingSphere, 1.0f}};
    }

    renderData->boundingSpheres[i] = {
        glm::vec3 {renderData->transformations[i] * glm::vec4 {model->boundingSphere, 1.0f}}
      , model->boundingSphereRadius
    };

    i += 1;

    // EVAL per-triangle texture
    // EVAL somethings without textures
    std::string textureName {"cbsandbw.dds"};
    if (!model->textures.empty()) {
      textureName = model->textures.back();
    }
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

void ModelRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  commandBuffer.bindResource(*pipeline);
}

ModelRenderer::BoundingSphere ModelRenderer::getBoundingSphere(uint64_t id) const {
  auto lookup = renderDataMap.find(id);
  if (lookup == renderDataMap.cend()) {
    return {};
  }

  return lookup->second->boundingSphere;
}

void ModelRenderer::updateModel(
    uint64_t id
  , size_t frameIdx
  , const glm::mat4& mvp
  , const glm::vec3& cameraPos
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
    renderData->uniformBuffers[i].writeData(shaderData, frameIdx);
    renderData->drawOrder[i] =
      std::make_pair(i, glm::length(cameraPos - renderData->boundingSpheres[i].position));
  }

  std::sort(
      renderData->drawOrder.begin()
    , renderData->drawOrder.end()
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
  renderData->decreaseMiss();

  for (auto& pair : renderData->drawOrder) {
    auto i = pair.first;
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
    commandBuffer.draw([numIndices, &renderData, i](VkCommandBuffer vkCommandBuffer, uint32_t) {
      pVkCmdSetCullModeEXT(vkCommandBuffer, renderData->backfaceCulling[i] ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
      vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);

      return VK_SUCCESS;
    });
  }

  return true;
}

}
