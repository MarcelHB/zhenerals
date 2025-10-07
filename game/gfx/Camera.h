// SPDX-License-Identifier: GPL-2.0

#ifndef H_GFX_CAMERA
#define H_GFX_CAMERA

#include <glm/glm.hpp>

namespace ZH::GFX {

class Camera {
  public:
    struct Settings {
      float near = 0.0f;
      float far = 1.0f;
      float fovDeg = 90.0f;
      float width = 3.0f;
      float height = 2.0f;
    };

    const glm::mat4& getCameraMatrix() const;
    const glm::vec3& getDirectionVector() const;
    const Settings& getPerspectiveSettings() const;
    const glm::mat4& getProjectionMatrix() const;
    const glm::vec3& getPosition() const;
    const glm::vec3& getUpVector() const;

    void moveAround(float x, float y, const glm::vec3&);
    void moveAxially(float x, float y);
    void moveDirectionally(float x, float y);
    void reposition(
        const glm::vec3& position
      , const glm::vec3& target
      , const glm::vec3& up
    );
    void setPerspectiveProjection(const Settings& settings);
    void zoom(float in);

  private:
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 direction = {0.0f, 0.0f, -1.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    glm::mat4 cameraMatrix = glm::mat4 {1.0f};
    glm::mat4 projectionMatrix = glm::mat4 {1.0f};
    Settings settings;

    void updateCameraMatrix();
    void updateProjectionMatrix();
};

};

#endif
