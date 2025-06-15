#ifndef H_GAME_BATTLEFIELD
#define H_GAME_BATTLEFIELD

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

    glm::mat4 getCameraMatrix();
    Daytime getDaytime() const;
    std::shared_ptr<Map> getMap() const;
  private:
    std::shared_ptr<Map> map;
    Objects::InstanceFactory& instanceFactory;
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;
};

}

#endif
