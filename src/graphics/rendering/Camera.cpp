#include "graphics/rendering/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(const uint32_t width, const uint32_t height) :
  width(static_cast<float>(width)),
  height(static_cast<float>(height)),
  lastX(static_cast<float>(width) / 2.0f),
  lastY(static_cast<float>(height) / 2.0f) {
}

void Camera::updateWindowDimensions(const float w, const float h) {
  width = w;
  height = h;
}

void Camera::updateMouse(const float x, const float y) {
  if (firstMouse) {
    lastX = x;
    lastY = y;
    firstMouse = false;
    return;
  }

  yaw += (x - lastX) * sensitivity;
  pitch += (lastY - y) * sensitivity;

  pitch = std::clamp(pitch, -89.0f, 89.0f);

  cameraFront = glm::normalize(
      glm::vec3(
          cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
          sin(glm::radians(pitch)),
          sin(glm::radians(yaw)) * cos(glm::radians(pitch))));

  cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

  lastX = x;
  lastY = y;
}

void Camera::resetMouse() {
  firstMouse = true;
}

void Camera::updateControls(
    const double deltaTime,
    const uint8_t activeControls) {
  glm::vec3 moveDirection(0.0f);

  if (activeControls & FORWARD)
    moveDirection += cameraFront;
  if (activeControls & BACKWARD)
    moveDirection -= cameraFront;
  if (activeControls & RIGHT)
    moveDirection += cameraRight;
  if (activeControls & LEFT)
    moveDirection -= cameraRight;
  if (activeControls & UP)
    moveDirection += cameraUp;
  if (activeControls & DOWN)
    moveDirection -= cameraUp;

  if (glm::dot(moveDirection, moveDirection) > 1e-6f) {
    moveDirection = glm::normalize(moveDirection);
  }

  cameraPos += moveDirection * speed *
      (activeControls & SPRINT ? sprintMultiplier : 1.0f) *
      static_cast<float>(deltaTime);
}

glm::vec3 Camera::getPosition() const {
  return cameraPos;
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Camera::getProjectionMatrix() const {
  const float aspect = (height > 0.0f) ? (width / height) : 1.0f;
  return glm::perspective(glm::radians(fov), aspect, 0.1f, 10000.0f);
}