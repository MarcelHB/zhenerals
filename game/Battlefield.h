#ifndef H_GAME_BATTLEFIELD
#define H_GAME_BATTLEFIELD

#include "common.h"
#include "Map.h"
#include "objects/InstanceFactory.h"

namespace ZH {

class Battlefield {
  public:
    Battlefield(
        std::shared_ptr<Map>
      , MapBuilder& mapBuilder
      , Objects::InstanceFactory& instanceFactory
    );

    glm::mat4 getCameraMatrix() const;
    glm::mat4 getObjectToWorldMatrix(const glm::vec3& pos, float radAngle) const;
    Daytime getDaytime() const;
    std::shared_ptr<Map> getMap() const;

    std::list<std::shared_ptr<Objects::Instance>>& getObjectInstances();
  private:
    std::shared_ptr<Map> map;
    Objects::InstanceFactory& instanceFactory;
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;

    std::list<std::shared_ptr<Objects::Instance>> instances;

    void loadInstances(MapBuilder& mapBuilder);
};

}

#endif
