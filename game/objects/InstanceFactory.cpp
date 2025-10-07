// SPDX-License-Identifier: GPL-2.0

#include "InstanceFactory.h"

namespace ZH::Objects {

InstanceFactory::InstanceFactory(ObjectLoader& objectLoader) :
  objectLoader(objectLoader)
{}

std::shared_ptr<Instance> InstanceFactory::getInstance(const MapObject& mapObject) const {
  Instance instance;

  auto object = objectLoader.getObject(mapObject.name);
  if (!object) {
    return {};
  }

  instance.base = object;
  instance.position = mapObject.location;
  instance.angle = mapObject.angle;
  instance.id = Instance::nextID++;

  return std::make_shared<Instance>(std::move(instance));
}

}
