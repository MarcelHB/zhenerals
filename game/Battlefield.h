// SPDX-License-Identifier: GPL-2.0

#ifndef H_GAME_BATTLEFIELD
#define H_GAME_BATTLEFIELD

#include "common.h"
#include "Map.h"
#include "objects/InstanceFactory.h"
#include "gfx/Camera.h"

namespace ZH {

class Battlefield {
  public:
    Battlefield(
        std::shared_ptr<Map>
      , MapBuilder& mapBuilder
      , Objects::InstanceFactory& instanceFactory
    );

    bool cameraHasMoved() const;
    void frameDoneTick();

    const GFX::Camera& getCamera() const;
    glm::mat4 getObjectToGridMatrix(const glm::vec3& pos, float radAngle) const;
    Daytime getDaytime() const;
    std::shared_ptr<Map> getMap() const;

    std::list<std::shared_ptr<Objects::Instance>>& getObjectInstances();

    void moveCameraAxially(float x, float y);
    void moveCameraDirectionally(float x, float y);
    void setPerspectiveProjection(
        float near
      , float far
      , float fovDeg
      , float width
      , float height
    );
    void zoomCamera(float in);
  private:
    std::shared_ptr<Map> map;
    Objects::InstanceFactory& instanceFactory;
    GFX::Camera camera;
    bool newMatrices = true;

    std::list<std::shared_ptr<Objects::Instance>> instances;

    void loadInstances(MapBuilder& mapBuilder);
};

}

#endif
