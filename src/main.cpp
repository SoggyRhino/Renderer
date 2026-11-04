#include <thread>

#include "scenes/tests/AllModelsTestScene.h"
#include "scenes/tests/GLTFTestScene.h"
#include "utils/Window.h"

int main(int argc, const char* argv[]) {
  Window window{};

  GLTFTestScene scene{window.getWidth(), window.getHeight(),
                      "models/BarramundiFish/BarramundiFish.gltf"};
  const auto keys = scene.getBoundKeys();
  window.setShowCursor(scene.shouldShowCursor());

  double lastFrame = 0.0f;
  while (!window.shouldClose()) {
    glfwPollEvents();

    for (const auto& key : keys) {
      if (window.isPressed(key)) {
        scene.handleKeyPress(key);
      }
    }

    const double currentFrame = glfwGetTime();
    const double deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (const double sleep = 1.0f / 120.0f - deltaTime; sleep > 0) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(static_cast<int>(sleep * 1000)));
    }

    window.clear();

    scene.render();
    scene.update(deltaTime);

    if (scene.shouldShowCursor()) {
      window.beginUI();
      scene.renderUI();
      window.endUI();
    }

    window.update();
  }
}