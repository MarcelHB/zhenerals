// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_OBJECTS_INSTANCE_FACTORY
#define H_GAME_OBJECTS_INSTANCE_FACTORY

#include "../common.h"
#include "Instance.h"
#include "../Map.h"
#include "../ObjectLoader.h"

namespace ZH::Objects {

class InstanceFactory {
  public:
    InstanceFactory(ObjectLoader& objectLoader);

    std::shared_ptr<Instance> getInstance(const MapObject&) const;
  private:
    ObjectLoader& objectLoader;
};

}

#endif
