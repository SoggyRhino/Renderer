#include "Scene.h"

Scene::Scene(const int windowWidth, const int windowHeight) :
  windowWidth(windowWidth),
  windowHeight(windowHeight) {
  bind(Key::Tab, [this] {
    showCursor = !showCursor;
    showUI = showCursor;
  });
  bind(Key::Escape, [this] {
    showCursor = true;
    showUI = true;
  });
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
}