// SPDX-License-Identifier: GPL-2.0

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "../gfx/Frustum.h"
#include "BattlefieldRenderer.h"

namespace ZH {

struct TerrainScene {
  alignas(16) glm::mat4 mvp;
  alignas(16) glm::vec3 sunlight;
};

struct WaterScene {
  alignas(16) glm::mat4 mvp;
};

static constexpr float MODEL_HEIGHT_SCALE = 1.6f;

BattlefieldRenderer::BattlefieldRenderer(
    Vugl::Context& vuglContext
  , const Config& config
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
  , const TerrainINI::Terrains& terrains
  , const WaterINI::WaterSettings& waterSettings
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , battlefield(battlefield)
  , modelRenderer {vuglContext, config, textureCache, modelCache}
  , terrains(terrains)
  , waterSettings(waterSettings)
{}

bool BattlefieldRenderer::init(Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto& size = battlefield.getMapGameSize();

  auto vp = vuglContext.getViewport();
  battlefield.setPerspectiveProjection(
      10.0f
    , 10000.0f
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
      , -1500.0f
      , size.y * 0.75f
    };

  sunlightNormal = glm::normalize(lightTarget - lightPos);

  terrainScaleMatrix =
    glm::scale(
        glm::mat4 {1.0f}
      , glm::vec3 {10.0f, Map::TERRAIN_HEIGHT_SCALE, 10.0f}
      );

  waterScaleMatrix =
    glm::scale(
        glm::mat4 {1.0f}
      , glm::vec3 {10.0f, 1.0f, 10.0f}
    );

  if (!prepareTerrainVertices()) {
    WARN_ZH("BattlefieldRenderer", "Could not set up terrain");
    return false;
  }

  auto map = battlefield.getMap();
  if (!prepareTerrainPipeline(renderPass, map->getTexturesIndex())) {
    WARN_ZH("BattlefieldRenderer", "Could not set up terrain rendering");
    return false;
  }

  if (!map->getWater().empty()) {
    if (!prepareWaterVertices()) {
      WARN_ZH("BattlefieldRenderer", "Could not set up water");
      return false;
    }

    if (!prepareWaterPipeline(renderPass)) {
      WARN_ZH("BattlefieldRenderer", "Could not set up water rendering");
      return false;
    }

    hasWater = true;
  }

  if (!modelRenderer.preparePipeline(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not set up model rendering");
    return false;
  }

  if (!preparePatches(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not set up patch rendering");
    return false;
  }
  prepareScorches();

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
  renderPatches(commandBuffer, frameIdx, newMatrices);
  renderObjectInstances(commandBuffer, frameIdx, newMatrices);
  // TODO this still needs better Z ordering
  renderWater(commandBuffer, frameIdx);

  commandBuffer.closeRendering();

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
}

bool BattlefieldRenderer::preparePatches(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  pipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  pipelineSetup.vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;

  pipelineSetup.setVSCode(readFile("shaders/patch.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/patch.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  pipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  patchPipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (patchPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  std::vector<float> data = {
    // vertex          normal             uv
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f
  };

  patchVertices =
    std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  patchVertices->writeData(data, std::vector<uint16_t> {});
  if (!vuglContext.uploadResource(*patchVertices)) {
    return false;
  }

  return true;
}

bool BattlefieldRenderer::prepareScorches() {
  scorchTextureSampler = textureCache.getTextureSampler("exscorch01.dds");
  if (!scorchTextureSampler) {
    return false;
  }
  vuglContext.uploadResource(*scorchTextureSampler);

  return true;
}

bool BattlefieldRenderer::prepareScorchData(const Battlefield::ScorchData& scorch) {
  auto lookup = scorchData.find(scorch.id);
  if (lookup != scorchData.cend()) {
    return true;
  }

  auto entry = scorchData.emplace(std::make_pair(scorch.id, ScorchData {}));
  auto& renderData = entry.first->second;
  renderData.position = scorch.location;
  renderData.radius = scorch.radius;

  glm::vec3 translation {0.0f, 0.0f, 0.0f};
  // upper two rows of 3x3 texture, but 64x64 at size with 32 gap
  auto rest = scorch.type % 3;
  translation.x = rest * (1.0f/4.0f + 1.0/8.0f);

  if (scorch.type >= 3) {
    translation.y = 1.0f/4.0f + 1.0/8.0f;
  }

  renderData.uv =
    glm::translate(glm::mat4 {1.0f}, translation)
      * glm::scale(glm::mat4 {1.0f}, glm::vec3 {1.0f/4.0f, 1.0f/4.0f, 1.0f});
  renderData.descriptorSet =
    std::make_shared<Vugl::DescriptorSet>(patchPipeline->createDescriptorSet());
  renderData.uniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(ScorchUBData)));

  renderData.descriptorSet->assignUniformBuffer(*renderData.uniformBuffer);
  renderData.descriptorSet->assignCombinedSampler(*scorchTextureSampler);

  renderData.descriptorSet->updateDevice();

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

  pipelineSetup.vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  // wireframe:
  // pipelineSetup.vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;

  terrainPipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

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
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

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
      || base->drawMetaData.type == Objects::DrawType::POLICE_CAR_DRAW
      || base->drawMetaData.type == Objects::DrawType::SUPPLY_DRAW
      || base->drawMetaData.type == Objects::DrawType::TANK_DRAW
      || base->drawMetaData.type == Objects::DrawType::TRUCK_DRAW
  ) {
    success = prepareModelDrawData(instance);
  } else if (base->drawMetaData.type == Objects::DrawType::DEFAULT_DRAW) {
    // EVAL Nothing to draw
    return true;
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
    auto& camera = battlefield.getCamera();
    auto& camMatrix = camera.getCameraMatrix();
    drawChecks.clear();
    drawChecks.reserve(battlefield.getObjectInstances().size());

    GFX::Frustum frustrum {camera};

    for (auto& instance : battlefield.getObjectInstances()) {
      if (instance->getBase()->drawMetaData.type == Objects::DrawType::DEFAULT_DRAW) {
        continue;
      }

      modelRenderer.resetFrames(instance->getID());

      auto modelMatrix = battlefield.getWorldMatrix(instance->getPosition(), 0.0f);

      auto& drawCheck = drawChecks.emplace_back();
      drawCheck.instance = instance;
      drawCheck.sphere = boundingSpheres.find(instance->getID())->second;

      // bounding sphere to world
      auto& sphere = drawCheck.sphere;
      drawCheck.sphere.position = glm::vec3 {modelMatrix * glm::vec4 {sphere.position, 1.0f}};
      drawCheck.draw = frustrum.isSphereInside(sphere.position, sphere.radius);
      drawCheck.dist = glm::length(camera.getPosition() - sphere.position);
    }

    std::sort(
        drawChecks.begin()
      , drawChecks.end()
      , [](const DrawCheck& a, const DrawCheck& b) { return a.dist > b.dist; }
    );
  }

  for (auto& dc : drawChecks) {
    if (dc.draw) {
      renderObjectInstance(*dc.instance, commandBuffer, frameIdx);
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
) {

  if (vuglContext.isDebuggingAllowed()) {
    std::string label = std::to_string(instance.getID());
    label.append(": ");
    label.append(instance.getBase()->name);
    commandBuffer.beginDebugLabel(label);
  }

  if (instance.needsRedraw() || modelRenderer.needsUpdate(instance.getID(), frameIdx)) {
    auto worldMatrix =
      battlefield.getWorldMatrix(
          instance.getPosition()
        , instance.getAngle()
      );

    auto normalMatrix =
      glm::rotate(
          glm::mat4 {1.0f}
        , -instance.getAngle()
        , glm::vec3 {0.0f, 1.0f, 0.0f}
      );

    auto& camera = battlefield.getCamera();
    auto mvp =
      camera.getProjectionMatrix()
      * camera.getCameraMatrix()
      * worldMatrix;

    modelRenderer.updateModel(
        instance.getID()
      , frameIdx
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

void BattlefieldRenderer::renderPatches(Vugl::CommandBuffer& commandBuffer, size_t frameIdx, bool newMatrices) {
  TRACY(ZoneScoped);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Scorches");
  }

  for (auto& scorch : battlefield.getScorches()) {
    if (!prepareScorchData(scorch)) {
      continue;
    }
  }

  // TODO consider changes to scorchs set (ptrs)
  scorchOrderData.resize(scorchData.size());

  auto& camera = battlefield.getCamera();
  auto map = battlefield.getMap();

  GFX::Frustum frustrum {camera};

  commandBuffer.bindResource(*patchPipeline);
  commandBuffer.bindResource(*patchVertices);

  if (newMatrices) {
    TRACY(ZoneScoped);
    scorchFrameIdxSet = 0;

    size_t i = 0;
    for (auto& pair : scorchData) {
      auto& scorch = pair.second;
      auto& drawData = scorchOrderData[i];

      auto scale = scorch.radius * 6.0f; // EVAL why so much bigger than drawn
      auto position = glm::vec3 {map->getWorldOffsetMatrix() * glm::vec4 {scorch.position, 1.0f}};

      drawData.scorch = &scorch;
      drawData.draw = frustrum.isSphereInside(position, scale);
      drawData.dist = glm::length(camera.getPosition() - position);

      i += 1;
    }

    std::sort(
        scorchOrderData.begin()
      , scorchOrderData.end()
      , [](const ScorchOrderData& a, const ScorchOrderData& b) { return a.dist > b.dist; }
    );
  }

  float distStep = 0.1f / (scorchOrderData.size());
  size_t i = 0;
  bool needsFrameUpdate = (scorchFrameIdxSet & (1 << frameIdx)) == 0;
  auto numVertices = patchVertices->getNumVertices();
  auto camMatrix = camera.getProjectionMatrix() * camera.getCameraMatrix();

  ScorchUBData ubData;
  ubData.sunlight = sunlightNormal;

  for (auto& orderData : scorchOrderData) {
    TRACY(ZoneScoped);
    auto scorch = orderData.scorch;

    if (!orderData.draw) {
      i += 1;
      continue;
    }

    if (needsFrameUpdate) {
      auto scale = scorch->radius * 2.0f;

      auto worldMatrix = battlefield.getWorldMatrix(scorch->position, 0);
      auto drawTranslation =
        glm::translate(
            glm::mat4 {1.0f}
          , glm::vec3 {-scorch->radius, 0.1f + i * distStep, -scorch->radius}
        );

      auto scaleMatrix =
        glm::scale(
            glm::mat4 {1.0f}
          , glm::vec3 {scale, 1.0f, scale}
        );
      ubData.uv = scorch->uv;
      ubData.mvp =
        camMatrix
        * drawTranslation
        * worldMatrix
        * scaleMatrix;

      scorch->uniformBuffer->writeData(ubData, frameIdx);
    }

    if (vuglContext.isDebuggingAllowed()) {
      std::string label = std::to_string(i);
      commandBuffer.beginDebugLabel(label);
    }

    commandBuffer.bindResource(*scorch->descriptorSet);
    commandBuffer.draw([numVertices](VkCommandBuffer vkCommandBuffer, uint32_t) {
      vkCmdDraw(vkCommandBuffer, 6, 1, 0, 0);

      return VK_SUCCESS;
    });

    if (vuglContext.isDebuggingAllowed()) {
      commandBuffer.endDebugLabel();
    }

    i += 1;
  }

  scorchFrameIdxSet |= (1 << frameIdx);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void BattlefieldRenderer::renderTerrain(Vugl::CommandBuffer& commandBuffer, size_t frameIdx) {
  TRACY(ZoneScoped);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Terrain");
  }

  TerrainScene scene;
  auto& camera = battlefield.getCamera();
  scene.mvp =
    camera.getProjectionMatrix()
    * camera.getCameraMatrix()
    * terrainScaleMatrix;

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
  TRACY(ZoneScoped);

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
    camera.getProjectionMatrix()
    * camera.getCameraMatrix()
    * waterScaleMatrix;
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
