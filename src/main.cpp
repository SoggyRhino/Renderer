#include <chrono>

#include "scenes/tests/SPlisHSPlasHTestScene.h"
#include "graphics/rendering/wrappers/RenderState.h"
#include "graphics/runtime/ImGuiLayer.h"
#include "graphics/runtime/Window.h"

int main(int argc, const char* /*argv*/[]) {
  Window window{};
  ImGuiLayer const ui{window};

  SPlisHSPlasHTestScene scene{window.getWidth(), window.getHeight()};
  window.setResizeCallback(
      [&scene](const int width, const int height) {
        scene.handleWindowResize(width, height);
      });
  window.setKeyCallback(
      [&scene](const Key key, const KeyAction action) {
        scene.onKeyEvent(key, action);
      });
  window.setMouseCallback(
      [&scene](const float x, const float y) {
        scene.handleMouseMovement(x, y);
      });
  window.setMouseLookResetCallback(
      [&scene] {
        scene.resetMouseMovement();
      });

  bool showCursor = scene.shouldShowCursor();
  window.setShowCursor(showCursor);

  auto lastFrame = std::chrono::steady_clock::now();
  while (!window.shouldClose()) {
    window.pollEvents();

    const auto currentFrame = std::chrono::steady_clock::now();
    const double deltaTime = std::chrono::duration<double>(
        currentFrame - lastFrame).count();
    lastFrame = currentFrame;

    RenderState::clear();

    scene.updateInput(deltaTime, window.input());
    scene.update(deltaTime);

    if (const bool nextShowCursor = scene.shouldShowCursor();
        nextShowCursor != showCursor) {
      showCursor = nextShowCursor;
      window.setShowCursor(showCursor);
    }

    scene.render();

    if (scene.shouldRenderUI()) {
      ui.begin();
      scene.renderUI();
      ui.end();
    }

    window.update();
  }
}