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
    class ScorchData {
      friend Battlefield;

      public:
        uint64_t id;
        glm::vec3 location;
        float radius = 1.0f;
        uint8_t type = 0;

      private:
        static uint64_t nextID;

        ScorchData();
    };

    Battlefield(
        std::shared_ptr<Map>
      , MapBuilder& mapBuilder
      , Objects::InstanceFactory& instanceFactory
    );

    bool cameraHasMoved() const;
    void frameDoneTick();

    const GFX::Camera& getCamera() const;
    Daytime getDaytime() const;
    std::shared_ptr<Map> getMap() const;
    const glm::vec2& getMapGameSize() const;
    std::list<std::shared_ptr<Objects::Instance>>& getObjectInstances();
    const std::list<ScorchData>& getScorches() const;
    float getWorldHeight(const glm::vec3&) const;
    glm::mat4 getWorldMatrix(const glm::vec3& pos, float radAngle) const;

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
    glm::vec2 mapGameSize;
    Objects::InstanceFactory& instanceFactory;
    GFX::Camera camera;
    bool newMatrices = true;

    std::list<std::shared_ptr<Objects::Instance>> instances;
    std::list<ScorchData> scorches;

    void loadInstances(MapBuilder& mapBuilder);
    void loadScorches(MapBuilder& mapBuilder);
};

}

#endif
