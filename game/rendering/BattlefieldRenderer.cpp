#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "BattlefieldRenderer.h"

namespace ZH {

struct TerrainScene {
  alignas(16) glm::mat4 mvp;
  alignas(16) glm::vec3 sunlight;
};

struct WaterScene {
  alignas(16) glm::mat4 mvp;
};

BattlefieldRenderer::BattlefieldRenderer(
    Vugl::Context& vuglContext
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
  , const TerrainINI::Terrains& terrains
  , const WaterINI::WaterSettings& waterSettings
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , modelCache(modelCache)
  , battlefield(battlefield)
  , terrains(terrains)
  , waterSettings(waterSettings)
{}

bool BattlefieldRenderer::init(Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto map = battlefield.getMap();
  auto size = map->getSize();

  auto vp = vuglContext.getViewport();
  projectMatrix =
    glm::perspective(
        glm::radians(90.0f)
      , vp.width / static_cast<float>(vp.height)
      , 0.1f
      , 1000.0f
    );

  auto lightTarget =
    glm::vec3 {
        size.x / 2.0f
      , 0
      , size.y / 2.0f
    };

  auto lightPos =
    glm::vec3 {
        size.x * 0.25f
      , 255 * -0.5f
      , size.y * 0.75f
    };

  sunlightNormal = glm::normalize(lightTarget - lightPos);

  if (!prepareTerrainVertices()) {
    WARN_ZH("BattlefieldRenderer", "Could not set up terrain");
    return false;
  }

  if (!prepareTerrainPipeline(renderPass, map->getTexturesIndex())) {
    WARN_ZH("BattlefieldRenderer", "Could not setup up terrain rendering");
    return false;
  }

  if (!map->getWater().empty()) {
    if (!prepareWaterVertices()) {
      WARN_ZH("BattlefieldRenderer", "Could not set up water");
      return false;
    }

    if (!prepareWaterPipeline(renderPass)) {
      WARN_ZH("BattlefieldRenderer", "Could not setup up water rendering");
      return false;
    }

    hasWater = true;
  }

  if (!prepareModelPipeline(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not setup model rendering");
    return false;
  }

  return true;
}

std::shared_ptr<Vugl::CommandBuffer> BattlefieldRenderer::createRenderList(size_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto commandBuffer = vuglContext.createCommandBuffer(frameIdx, true);

  std::array<VkClearValue, 2> clearColors{};
  clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearColors[1].depthStencil = {1.0f, 0};
  commandBuffer.beginRendering(renderPass, clearColors);

  renderTerrain(commandBuffer, frameIdx);
  renderObjectInstances(commandBuffer, frameIdx);
  renderWater(commandBuffer, frameIdx);

  commandBuffer.closeRendering();

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
}

bool BattlefieldRenderer::prepareModelPipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.setVSCode(readFile("shaders/model.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/model.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  modelPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (modelPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

bool BattlefieldRenderer::prepareTerrainPipeline(
    Vugl::RenderPass& renderPass
  , const std::vector<std::string>& texturesIndex
) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.setVSCode(readFile("shaders/terrain.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/terrain.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveSampler(VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveTexture(VK_SHADER_STAGE_FRAGMENT_BIT, texturesIndex.size());
  pipelineSetup.reserveTexture(VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 32, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 36, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_SFLOAT, 40, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 44, 8, 0);

  terrainPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (terrainPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  terrainUniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(TerrainScene)));
  terrainDescriptorSet =
    std::make_shared<Vugl::DescriptorSet>(terrainPipeline->createDescriptorSet());
  terrainTextureSampler =
    std::make_shared<Vugl::Sampler>(vuglContext.createSampler());
  terrainDescriptorSet->assignUniformBuffer(*terrainUniformBuffer);
  terrainDescriptorSet->assignSampler(*terrainTextureSampler);

  terrainTextures.reserve(texturesIndex.size());
  for (auto& keyName : texturesIndex) {
    auto terrainLookup = terrains.find(keyName);
    if (terrainLookup == terrains.cend()) {
      WARN_ZH("BattlefieldRenderer", "Terrain not found");
      continue;
    }

    auto texture = terrainTextures.emplace_back(textureCache.getTexture(terrainLookup->second.textureName));
    if (!texture) {
      WARN_ZH("BattlefieldRenderer", "Terrain texture not found");
      continue;
    }
    terrainDescriptorSet->assignTexture(*texture, 2);
    vuglContext.uploadResource(*texture);
  }

  cloudTexture = textureCache.getTexture("tscloudmed.dds");
  if (!cloudTexture) {
    return false;
  }

  terrainDescriptorSet->assignTexture(*cloudTexture);
  vuglContext.uploadResource(*cloudTexture);

  terrainDescriptorSet->updateDevice();

  return true;
}

bool BattlefieldRenderer::prepareTerrainVertices() {
  terrainVertices =
    std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  auto map = battlefield.getMap();

  terrainVertices->setBigIndexBuffer(true);
  terrainVertices->writeData(map->getVertexData(), map->getVertexIndices());
  if (terrainVertices->getLastResult() != VK_SUCCESS) {
    return false;
  }

  vuglContext.uploadResource(*terrainVertices);

  return true;
}

bool BattlefieldRenderer::prepareWaterPipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  pipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  pipelineSetup.setVSCode(readFile("shaders/water.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/water.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  pipelineSetup.reserveSampler(VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveTexture(VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveTexture(VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 12, 8, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_SFLOAT, 20, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  waterPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (waterPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  waterUniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(WaterScene)));
  waterDescriptorSet =
    std::make_shared<Vugl::DescriptorSet>(waterPipeline->createDescriptorSet());
  waterDescriptorSet->assignUniformBuffer(*waterUniformBuffer);
  waterDescriptorSet->assignSampler(*terrainTextureSampler);

  waterTexture = textureCache.getTexture("twwater01.dds");
  if (!waterTexture) {
    return false;
  }

  vuglContext.uploadResource(*waterTexture);
  waterDescriptorSet->assignTexture(*waterTexture);
  waterDescriptorSet->assignTexture(*cloudTexture);

  waterDescriptorSet->updateDevice();

  return true;
}

bool BattlefieldRenderer::prepareWaterVertices() {
  waterVertices =
    std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  auto map = battlefield.getMap();

  waterVertices->writeData(map->getWaterVertices(), std::vector<uint16_t>{});
  if (waterVertices->getLastResult() != VK_SUCCESS) {
    return false;
  }

  vuglContext.uploadResource(*waterVertices);

  return true;
}

bool BattlefieldRenderer::prepareModelData(Objects::Instance& instance) {
  auto lookup = modelRenderData.find(instance.getID());

  if (lookup != modelRenderData.cend()) {
    lookup->second->decreaseMiss();
    return true;
  }

  auto base = instance.getBase();
  if (base->drawMetaData.type == Objects::DrawType::MODEL_DRAW) {
      prepareModelDrawData(instance);
  } else if (base->drawMetaData.type == Objects::DrawType::TREE_DRAW) {
    // Silence
  } else {
    WARN_ZH(
        "BattlefieldRenderer"
      , "Skipping drawing of {}, draw mode not implemented"
      , instance.getBase()->name
    );
  }

  return true;
}

bool BattlefieldRenderer::prepareModelDrawData(Objects::Instance& instance) {
  auto base = instance.getBase();
  auto drawData = static_pointer_cast<Objects::ModelDrawData>(base->drawMetaData.drawData);

  // EVAL condition states
  auto models = modelCache.getModels(drawData->defaultConditionState.model);
  if (models == nullptr) {
    WARN_ZH("BattlefieldRenderer", "Unable to find models for {}", drawData->defaultConditionState.model);
    return false;
  }

  MurmurHash3_32 hasher;
  hasher.feed(drawData->defaultConditionState.model);
  uint32_t key = hasher.getHash();

  auto renderData = std::make_shared<ModelRenderData>();
  renderData->vertexKey = key;
  renderData->descriptorSet =
    std::make_shared<Vugl::DescriptorSet>(modelPipeline->createDescriptorSet());
  renderData->uniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(ModelData)));

  renderData->descriptorSet->assignUniformBuffer(*renderData->uniformBuffer);

  // EVAL all models
  // TODO better cleanup/avoidance if either setup fails
  auto& model = models->front();
  auto vertexLookup = vertexData.find(renderData->vertexKey);
  if (vertexLookup == vertexData.cend()) {
    auto modelVertices =
      std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));

    modelVertices->setBigIndexBuffer(true);
    modelVertices->writeData(model->vertexData, model->vertexIndices);

    vuglContext.uploadResource(*modelVertices);

    vertexData.emplace(std::make_pair(renderData->vertexKey, std::move(modelVertices)));
  }

  // EVAL per-triangle texture
  auto& textureName = model->textures[0];
  auto sampler = textureCache.getTextureSampler(textureName);
  if (!sampler) {
    WARN_ZH("BattlefieldRenderer", "Failed to load model texture {}", textureName);
    return false;
  }

  renderData->descriptorSet->assignCombinedSampler(*sampler);
  vuglContext.uploadResource(*sampler);

  renderData->descriptorSet->updateDevice();
  modelRenderData.emplace(std::make_pair(instance.getID(), std::move(renderData)));

  return true;
}

void BattlefieldRenderer::renderObjectInstances(Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  for (auto& pair : modelRenderData) {
    pair.second->increaseMiss();
  }

  for (auto& instance : battlefield.getObjectInstances()) {
    if (!prepareModelData(*instance)) {
      WARN_ZH(
          "BattlefieldRenderer"
        , "Skipping drawing of {}, model not loaded"
        , instance->getBase()->name
      );
      continue;
    }
  }

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Objects");
  }

  commandBuffer.bindResource(*modelPipeline);

  for (auto& instance : battlefield.getObjectInstances()) {
    // TODO visibility check
    renderObjectInstance(*instance, commandBuffer, frameIdx);
  }

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }

  for (auto it = modelRenderData.begin(); it != modelRenderData.end();) {
    if (it->second->getMisses() >= 2) {
      it = modelRenderData.erase(it);
    } else {
      it++;
    }
  }
}

void BattlefieldRenderer::renderObjectInstance(Objects::Instance& instance, Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  auto renderDataLookup = modelRenderData.find(instance.getID());
  if (renderDataLookup == modelRenderData.cend()) {
    return;
  }
  auto& renderData = renderDataLookup->second;

  auto elementBufferLookup = vertexData.find(renderData->vertexKey);
  if (elementBufferLookup == vertexData.cend()) {
    return;
  }
  auto& elementBuffer = elementBufferLookup->second;

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel(std::to_string(instance.getID()));
  }

  if (instance.needsRedraw()) {
    auto modelMatrix =
      battlefield.getObjectToWorldMatrix(
          instance.getPosition()
        , instance.getAngle()
      );

    glm::mat4 axisFlip {1.0f};
    axisFlip[1][1] = 0.0f;
    axisFlip[1][2] = 1.0f;
    axisFlip[2][1] = 1.0f;
    axisFlip[2][2] = 0.0f;

    auto normalMatrix =
      glm::rotate(axisFlip, instance.getAngle(), glm::vec3{0.0f, 1.0f, 0.0f});

    renderData->modelData.mvp =
      projectMatrix * battlefield.getCameraMatrix() * modelMatrix;
    renderData->modelData.sunlight = sunlightNormal;
    renderData->modelData.normalMatrix = normalMatrix;

    instance.setRedrawn();
  }

  renderData->uniformBuffer->writeData(renderData->modelData, frameIdx);

  commandBuffer.bindResource(*renderData->descriptorSet);
  commandBuffer.bindResource(*elementBuffer);

  auto numIndices = elementBuffer->getNumIndices();
  commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);
    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void BattlefieldRenderer::renderTerrain(Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Terrain");
  }

  TerrainScene scene;
  scene.mvp =
    projectMatrix * battlefield.getCameraMatrix();
  scene.sunlight = sunlightNormal;
  terrainUniformBuffer->writeData(scene, frameIdx);

  commandBuffer.bindResource(*terrainPipeline);
  commandBuffer.bindResource(*terrainDescriptorSet);
  commandBuffer.bindResource(*terrainVertices);

  auto map = battlefield.getMap();
  auto numIndices = map->getVertexIndices().size();
  commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);
    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void BattlefieldRenderer::renderWater(Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  if (!hasWater) {
    return;
  }

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Water");
  }

  auto map = battlefield.getMap();
  WaterScene scene;
  scene.mvp = projectMatrix * battlefield.getCameraMatrix();
  waterUniformBuffer->writeData(scene, frameIdx);

  commandBuffer.bindResource(*waterPipeline);
  commandBuffer.bindResource(*waterDescriptorSet);
  commandBuffer.bindResource(*waterVertices);

  auto numVertices = map->getWaterVertices().size();
  commandBuffer.draw([numVertices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDraw(vkCommandBuffer, numVertices, 1, 0, 0);
    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

}
