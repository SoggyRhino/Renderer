#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(const uint32_t height, const uint32_t width) :
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
  yaw += (x - lastX) * sensitivity;
  pitch += (lastY - y) * sensitivity;

  pitch = std::max(std::min(pitch, 89.0f), -89.0f);

  cameraFront = glm::normalize(
      glm::vec3(
          cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
          sin(glm::radians(pitch)),
          sin(glm::radians(yaw)) * cos(glm::radians(pitch))));

  cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

  lastX = x;
  lastY = y;
}

void Camera::updateControls(const double deltaTime,
                            const uint8_t activeControls) {
  glm::vec3 moveDirection(0.0f);

  if (activeControls & FORWARD) moveDirection += glm::vec3(0.0f, 0.0f, -1.0f);
  if (activeControls & BACKWARD) moveDirection += glm::vec3(0.0f, 0.0f, 1.0f);
  if (activeControls & RIGHT) moveDirection += glm::vec3(1.0f, 0.0f, 0.0f);
  if (activeControls & LEFT) moveDirection += glm::vec3(-1.0f, 0.0f, 0.0f);
  if (activeControls & UP) moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);
  if (activeControls & DOWN) moveDirection += glm::vec3(0.0f, -1.0f, 0.0f);

  if (glm::length(moveDirection) > 0.0f) {
    moveDirection = glm::normalize(moveDirection);
  }

  cameraPos += moveDirection * speed * (activeControls & SPRINT ? 2.0f : 1.0f) *
      static_cast<float>(deltaTime);
}

glm::vec3 Camera::getPosition() const {
  return cameraPos;
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Camera::getProjectionMatrix() const {
  return glm::perspective(glm::radians(fov), width / height, 0.1f, 10000.0f);
}