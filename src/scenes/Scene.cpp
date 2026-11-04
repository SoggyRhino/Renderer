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

void Scene::onKeyEvent(const Key key, const KeyAction action) {
  if (action != KeyAction::Press) {
    return;
  }

  if (const auto it = callbacks.find(key); it != callbacks.end()) {
    it->second();
  }
}

void Scene::handleWindowResize(const int width, const int height) {
  windowWidth = width;
  windowHeight = height;
  camera.updateWindowDimensions(static_cast<float>(width),
                                static_cast<float>(height));
}