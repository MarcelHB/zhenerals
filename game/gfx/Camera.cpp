#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
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

const glm::mat4& Camera::getProjectionMatrix() const {
  return projectionMatrix;
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

void Camera::setPerspectiveProjection(
    float near
  , float far
  , float fovDeg
  , float width
  , float height
) {
  this->near = near;
  this->far = far;
  this->fovDeg = fovDeg;
  this->width = width;
  this->height = height;

  updateProjectionMatrix();
}

void Camera::updateCameraMatrix() {
  cameraMatrix = glm::lookAt(position, position + direction, up);
}

void Camera::updateProjectionMatrix() {
  projectionMatrix =
    glm::perspective(
        glm::radians(fovDeg)
      , width / height
      , near
      , far
    );
}

void Camera::zoom(float in) {
  position += direction * in;
  updateCameraMatrix();
}

}
