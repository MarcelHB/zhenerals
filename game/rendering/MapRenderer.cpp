#include <glm/gtc/matrix_transform.hpp>

#include "../Logging.h"
#include "MapRenderer.h"

namespace ZH {

struct TerrainScene {
  alignas(16) glm::mat4 mvp;
};

MapRenderer::MapRenderer(
    Vugl::Context& vuglContext
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , battlefield(battlefield)
{}

std::shared_ptr<Vugl::CommandBuffer> MapRenderer::createRenderList(size_t frameIdx, Vugl::RenderPass& renderPass) {
  TRACY(ZoneScoped);

  if (!terrainVertices && !prepareTerrainVertices()) {
    WARN_ZH("MapRenderer", "Could not set up terrain");
    return {};
  }

  if (!terrainPipeline && !prepareTerrainPipeline(renderPass)) {
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

  // Terrain
  auto terrainMvp =
    projectMatrix * battlefield.getCameraMatrix();
  terrainUniformBuffer->writeData(terrainMvp, frameIdx);

  commandBuffer.bindResource(*terrainPipeline);
  commandBuffer.bindResource(*terrainDescriptorSet);
  commandBuffer.bindResource(*terrainVertices);

  auto map = battlefield.getMap();
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

bool MapRenderer::prepareTerrainPipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
  pipelineSetup.vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
  pipelineSetup.setVSCode(readFile("shaders/terrain.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/terrain.frag.spv"));
  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 24, 8, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 32, 4, 0);

  terrainPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (terrainPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  terrainUniformBuffer =
    std::make_unique<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(TerrainScene)));
  terrainDescriptorSet =
    std::make_unique<Vugl::DescriptorSet>(terrainPipeline->createDescriptorSet());
  terrainDescriptorSet->assignUniformBuffer(*terrainUniformBuffer);
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
