// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_OBJECTS_INSTANCE
#define H_GAME_OBJECTS_INSTANCE

#include <string>

#include "../common.h"
#include "Object.h"

namespace ZH::Objects {

class InstanceFactory;

class Instance {
  friend InstanceFactory;

  public:
    float getAngle() const;
    std::shared_ptr<const ObjectBuilder> getBase() const;
    uint64_t getID() const;
    const glm::vec3& getPosition() const;

    bool needsRedraw() const;
    void setRedrawn();

  private:
    static uint64_t nextID;

    uint64_t id;
    bool drawUpdate = true;

    glm::vec3 position;
    float angle;
    std::shared_ptr<const ObjectBuilder> base;
};

}

#endif
