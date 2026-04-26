// SPDX-License-Identifier: GPL-2.0

#include <algorithm>

#include "InstanceRenderer.h"

namespace ZH {

InstanceRenderer::InstanceRenderer(
    Vugl::Context& vuglContext
  , const Config& config
  , GFX::TextureCache& textureCache
  , GFX::ModelCache& modelCache
) : modelRenderer {vuglContext, config, textureCache, modelCache}
{}

void InstanceRenderer::beginResourceCounting() {
  modelRenderer.beginResourceCounting();
}

void InstanceRenderer::finishResourceCounting() {
  modelRenderer.beginResourceCounting();
}

ModelRenderer::BoundingSphere InstanceRenderer::getBoundingSphere(const Objects::Instance& instance) const {
  auto lookup = drawData.find(instance.getID());
  if (lookup == drawData.cend()) {
    return {};
  }

  return lookup->second.boundingSphere;
};

bool InstanceRenderer::prepareInstance(const Objects::Instance& instance) {
  TRACY(ZoneScoped);

  auto base = instance.getBase();
  auto id = instance.getID();
  auto lookup = drawData.find(id);
  if (lookup != drawData.cend()) {
    return true;
  }

  auto& newData = drawData[id];

  bool success = true;
  for (auto& drawMetaData : base->drawMetaData) {
    // Nothing to draw
    if (drawMetaData.type == Objects::DrawType::DEFAULT_DRAW) {
      continue;
    }

    if (drawMetaData.type == Objects::DrawType::DEPENDENCY_MODEL_DRAW
        || drawMetaData.type == Objects::DrawType::MODEL_DRAW
        || drawMetaData.type == Objects::DrawType::OVERLORD_AIRCRAFT_DRAW
        || drawMetaData.type == Objects::DrawType::POLICE_CAR_DRAW
        || drawMetaData.type == Objects::DrawType::SUPPLY_DRAW
        || drawMetaData.type == Objects::DrawType::TANK_DRAW
        || drawMetaData.type == Objects::DrawType::TRUCK_DRAW
    ) {
      success &= prepareModelDrawData(instance, drawMetaData.drawData, newData);
    } else if (drawMetaData.type == Objects::DrawType::TREE_DRAW) {
      success &= prepareTreeDrawData(instance, drawMetaData.drawData, newData);
    } else {
      WARN_ZH(
          "InstanceRenderer"
        , "Skipping drawing of {}, draw mode not implemented"
        , instance.getBase()->name
      );
      success = false;
    }
  }

  // workaround until we know when to combine bounding spheres
  if (!newData.currentDrawStates.empty()) {
    newData.boundingSphere = modelRenderer.getBoundingSphere(newData.currentDrawStates.front().modelID);
  }

  return success;
}

void InstanceRenderer::bindPipeline(Vugl::CommandBuffer& commandBuffer) {
  return modelRenderer.bindPipeline(commandBuffer);
}

bool InstanceRenderer::preparePipeline(Vugl::RenderPass& renderPass) {
  return modelRenderer.preparePipeline(renderPass);
}

bool InstanceRenderer::prepareModelDrawData(
    const Objects::Instance& instance
  , const std::shared_ptr<const Objects::DrawData>& instanceDrawSpec
  , InstanceData& instanceDrawState
) {
  auto base = instance.getBase();
  auto modelSpec = static_pointer_cast<const Objects::ModelDrawData>(instanceDrawSpec);

  // EVAL condition states
  // There are empty blocks around (ChinaAirfield)
  if (modelSpec->defaultConditionState.model.empty()
      && modelSpec->conditionStates.empty()) {
    return true;
  }

  std::string modelName;
  if (!modelSpec->defaultConditionState.model.empty()) {
    modelName = modelSpec->defaultConditionState.model;
  }

  auto& drawState = instanceDrawState.currentDrawStates.emplace_back();

  determineModel(instance, modelSpec, drawState);
  if (drawState.hidden) {
    return true;
  }

  if (!modelRenderer.prepareModel(drawState.modelID, drawState.modelName)) {
    WARN_ZH("BattlefieldRenderer", "Unable to find model {} for {}", drawState.modelName, base->name);
    return false;
  }

  return true;
}

void InstanceRenderer::determineModel(
    const Objects::Instance& instance
  , const std::shared_ptr<const Objects::ModelDrawData>& modelDrawSpec
  , InstanceData::DrawState& drawState
) {
  if (drawState.modelID == 0) {
    drawState.modelID = nextModelID++;
  }

  drawState.hidden = false;

  auto& instanceConditions = instance.getCurrentConditions();

  // No conditions without default state -> nothing r/n
  if (instanceConditions.empty() && modelDrawSpec->defaultConditionState.model.empty()) {
    drawState.hidden = true;
    return;
  }

  // EVAL aliases
  // Look for what has the biggest intersection
  auto bestIt = modelDrawSpec->conditionStates.cend();
  size_t numCommon = 0;
  for (auto it = modelDrawSpec->conditionStates.cbegin(); it != modelDrawSpec->conditionStates.cend(); ++it) {
    std::set<Objects::ModelCondition> intersection;
    std::set_intersection(
        instanceConditions.cbegin(), instanceConditions.cend()
      , it->conditions.cbegin(), it->conditions.cend()
      , std::inserter(intersection, intersection.begin())
    );

    if (intersection.size() > numCommon) {
      numCommon = intersection.size();
      bestIt = it;
    }
  }

  if (bestIt != modelDrawSpec->conditionStates.cend()) {
    drawState.modelName = bestIt->model;
    drawState.applicableConditions = bestIt->conditions;
    return;
  }

  // fallback
  if (modelDrawSpec->defaultConditionState.model.empty()) {
    drawState.hidden = true;
    return;
  }

  drawState.modelName = modelDrawSpec->defaultConditionState.model;
}

bool InstanceRenderer::prepareTreeDrawData(
    const Objects::Instance& instance
  , const std::shared_ptr<const Objects::DrawData>& instanceDrawSpec
  , InstanceData& instanceDrawState
) {
  auto treeSpec = static_pointer_cast<const Objects::TreeDrawData>(instanceDrawSpec);

  auto& drawState = instanceDrawState.currentDrawStates.emplace_back();
  drawState.modelID = nextModelID++;
  drawState.applicableConditions = {Objects::ModelCondition::ALL};

  if (!modelRenderer.prepareModel(drawState.modelID, treeSpec->model)) {
    WARN_ZH("BattlefieldRenderer", "Unable to find models for {}", instance.getBase()->name);
    return false;
  }

  return true;
}

bool InstanceRenderer::needsUpdate(const Objects::Instance& instance, size_t frameIdx) const {
  auto lookup = drawData.find(instance.getID());
  if (lookup == drawData.cend()) {
    return false;
  }

  auto& renderData = lookup->second;

  return (renderData.frameIdxSet & (1 << frameIdx)) == 0;
}

void InstanceRenderer::updateInstance(
    const Objects::Instance& instance
  , size_t frameIdx
  , const glm::mat4& mvp
  , const glm::vec3& cameraPos
  , const glm::mat4& normal
  , const glm::vec3& sunlightNormal
)  {
  auto lookup = drawData.find(instance.getID());
  if (lookup == drawData.cend()) {
    return;
  }

  auto& drawStates = lookup->second.currentDrawStates;
  for (auto& drawState : drawStates) {
    modelRenderer.updateModel(
        drawState.modelID
      , frameIdx
      , mvp
      , cameraPos
      , normal
      , sunlightNormal
    );
  }

  lookup->second.frameIdxSet |= (1 << frameIdx);
}

void InstanceRenderer::resetFrames(const Objects::Instance& instance) {
  auto lookup = drawData.find(instance.getID());
  if (lookup == drawData.cend()) {
    return;
  }

  lookup->second.frameIdxSet = 0;
}

bool InstanceRenderer::renderInstance(
    const Objects::Instance& instance
  , Vugl::CommandBuffer& commandBuffer
)  {
  TRACY(ZoneScoped);

  auto lookup = drawData.find(instance.getID());
  if (lookup == drawData.cend()) {
    return false;
  }

  bool success = true;
  auto& drawStates = lookup->second.currentDrawStates;
  for (auto& drawState : drawStates) {
    success &= modelRenderer.renderModel(drawState.modelID, commandBuffer);
  }

  return success;
}

}
