#pragma once

#include "../../graphics/rendering/Camera.h"
#include "graphics/runtime/Input.h"

class CameraController {
public:
  CameraController(const int width, const int height) : camera(width, height) {
  }

  void onResize(int width, int height);

  void onMouseMovement(float x, float y);

  void resetMouse();

  void update(double dt, const InputState& input);

  [[nodiscard]] Camera& getCamera() {
    return camera;
  }

  [[nodiscard]] const Camera& getCamera() const {
    return camera;
  }

private:
  Camera camera;
};