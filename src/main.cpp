#include <thread>

#include "scenes/SquareTestScene.h"
#include "utils/Window.h"

int main(int argc, const char *argv[]) {
    Window window{};
    SquareTestScene scene{};

    double lastFrame = 0.0f;

    while (!window.shouldClose()) {
        const double currentFrame = glfwGetTime();
        const double deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (const double sleep = (1.0f / 120.0f) - deltaTime; sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep * 1000)));
        }

        window.clear();

        scene.render();
        scene.update(deltaTime);

        window.beginUI();
        scene.renderUI();
        window.endUI();

        window.update();
    }
}
