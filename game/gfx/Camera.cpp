#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.h"

namespace ZH::GFX {

const glm::mat4& Camera::getCameraMatrix() const {
  return cameraMatrix;
}

const glm::vec3& Camera::getDirectionVector() const {
  return direction;
}

const Camera::Settings& Camera::getPerspectiveSettings() const {
  return settings;
}

const glm::mat4& Camera::getProjectionMatrix() const {
  return projectionMatrix;
}

const glm::vec3& Camera::getPosition() const {
  return position;
}

void Camera::moveAround(float x, float y, const glm::vec3& p) {
  auto rayAxis = glm::normalize(position - p);
  auto screenAxis = glm::cross(rayAxis, up);

  auto rotXAxis = glm::cross(rayAxis, screenAxis);
  auto rotX = glm::rotate(x, rotXAxis);

  auto rotYAxis = glm::cross(position, up);
  auto rotY = glm::rotate(-y, rotYAxis);

  auto rot = rotY * rotX;

  position = rot * glm::vec4 {position, 1.0};
  up = rot * glm::vec4 {up, 1.0};

  updateCameraMatrix();

  direction = glm::normalize(p - position);
  cameraMatrix = glm::lookAt(position, p, up);
}

void Camera::moveAxially(float x, float y) {
  auto screenXAxis = glm::cross(direction, up);

  screenXAxis *= x;
  position.x += screenXAxis.x;
  position.y += y;
  position.z += screenXAxis.z;

  updateCameraMatrix();
}

void Camera::moveDirectionally(float x, float y) {
  auto screenXAxis = glm::cross(direction, up);

  auto rotXMatrix =
    glm::rotate(
        glm::mat4 {1.0f}
      , glm::radians(y)
      , screenXAxis
    );
  auto rotYMatrix =
    glm::rotate(
        glm::mat4 {1.0f}
      , glm::radians(x)
      , up
    );
  auto rotMatrix = rotYMatrix * rotXMatrix;

  direction = glm::vec3{rotMatrix * glm::vec4 {direction, 1.0f}};

  updateCameraMatrix();
}

void Camera::reposition(
    const glm::vec3& position
  , const glm::vec3& target
  , const glm::vec3& up
) {
  this->position = position;
  direction = glm::normalize(target - position);
  this->up = up;

  updateCameraMatrix();
}

void Camera::setPerspectiveProjection(const Settings& settings) {
  this->settings = settings;

  updateProjectionMatrix();
}

void Camera::updateCameraMatrix() {
  cameraMatrix = glm::lookAt(position, position + direction, up);
}

void Camera::updateProjectionMatrix() {
  projectionMatrix =
    glm::perspective(
        glm::radians(settings.fovDeg)
      , settings.width / settings.height
      , settings.near
      , settings.far
    );
}

void Camera::zoom(float in) {
  position += direction * in;
  updateCameraMatrix();
}

}
