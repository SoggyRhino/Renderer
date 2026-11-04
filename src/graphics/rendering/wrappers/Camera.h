#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

class Camera {
public:
  enum ControlFlags : uint8_t {
    NONE = 0,
    FORWARD = 1 << 0,
    BACKWARD = 1 << 1,
    LEFT = 1 << 2,
    RIGHT = 1 << 3,
    UP = 1 << 4,
    DOWN = 1 << 5,
    SPRINT = 1 << 6,
  };

  Camera(uint32_t height, uint32_t width);

  void updateWindowDimensions(float width, float height);

  void updateMouse(float x, float y);

  void updateControls(double deltaTime, uint8_t activeControls);

  [[nodiscard]] glm::vec3 getPosition() const;

  [[nodiscard]] glm::mat4 getViewMatrix() const;

  [[nodiscard]] glm::mat4 getProjectionMatrix() const;

private:
  glm::vec3 cameraPos{0.0f, 0.0f, 0.0f};
  glm::vec3 cameraFront{0.0f, 0.0f, -1.0f};
  glm::vec3 cameraUp{0.0f, 1.0f, 0.0f};
  glm::vec3 cameraRight{1.0f, 0.0f, 0.0f};
  float width;
  float height;
  float lastX;
  float lastY;

  float sensitivity = 0.1f;
  float yaw = -90.0f;
  float pitch = 0.0f;
  float fov = 45.0f;
  float speed = .05;
  float sprintMultiplier = 1;
};