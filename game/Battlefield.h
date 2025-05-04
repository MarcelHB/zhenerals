#ifndef H_GAME_BATTLEFIELD
#define H_GAME_BATTLEFIELD

#include "Map.h"

namespace ZH {

class Battlefield {
  public:
    Battlefield(std::shared_ptr<Map>);

    glm::mat4 getCameraMatrix();
    std::shared_ptr<Map> getMap() const;
  private:
    std::shared_ptr<Map> map;
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;
};

}

#endif
