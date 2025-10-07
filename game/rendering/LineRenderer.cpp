// SPDX-License-Identifier: GPL-2.0

#include "LineRenderer.h"

namespace ZH {

void LineRenderer::Lines::setMatrix(const glm::mat4& mvp) {
  frameIdxSet = 0;
  this->mvp = mvp;
}

void LineRenderer::Lines::writeMatrix(size_t frameIdx) {
  if ((frameIdxSet & (1 << frameIdx)) != 0) {
    return;
  }

  buffer->writeData(mvp, frameIdx);
  frameIdxSet |= (1 << frameIdx);
}

LineRenderer::LineRenderer(Vugl::Context& context) : vuglContext(context) {}

bool LineRenderer::preparePipeline(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};

  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_FALSE;
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  pipelineSetup.vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;

  pipelineSetup.setVSCode(readFile("shaders/lines.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/lines.frag.spv"));

  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);

  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 12, 12, 0);

  pipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  if (pipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

void LineRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  commandBuffer.bindResource(*pipeline);
}

LineRenderer::Lines LineRenderer::createLines(
    const std::vector<glm::vec3>& geometry
  , const std::vector<Color>& colors
) {
  Lines lines;

  if (geometry.size() != colors.size()) {
    lines.broken = true;
    return lines;
  }

  std::vector<glm::vec3> vertexData;
  vertexData.resize(geometry.size() * 2);

  for (size_t i = 0; i < geometry.size(); i++) {
    vertexData[i * 2] = geometry[i];
    glm::vec4 floatColors = colors[i];
    vertexData[i * 2 + 1] = glm::vec3 {floatColors};
  }

  lines.descriptor = std::make_shared<Vugl::DescriptorSet>(pipeline->createDescriptorSet());
  lines.linesAndColors = std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  lines.linesAndColors->writeData(vertexData, std::vector<float> {});

  vuglContext.uploadResource(*lines.linesAndColors);

  lines.buffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(glm::mat4)));

  lines.descriptor->assignUniformBuffer(*lines.buffer);
  lines.descriptor->updateDevice();

  return lines;
}

void LineRenderer::renderLines(Lines& lines, Vugl::CommandBuffer& commandBuffer) {
  if (lines.broken) {
    return;
  }

  commandBuffer.bindResource(*lines.descriptor);
  commandBuffer.bindResource(*lines.linesAndColors);

  auto numVertices = lines.linesAndColors->getNumVertices();
  commandBuffer.draw([numVertices](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdSetLineWidth(vkCommandBuffer, 1.0f);
    vkCmdDraw(vkCommandBuffer, numVertices, 1, 0, 0);
    return VK_SUCCESS;
  });
}

}
