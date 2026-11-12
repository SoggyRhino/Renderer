#include <chrono>
#include <thread>

#include "scenes/tests/GLTFTestScene.h"
#include "graphics/rendering/wrappers/RenderState.h"
#include "graphics/runtime/ImGuiLayer.h"
#include "graphics/runtime/Window.h"

int main(int argc, const char* /*argv*/[]) {
  Window window{};
  ImGuiLayer const ui{window};

  GLTFTestScene scene{window.getWidth(), window.getHeight(),
                      "models/BarramundiFish/BarramundiFish.gltf"};
  window.setResizeCallback(
      [&scene](const int width, const int height) {
        scene.handleWindowResize(width, height);
      });
  window.setKeyCallback(
      [&scene](const Key key, const KeyAction action) {
        scene.onKeyEvent(key, action);
      });
  window.setShowCursor(scene.shouldShowCursor());

  auto lastFrame = std::chrono::steady_clock::now();
  while (!window.shouldClose()) {
    window.pollEvents();

    const auto currentFrame = std::chrono::steady_clock::now();
    const double deltaTime = std::chrono::duration<double>(
        currentFrame - lastFrame).count();
    lastFrame = currentFrame;

    if (const double sleep = (1.0F / 120.0F) - deltaTime; sleep > 0) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(static_cast<int>(sleep * 1000)));
    }

    RenderState::clear();

    scene.updateInput(deltaTime, window.input());
    scene.update(deltaTime);
    scene.render();

    if (scene.shouldShowCursor()) {
      ui.begin();
      scene.renderUI();
      ui.end();
    }

    window.update();
  }
}