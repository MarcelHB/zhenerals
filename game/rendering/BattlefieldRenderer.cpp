// SPDX-License-Identifier: GPL-2.0

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "../MurmurHash.h"
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

BattlefieldRenderer::BattlefieldRenderer(
    Vugl::Context& vuglContext
  , const Config& config
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
  , std::shared_ptr<ResourceLoader> iniResourceLoader
) : vuglContext(vuglContext)
  , config(config)
  , textureCache(textureCache)
  , battlefield(battlefield)
  , instanceRenderer {vuglContext, config, textureCache, modelCache}
  , iniResourceLoader {iniResourceLoader}
{}

bool BattlefieldRenderer::init(Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  {
    auto terrainIniStream = iniResourceLoader->getFileStream("data\\ini\\terrain.ini");
    if (terrainIniStream) {
      auto stream = terrainIniStream->getStream();
      TerrainINI terrainINI {stream};
      terrains = terrainINI.parse();
    } else {
      return false;
    }
  }

  {
    auto waterIniStream = iniResourceLoader->getFileStream("data\\ini\\water.ini");
    if (waterIniStream) {
      auto stream = waterIniStream->getStream();
      WaterINI waterINI {stream};
      waterSettings = waterINI.parse();
    } else {
      return false;
    }
  }

  {
    auto roadsBridgesStream = iniResourceLoader->getFileStream("data\\ini\\roads.ini");
    if (roadsBridgesStream) {
      auto stream = roadsBridgesStream->getStream();
      RoadsBridgesINI roadsBridgesINI {stream};
      auto result = roadsBridgesINI.parse();
      bridges = std::move(result.bridges);
      roads = std::move(result.roads);
    }
  }

  auto vp = vuglContext.getViewport();
  battlefield.setPerspectiveProjection(
      10.0f
    , 10000.0f
    , 60.0f
    , vp.width * 1.0f
    , vp.height * 1.0f
  );

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

  if (!instanceRenderer.preparePipeline(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not set up instance rendering");
    return false;
  }

  patchRenderer =
    std::make_shared<PatchRenderer>(
        vuglContext
      , config
      , battlefield
      , textureCache
      , roads
    );
  if (!patchRenderer->init(renderPass)) {
    WARN_ZH("BattlefieldRenderer", "Could not set up patch rendering");
    return false;
  }

  return true;
}

void BattlefieldRenderer::createRenderList(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  auto newMatrices = battlefield.cameraHasMoved();

  std::array<VkClearValue, 2> clearColors{};
  clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearColors[1].depthStencil = {1.0f, 0};
  commandBuffer.beginRendering(renderPass, clearColors);

  renderTerrain(commandBuffer, frameIdx);
  patchRenderer->renderPatches(commandBuffer, frameIdx, newMatrices);
  renderObjectInstances(commandBuffer, frameIdx, newMatrices);
  // TODO this still needs better Z ordering
  renderWater(commandBuffer, frameIdx);

  commandBuffer.closeRendering();
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
    MurmurHash3_32 hasher;
    hasher.feed(keyName);

    auto terrainLookup = terrains.find(hasher.getHash().value);
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

void BattlefieldRenderer::renderObjectInstances(
    Vugl::CommandBuffer& commandBuffer
  , uint32_t frameIdx
  , bool newMatrices
) {
  TRACY(ZoneScoped);

  instanceRenderer.beginResourceCounting();

  for (auto& instance : battlefield.getObjectInstances()) {
    if (!instanceRenderer.prepareInstance(*instance)) {
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

  instanceRenderer.bindPipeline(commandBuffer);

  // TODO instance movement, check for new/deleted instances
  if (newMatrices) {
    auto& camera = battlefield.getCamera();
    auto& camMatrix = camera.getCameraMatrix();
    drawChecks.clear();
    drawChecks.reserve(battlefield.getObjectInstances().size());

    GFX::Frustum frustrum {camera};

    for (auto& instance : battlefield.getObjectInstances()) {
      instanceRenderer.resetFrames(*instance);

      auto modelMatrix = battlefield.getWorldMatrix(instance->getPosition(), 0.0f);

      auto& drawCheck = drawChecks.emplace_back();
      drawCheck.instance = instance;
      drawCheck.sphere = instanceRenderer.getBoundingSphere(*instance);

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

  instanceRenderer.finishResourceCounting();
}

void BattlefieldRenderer::renderObjectInstance(
    Objects::Instance& instance
  , Vugl::CommandBuffer& commandBuffer
  , uint32_t frameIdx
) {
  TRACY(ZoneScoped);
  TRACY(ZoneText(instance.getBase()->name.c_str(), instance.getBase()->name.size()));

  if (vuglContext.isDebuggingAllowed()) {
    auto label = fmt::format("{}: {}", instance.getID(), instance.getBase()->name);
    commandBuffer.beginDebugLabel(label);
  }

  if (instance.needsRedraw() || instanceRenderer.needsUpdate(instance, frameIdx)) {
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

    instanceRenderer.updateInstance(
        instance
      , frameIdx
      , mvp
      , camera.getPosition()
      , normalMatrix
      , battlefield.getSunlightNormal()
    );

    instance.setRedrawn();
  }

  instanceRenderer.renderInstance(instance, commandBuffer);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void BattlefieldRenderer::renderTerrain(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx) {
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

  scene.sunlight = battlefield.getSunlightNormal();
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

void BattlefieldRenderer::renderWater(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx) {
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
