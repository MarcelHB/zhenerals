#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "BattlefieldRenderer.h"

namespace ZH {

struct TerrainScene {
  alignas(16) glm::mat4 mvp;
  alignas(16) glm::vec3 sunLight;
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

std::shared_ptr<Vugl::CommandBuffer> BattlefieldRenderer::createRenderList(size_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  if (!terrainVertices && !prepareTerrainVertices()) {
    WARN_ZH("BattlefieldRenderer", "Could not set up terrain");
    return {};
  }

  auto map = battlefield.getMap();
  if (!terrainPipeline && !prepareTerrainPipeline(renderPass, map->getTexturesIndex())) {
    WARN_ZH("BattlefieldRenderer", "Could not setup up terrain rendering");
    return {};
  }

  if (!map->getWater().empty() && !waterSetupAttempted) {
    waterSetupAttempted = true;
    if (!waterVertices && !prepareWaterVertices()) {
      WARN_ZH("BattlefieldRenderer", "Could not set up water");
      return {};
    }

    if (!waterPipeline && !prepareWaterPipeline(renderPass)) {
      WARN_ZH("BattlefieldRenderer", "Could not setup up water rendering");
      return {};
    }

    hasWater = true;
  }

  auto commandBuffer = vuglContext.createCommandBuffer(frameIdx, true);

  auto vp = vuglContext.getViewport();
  auto projectMatrix =
    glm::perspective(
        glm::radians(90.0f)
      , vp.width / static_cast<float>(vp.height)
      , 0.1f
      , 1000.0f
    );

  std::array<VkClearValue, 2> clearColors{};
  clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearColors[1].depthStencil = {1.0f, 0};
  commandBuffer.beginRendering(renderPass, clearColors);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Terrain");
  }

  auto size = map->getSize();
  // Terrain
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

  TerrainScene scene;
  scene.mvp =
    projectMatrix * battlefield.getCameraMatrix();
  scene.sunLight = glm::normalize(lightTarget - lightPos);
  terrainUniformBuffer->writeData(scene, frameIdx);

  commandBuffer.bindResource(*terrainPipeline);
  commandBuffer.bindResource(*terrainDescriptorSet);
  commandBuffer.bindResource(*terrainVertices);

  auto numIndices = map->getVertexIndices().size();
  commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);
    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }

  if (hasWater) {
    if (vuglContext.isDebuggingAllowed()) {
      commandBuffer.beginDebugLabel("Water");
    }

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

  commandBuffer.closeRendering();

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
}

bool BattlefieldRenderer::prepareModelPipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.setVSCode(readFile("shaders/model.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/model.frag.spv"));

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);

  modelPipline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (modelPipline->getLastResult() != VK_SUCCESS) {
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

}
