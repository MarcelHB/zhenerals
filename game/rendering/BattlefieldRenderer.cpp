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
  , battlefield(battlefield)
  , modelRenderer {vuglContext, textureCache, modelCache}
  , terrains(terrains)
  , waterSettings(waterSettings)
{}

bool BattlefieldRenderer::init(Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto map = battlefield.getMap();
  auto size = map->getSize();

  auto vp = vuglContext.getViewport();
  battlefield.setPerspectiveProjection(
      0.1f
    , 1000.0f
    , 60.0f
    , vp.width * 1.0f
    , vp.height * 1.0f
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

  if (!modelRenderer.preparePipeline(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not setup model rendering");
    return false;
  }

  return true;
}

std::shared_ptr<Vugl::CommandBuffer> BattlefieldRenderer::createRenderList(size_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto newMatrices = battlefield.cameraHasMoved();
  auto commandBuffer = vuglContext.createCommandBuffer(frameIdx, true);

  std::array<VkClearValue, 2> clearColors{};
  clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearColors[1].depthStencil = {1.0f, 0};
  commandBuffer.beginRendering(renderPass, clearColors);

  renderTerrain(commandBuffer, frameIdx);
  renderObjectInstances(commandBuffer, frameIdx, newMatrices);
  // TODO this still needs better Z ordering
  renderWater(commandBuffer, frameIdx);

  commandBuffer.closeRendering();

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
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

  pipelineSetup.vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;

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

  pipelineSetup.vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;

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
  bool success = false;

  auto base = instance.getBase();
  // TODO differentiation
  if (base->drawMetaData.type == Objects::DrawType::DEPENDENCY_MODEL_DRAW
      || base->drawMetaData.type == Objects::DrawType::MODEL_DRAW
      || base->drawMetaData.type == Objects::DrawType::OVERLORD_AIRCRAFT_DRAW
      || base->drawMetaData.type == Objects::DrawType::SUPPLY_DRAW
      || base->drawMetaData.type == Objects::DrawType::TANK_DRAW
      || base->drawMetaData.type == Objects::DrawType::TRUCK_DRAW
  ) {
    success = prepareModelDrawData(instance);
  } else if (base->drawMetaData.type == Objects::DrawType::TREE_DRAW) {
    success = prepareTreeDrawData(instance);
  } else {
    WARN_ZH(
        "BattlefieldRenderer"
      , "Skipping drawing of {}, draw mode not implemented"
      , instance.getBase()->name
    );
    return true;
  }

  if (!success) {
    return false;
  }

  auto instanceID = instance.getID();
  auto lookup = boundingSpheres.find(instanceID);
  if (lookup != boundingSpheres.cend()) {
    return true;
  }

  auto sphere = modelRenderer.getBoundingSphere(instanceID);
  boundingSpheres.emplace(instanceID, std::move(sphere));

  return true;
}

bool BattlefieldRenderer::prepareModelDrawData(Objects::Instance& instance) {
  auto base = instance.getBase();
  auto drawData = static_pointer_cast<Objects::ModelDrawData>(base->drawMetaData.drawData);

  // EVAL condition states
  if (drawData->defaultConditionState.model.empty() && drawData->conditionStates.empty()) {
    WARN_ZH("BattlefieldRenderer", "No suitable condition state for {}", instance.getBase()->name);
    return false;
  }

  std::string modelName;
  if (!drawData->defaultConditionState.model.empty()) {
    modelName = drawData->defaultConditionState.model;
  } else {
    modelName = drawData->conditionStates.begin()->model;
  }

  if (!modelRenderer.prepareModel(instance.getID(), modelName)) {
    WARN_ZH("BattlefieldRenderer", "Unable to find models for {}", instance.getBase()->name);
    return false;
  }

  return true;
}

bool BattlefieldRenderer::prepareTreeDrawData(Objects::Instance& instance) {
  auto base = instance.getBase();
  auto drawData = static_pointer_cast<Objects::TreeDrawData>(base->drawMetaData.drawData);

  if (!modelRenderer.prepareModel(instance.getID(), drawData->model)) {
    WARN_ZH("BattlefieldRenderer", "Unable to find models for {}", instance.getBase()->name);
    return false;
  }

  return true;
}

void BattlefieldRenderer::renderObjectInstances(
    Vugl::CommandBuffer& commandBuffer
  , size_t frameIdx
  , bool newMatrices
) {
  TRACY(ZoneScoped);

  modelRenderer.beginResourceCounting();

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

  modelRenderer.bindPipeline(commandBuffer);

  // TODO instance movement, check for new/deleted instances
  if (newMatrices) {
    auto map = battlefield.getMap();
    auto& camMatrix = battlefield.getCamera().getCameraMatrix();
    drawChecks.clear();
    drawChecks.reserve(battlefield.getObjectInstances().size());

    for (auto& instance : battlefield.getObjectInstances()) {
      auto& drawCheck = drawChecks.emplace_back();
      drawCheck.instance = instance;
      drawCheck.sphere = boundingSpheres.find(instance->getID())->second;

      auto modelMatrix = battlefield.getObjectToGridMatrix(instance->getPosition(), 0.0f);
      drawCheck.sphere.position = glm::vec3 {camMatrix * modelMatrix * glm::vec4 {drawCheck.sphere.position, 1.0f}};
      drawCheck.dist = glm::length(drawCheck.sphere.position);
    }

    std::sort(
        drawChecks.begin()
      , drawChecks.end()
      , [](const DrawCheck& a, const DrawCheck& b) { return a.dist > b.dist; }
    );
  }

  for (auto& dc : drawChecks) {
    if (dc.draw) {
      renderObjectInstance(*dc.instance, commandBuffer, frameIdx, newMatrices);
    }
  }

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }

  modelRenderer.finishResourceCounting();
}

void BattlefieldRenderer::renderObjectInstance(
    Objects::Instance& instance
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIdx
  , bool newMatrices
) {

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel(std::to_string(instance.getID()));
  }

  if (newMatrices
      || instance.needsRedraw()
      || modelRenderer.needsUpdate(instance.getID(), frameIdx)
  ) {
    auto modelMatrix =
      battlefield.getObjectToGridMatrix(
          instance.getPosition()
        , instance.getAngle()
      );

    auto normalMatrix =
      glm::rotate(glm::mat4 {1.0f}, instance.getAngle(), glm::vec3{0.0f, 1.0f, 0.0f});

    auto& camera = battlefield.getCamera();
    auto mvp =
      camera.getProjectionMatrix()
      * camera.getCameraMatrix()
      * modelMatrix;

    modelRenderer.updateModel(
        instance.getID()
      , frameIdx
      , newMatrices
      , mvp
      , camera.getCameraMatrix()
      , normalMatrix
      , sunlightNormal
    );

    instance.setRedrawn();
  }

  modelRenderer.renderModel(instance.getID(), commandBuffer);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void BattlefieldRenderer::renderTerrain(Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Terrain");
  }

  TerrainScene scene;
  auto& camera = battlefield.getCamera();
  scene.mvp =
    camera.getProjectionMatrix() * camera.getCameraMatrix();
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
  auto& camera = battlefield.getCamera();
  scene.mvp =
    camera.getProjectionMatrix() * camera.getCameraMatrix();
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
