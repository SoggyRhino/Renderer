#pragma once
#include <functional>
#include <ranges>
#include <unordered_map>

#include "GLFW/glfw3.h"
#include "utils/Camera.h"

class Scene {
public:
  Scene(int windowWidth, int windowHeight);

  virtual ~Scene() = default;

  virtual void update(double dt) = 0;

  virtual void render() const = 0;

  virtual void renderUI() = 0;

  void handleMouseMovement(float x, float y);

  [[nodiscard]] bool shouldShowCursor() const;

  [[nodiscard]] bool shouldRenderUI() const;

  std::vector<int> getBoundKeys();

  void handleKeyPress(int key);

  void handleWindowResize(int width, int height);

  void setupCameraBindings();

protected:
  int windowWidth{}, windowHeight{};
  bool showUI = true;
  bool showCursor = true;
  uint8_t controls{0};
  Camera camera;

  std::unordered_map<int, std::function<void()>> callbacks;

  void bind(const int key, std::function<void()> callback) {
    callbacks[key] = std::move(callback);
  }
};