#include "CameraController.h"

void CameraController::onResize(const int width, const int height) {
  camera.updateWindowDimensions(static_cast<float>(width),
                                static_cast<float>(height));
}

void CameraController::onMouseMovement(const float x, const float y) {
  camera.updateMouse(x, y);
}

void CameraController::update(const double dt, const InputState& input) {
  uint8_t controls = Camera::NONE;

  if (input.held(Key::W)) controls |= Camera::FORWARD;
  if (input.held(Key::S)) controls |= Camera::BACKWARD;
  if (input.held(Key::A)) controls |= Camera::LEFT;
  if (input.held(Key::D)) controls |= Camera::RIGHT;
  if (input.held(Key::Space)) controls |= Camera::UP;
  if (input.held(Key::LeftShift)) controls |= Camera::DOWN;

  camera.updateControls(dt, controls);
}