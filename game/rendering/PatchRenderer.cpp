// SPDX-License-Identifier: GPL-2.0

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "../gfx/Frustum.h"
#include "PatchRenderer.h"

namespace ZH {

PatchRenderer::PatchRenderer(
    Vugl::Context& vuglContext
  , const Config& config
  , Battlefield& battlefield
  , GFX::TextureCache& textureCache
  , const RoadsBridgesINI::Roads& roads
) : vuglContext(vuglContext)
  , config(config)
  , textureCache(textureCache)
  , battlefield(battlefield)
  , roads(roads)
{}

bool PatchRenderer::init(Vugl::RenderPass& renderPass) {
  if (!prepareScorches()) {
    return false;
  }

  if (!preparePatches(renderPass)) {
    return false;
  }

  return true;
}

void PatchRenderer::beginResourceCounting() {
  for (auto& pair : scorchData) {
    pair.second.increaseMiss();
  }
}

void PatchRenderer::finishResourceCounting() {
  for (auto it = scorchData.begin(); it != scorchData.end();) {
    if (it->second.getMisses() >= config.refreshRate.value_or(60) * 60) {
      it = scorchData.erase(it);
    } else {
      it++;
    }
  }
}

bool PatchRenderer::prepareScorches() {
  scorchTextureSampler = textureCache.getTextureSampler("exscorch01.dds");
  if (!scorchTextureSampler) {
    return false;
  }
  vuglContext.uploadResource(*scorchTextureSampler);

  return true;
}

bool PatchRenderer::preparePatches(Vugl::RenderPass& renderPass) {
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

bool PatchRenderer::prepareScorchData(const Battlefield::ScorchData& scorch) {
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

void PatchRenderer::renderPatches(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx, bool newMatrices) {
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
  ubData.sunlight = battlefield.getSunlightNormal();

  for (auto& orderData : scorchOrderData) {
    TRACY(ZoneScoped);
    auto scorch = orderData.scorch;

    if (!orderData.draw) {
      i += 1;
      continue;
    }

    scorch->decreaseMiss();
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

}
