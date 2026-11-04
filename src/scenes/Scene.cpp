#include "Scene.h"

Scene::Scene(const int windowWidth, const int windowHeight) :
  windowWidth(windowWidth),
  windowHeight(windowHeight),
  camera(windowWidth, windowHeight) {
}

void Scene::handleMouseMovement(float x, float y) {
}

[[nodiscard]] bool Scene::shouldShowCursor() const {
  return showCursor;
}

[[nodiscard]] bool Scene::shouldRenderUI() const {
  return showUI;
}

std::vector<int> Scene::getBoundKeys() {
  auto keys_view = std::views::keys(callbacks);
  std::vector keys(keys_view.begin(), keys_view.end());
  return keys;
}

void Scene::handleKeyPress(const int key) {
  if (const auto it = callbacks.find(key); it != callbacks.end()) {
    it->second();
  }
}

void Scene::handleWindowResize(const int width, const int height) {
  windowWidth = width;
  windowHeight = height;
}

void Scene::setupCameraBindings() {
  bind(GLFW_KEY_W, [this] {
    controls |= Camera::FORWARD;
  });
  bind(GLFW_KEY_S, [this] {
    controls |= Camera::BACKWARD;
  });
  bind(GLFW_KEY_A, [this] {
    controls |= Camera::LEFT;
  });
  bind(GLFW_KEY_D, [this] {
    controls |= Camera::RIGHT;
  });
  bind(GLFW_KEY_SPACE, [this] {
    controls |= Camera::UP;
  });
  bind(GLFW_KEY_LEFT_SHIFT, [this] {
    controls |= Camera::DOWN;
  });
}