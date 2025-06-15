#ifndef H_GAME_OBJECTS_INSTANCE
#define H_GAME_OBJECTS_INSTANCE

#include <string>

#include "Object.h"

namespace ZH::Objects {

class InstanceFactory;

class Instance {
  friend InstanceFactory;

  public:
  private:
    glm::vec3 position;
    float angle;
    std::shared_ptr<ObjectBuilder> base;
};

}

#endif
