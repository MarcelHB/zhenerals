#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "MapRenderer.h"

namespace ZH {

struct TerrainScene {
  alignas(16) glm::mat4 mvp;
  alignas(16) glm::vec3 sunLight;
};

MapRenderer::MapRenderer(
    Vugl::Context& vuglContext
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
  , const TerrainINI::Terrains& terrains
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , battlefield(battlefield)
  , terrains(terrains)
{}

std::shared_ptr<Vugl::CommandBuffer> MapRenderer::createRenderList(size_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  if (!terrainVertices && !prepareTerrainVertices()) {
    WARN_ZH("MapRenderer", "Could not set up terrain");
    return {};
  }

  auto map = battlefield.getMap();
  if (!terrainPipeline && !prepareTerrainPipeline(renderPass, map->getTexturesIndex())) {
    WARN_ZH("MapRenderer", "Could not setup up terrain rendering");
    return {};
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
  auto numVertices = map->getVertexData().size();

  commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);
    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }

  commandBuffer.closeRendering();

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
}

bool MapRenderer::prepareTerrainPipeline(
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
      WARN_ZH("MapRenderer", "Terrain not found");
      continue;
    }

    auto texture = terrainTextures.emplace_back(textureCache.getTexture(terrainLookup->second.textureName));
    if (!texture) {
      WARN_ZH("MapRenderer", "Terrain texture not found");
      continue;
    }
    terrainDescriptorSet->assignTexture(*texture, 2);
    vuglContext.uploadResource(*texture);
  }

  cloudTexture = textureCache.getTexture("tscloudmed.dds");
  terrainDescriptorSet->assignTexture(*cloudTexture);
  vuglContext.uploadResource(*cloudTexture);

  terrainDescriptorSet->updateDevice();

  return true;
}

bool MapRenderer::prepareTerrainVertices() {
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

}
