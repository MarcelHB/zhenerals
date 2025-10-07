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
