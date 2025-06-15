#include "InstanceFactory.h"

namespace ZH::Objects {

InstanceFactory::InstanceFactory(ObjectLoader& objectLoader) :
  objectLoader(objectLoader)
{}

std::shared_ptr<Instance> InstanceFactory::getInstance(MapObject& mapObject) const {
  Instance instance;

  auto object = objectLoader.getObject(mapObject.name);
  if (!object) {
    return {};
  }

  instance.base = object;
  instance.position = mapObject.location;
  instance.angle = mapObject.angle;

  return std::make_shared<Instance>(std::move(instance));
}

}
