// SPDX-License-Identifier: GPL-2.0

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "BattlefieldRenderer.h"
#include "gfx/Frustum.h"
#include "MurmurHash.h"
#include "PatchRenderer.h"

namespace ZH {

constexpr float ROAD_HEIGHT_OFFSET = 0.9f;
constexpr float SCORCH_HEIGHT_OFFSET = 1.0f;
constexpr float ROAD_STRETCH = 3.0f;

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
  , roadResolution(battlefield)
  , roadsINI(roads)
{}

bool PatchRenderer::init(Vugl::RenderPass& renderPass) {
  if (!preparePatches(renderPass)) { return false; }
  if (!prepareScorches()) { return false; }
  if (!prepareRoads()) { return false; }

  return true;
}

void PatchRenderer::beginResourceCounting() {
  for (auto& road : roadData) {
    road.increaseMiss();
  }
  for (auto& pair : scorchData) {
    pair.second.increaseMiss();
  }
}

void PatchRenderer::finishResourceCounting() {
  for (auto it = roadData.begin(); it != roadData.end();) {
    if (it->getMisses() >= config.refreshRate.value_or(60) * 60) {
      it = roadData.erase(it);
    } else {
      it++;
    }
  }

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

bool PatchRenderer::prepareRoads() {
  auto& roads = battlefield.getRoads();
  std::list<RoadResolution::RoadElement> elements;

  uint32_t maxPreparedRoadSystem = 0;
  for (auto& roadNode : roads) {
    if (roadNode.roadSystem > maxPreparedRoadSystem) {
      maxPreparedRoadSystem = roadNode.roadSystem;
      auto subList = roadResolution.resolveRoadSystem(roadNode);
      elements.insert(elements.cend(), subList.begin(), subList.end());
    }
  }

  std::vector<float> data = {
     // vertex            normal             uv             compat
     -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,    0, 0, 0.0f, 0.0f, 0.0f
   ,  0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,    0, 0, 0.0f, 0.0f, 0.0f
   ,  0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,    0, 0, 0.0f, 0.0f, 0.0f

   , -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,    0, 0, 0.0f, 0.0f, 0.0f
   ,  0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,    0, 0, 0.0f, 0.0f, 0.0f
   , -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,    0, 0, 0.0f, 0.0f, 0.0f
  };

  roadDefaultVertices =
    std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  roadDefaultVertices->writeData(data, std::vector<uint16_t> {});
  vuglContext.uploadResource(*roadDefaultVertices);

  roadData.reserve(elements.size());

  for (auto& element : elements) {
    MurmurHash3_32 typeHasher;
    typeHasher.feed(element.roadType);
    auto hash = typeHasher.getHash();

    auto typeLookup = roadsINI.find(hash);
    if (typeLookup == roadsINI.cend()) {
      return false;
    }

    MurmurHash3_32 textureHasher;
    textureHasher.feed(typeLookup->second.texture);
    hash = textureHasher.getHash();

    auto textureLookup = roadTextures.find(hash);
    if (textureLookup == roadTextures.cend()) {
      auto sampler = textureCache.getTextureSampler(typeLookup->second.texture);
      if (!sampler) {
        return false;
      }

      textureLookup = roadTextures.emplace(hash, std::move(sampler)).first;
    }

    // Some are 256x256, some 512x512
    auto extent = textureLookup->second->getExtent();
    auto cut =
      RoadResolution::TEXTURE_CUTS[
        static_cast<std::underlying_type_t<RoadResolution::RoadElementType>>(element.type)
      ];
    auto stretchCorrection = RoadResolution::T_SIZE / extent.width;

    auto& gfxElement = roadData.emplace_back();
    gfxElement.textureKey = hash;
    gfxElement.zIndex = typeLookup->second.zIndex;

    gfxElement.mvp =
      glm::translate(glm::mat4 {1.0f}, element.location)
         * glm::rotate(glm::mat4 {1.0f}, element.floorRotation, glm::vec3 {0.0f, 1.0f, 0.0f})
         * glm::scale(
            glm::mat4 {1.0f}
          , glm::vec3 {
                element.stretch * RoadResolution::T_SIZE
              , 1
              , typeLookup->second.width * typeLookup->second.widthInTexture
            }
          );
    gfxElement.mvp = battlefield.getMap()->getWorldOffsetMatrix() * gfxElement.mvp;

    gfxElement.uv =
      glm::translate(
          glm::mat4 {1.0f}
        , glm::vec3 {
            element.offset + cut.first[0]
          , element.offset + cut.first[1]
          , 0.0f
        }
      )
        * glm::scale(
            glm::mat4 {1.0f}
          , glm::vec3 {
              cut.second[0] * element.stretch * ROAD_STRETCH * stretchCorrection
            , cut.second[1]
            , 0.0f
          }
        );
  }

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

  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 32, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_UINT, 36, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32_SFLOAT, 40, 4, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 44, 8, 0);

  patchPipeline =
    std::make_shared<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));

  return patchPipeline->getLastResult() == VK_SUCCESS;
}

void PatchRenderer::prepareRoadData(RoadData& roadData) {
  if (roadData.prepared) {
    return;
  }

  roadData.descriptorSet =
    std::make_shared<Vugl::DescriptorSet>(patchPipeline->createDescriptorSet());
  roadData.uniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(ScorchUBData)));

  auto lookup = roadTextures.find(roadData.textureKey);
  auto& sampler = lookup->second;

  roadData.descriptorSet->assignUniformBuffer(*roadData.uniformBuffer);
  roadData.descriptorSet->assignCombinedSampler(*sampler);

  vuglContext.uploadResource(*sampler);
  roadData.descriptorSet->updateDevice();

  roadData.prepared = true;
}

void PatchRenderer::prepareScorchData(const Battlefield::ScorchData& scorch) {
  auto lookup = scorchData.find(scorch.id);
  if (lookup != scorchData.cend()) {
    return;
  }

  auto entry = scorchData.emplace(std::make_pair(scorch.id, ScorchData {}));
  auto& renderData = entry.first->second;
  renderData.position = scorch.location;
  // data has it at 0
  renderData.position.y = battlefield.getWorldHeight(scorch.location);
  renderData.radius = scorch.radius;

  auto map = battlefield.getMap();
  glm::vec4 location = {scorch.location.x, 0.0f, scorch.location.z, 1.0f};

  auto& offsetMap = map->getWorldOffsetMatrix();
  location = offsetMap * location;

  IntFlatBox sliceRequest;
  sliceRequest.position = {
      static_cast<int32_t>(std::floor(location.x - scorch.radius) * 0.1f)
    , static_cast<int32_t>(std::floor(location.z - scorch.radius) * 0.1f)
  };
  sliceRequest.size = {
      static_cast<uint32_t>((std::ceil(location.x + scorch.radius) - std::floor(location.x - scorch.radius)) * 0.1f)
    , static_cast<uint32_t>((std::ceil(location.z + scorch.radius) - std::floor(location.z - scorch.radius)) * 0.1f)
  };

  auto vertices = map->getVertexSlice(sliceRequest);

  renderData.vertices =
    std::make_shared<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  renderData.vertices->setBigIndexBuffer(true);

  glm::vec3 translation {0.0f, 0.0f, 0.0f};
  // upper two rows of 3x3 texture, but 64x64 at size with 32 gap
  auto rest = scorch.type % 3;
  translation.x = rest * (1.0f/4.0f + 1.0/8.0f);

  if (scorch.type >= 3) {
    translation.y = 1.0f/4.0f + 1.0/8.0f;
  }

  float factor = 1.0f / sliceRequest.size.x;
  size_t numCells = sliceRequest.size.x * sliceRequest.size.y;

  for (size_t i = 0; i < numCells; ++i) {
    auto xCell = i % sliceRequest.size.x;
    auto yCell = i / sliceRequest.size.x;

    vertices.second[i * 4    ].uv = { xCell * factor, yCell * factor};
    vertices.second[i * 4 + 1].uv = { (xCell + 1) * factor, yCell * factor};
    vertices.second[i * 4 + 2].uv = { xCell * factor, (yCell + 1) * factor};
    vertices.second[i * 4 + 3].uv = { (xCell + 1) * factor, (yCell + 1) * factor};
  }

  renderData.vertices->writeData(vertices.second, vertices.first);

  renderData.uv =
    glm::translate(glm::mat4 {1.0f}, translation)
      * glm::scale(glm::mat4 {1.0f}, glm::vec3 {1.0f/4.0f, 1.0f/4.0f, 1.0f});
  renderData.descriptorSet =
    std::make_shared<Vugl::DescriptorSet>(patchPipeline->createDescriptorSet());
  renderData.uniformBuffer =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(ScorchUBData)));

  renderData.descriptorSet->assignUniformBuffer(*renderData.uniformBuffer);
  renderData.descriptorSet->assignCombinedSampler(*scorchTextureSampler);

  vuglContext.uploadResource(*renderData.vertices);
  renderData.descriptorSet->updateDevice();
}

void PatchRenderer::renderPatches(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx, bool newMatrices) {
  TRACY(ZoneScoped);

  renderRoads(commandBuffer, frameIdx, newMatrices);
  renderScorches(commandBuffer, frameIdx, newMatrices);
}

void PatchRenderer::renderRoads(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx, bool newMatrices) {
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Roads");
  }

  auto totalRoadTypes = roadsINI.size();
  ScorchUBData ubData;
  ubData.sunlight = battlefield.getSunlightNormal();

  auto& camera = battlefield.getCamera();
  auto camMatrix = camera.getProjectionMatrix() * camera.getCameraMatrix();
  auto heightCorrectionMatrix =
    glm::scale(
        glm::mat4 {1.0f}
      , glm::vec3 {1.0f, Map::TERRAIN_HEIGHT_SCALE, 1.0f}
    );

  commandBuffer.bindResource(*patchPipeline);
  commandBuffer.bindResource(*roadDefaultVertices);

  for (auto& road : roadData) {
    prepareRoadData(road);

    if (newMatrices) {
      road.frameIdxSet = 0;
    }

    road.decreaseMiss();

    bool needsFrameUpdate = (road.frameIdxSet & (1 << frameIdx)) == 0;
    if (needsFrameUpdate) {
      ubData.heightOffset = ROAD_HEIGHT_OFFSET - 0.5f * (road.zIndex * 1.0f / totalRoadTypes);
      ubData.uv = road.uv;
      ubData.mvp =
        camMatrix
        * heightCorrectionMatrix
        * road.mvp;

      road.uniformBuffer->writeData(ubData, frameIdx);
      road.frameIdxSet |= (1 << frameIdx);
    }

    commandBuffer.bindResource(*road.descriptorSet);
    commandBuffer.draw([](VkCommandBuffer vkCommandBuffer, uint32_t) {
      vkCmdDraw(vkCommandBuffer, 6, 1, 0, 0);

      return VK_SUCCESS;
    });
  }

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void PatchRenderer::renderScorches(Vugl::CommandBuffer& commandBuffer, uint32_t frameIdx, bool newMatrices) {
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel("Scorches");
  }

  for (auto& scorch : battlefield.getScorches()) {
    prepareScorchData(scorch);
  }

  // TODO consider changes to scorchs set (ptrs)
  scorchOrderData.resize(scorchData.size());

  auto& camera = battlefield.getCamera();
  auto map = battlefield.getMap();

  GFX::Frustum frustrum {camera};

  commandBuffer.bindResource(*patchPipeline);

  if (newMatrices) {
    TRACY(ZoneScoped);

    auto& offsetMatrix = map->getWorldOffsetMatrix();

    // TODO if there are big height differences in the patch, this is inaccurate
    size_t i = 0;
    for (auto& pair : scorchData) {
      auto& scorch = pair.second;
      auto& drawData = scorchOrderData[i];

      Sphere sphere = {
          glm::vec3 {offsetMatrix * glm::vec4 {scorch.position, 1.0f}}
        , std::sqrt(2 * scorch.radius * scorch.radius)
      };

      drawData.scorch = &scorch;
      drawData.draw = frustrum.isSphereInside(sphere);
      drawData.dist = glm::length(camera.getPosition() - sphere.position);
      drawData.frameIdxSet = 0;

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
  auto camMatrix = camera.getProjectionMatrix() * camera.getCameraMatrix();

  ScorchUBData ubData;
  ubData.sunlight = battlefield.getSunlightNormal();
  ubData.heightOffset = SCORCH_HEIGHT_OFFSET;

  for (auto& orderData : scorchOrderData) {
    TRACY(ZoneScoped);
    auto scorch = orderData.scorch;

    if (!orderData.draw) {
      i += 1;
      continue;
    }

    scorch->decreaseMiss();

    bool needsFrameUpdate = (orderData.frameIdxSet & (1 << frameIdx)) == 0;
    if (needsFrameUpdate) {
      ubData.uv = scorch->uv;
      ubData.mvp =
        camMatrix
        * BattlefieldRenderer::getTerrainScaleMatrix();

      scorch->uniformBuffer->writeData(ubData, frameIdx);
      orderData.frameIdxSet |= (1 << frameIdx);
    }

    if (vuglContext.isDebuggingAllowed()) {
      std::string label = std::to_string(i);
      commandBuffer.beginDebugLabel(label);
    }

    commandBuffer.bindResource(*scorch->vertices);
    commandBuffer.bindResource(*scorch->descriptorSet);
    auto numIndices = scorch->vertices->getNumIndices();
    commandBuffer.draw([numIndices](VkCommandBuffer vkCommandBuffer, uint32_t) {
      vkCmdDrawIndexed(vkCommandBuffer, numIndices, 1, 0, 0, 0);

      return VK_SUCCESS;
    });

    if (vuglContext.isDebuggingAllowed()) {
      commandBuffer.endDebugLabel();
    }

    i += 1;
  }


  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

}
