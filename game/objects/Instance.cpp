// SPDX-License-Identifier: GPL-2.0

#include "Instance.h"

namespace ZH::Objects {

uint64_t Instance::nextID = 0;

float Instance::getAngle() const {
  return angle;
}

std::shared_ptr<const ObjectBuilder> Instance::getBase() const {
  return base;
}

const std::set<Objects::ModelCondition>& Instance::getCurrentConditions() const {
  // TODO: outer circumstances (daytime, weather)
  if (conditionsExamined) {
    return currentConditions;
  }

  currentConditions.clear();

  // TODO: thresholds actually defined by gamedata.ini
  if (health >= 35.0f && health < 70.0f) {
    currentConditions.emplace(Objects::ModelCondition::DAMAGED);
  } else if (health > 0.0f && health < 35.0f) {
    currentConditions.emplace(Objects::ModelCondition::REALLY_DAMAGED);
  } else if (health == 0.0f) {
    currentConditions.emplace(Objects::ModelCondition::RUBBLE);
  }

  conditionsExamined = true;
  return currentConditions;
}

uint64_t Instance::getID() const {
  return id;
}

const glm::vec3& Instance::getPosition() const {
  return position;
}

bool Instance::needsRedraw() const {
  return drawUpdate;
}

void Instance::setRedrawn() {
  drawUpdate = false;
}

}
