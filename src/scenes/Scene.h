#pragma once
#include <functional>
#include <unordered_map>

#include "graphics/runtime/Input.h"

class Scene {
public:
  Scene(int windowWidth, int windowHeight);

  virtual ~Scene() = default;

  virtual void update(double dt) = 0;

  virtual void updateInput(double dt, const InputState& input) {
  }

  virtual void render() const = 0;

  virtual void renderUI() = 0;

  void handleMouseMovement(float x, float y);

  [[nodiscard]] bool shouldShowCursor() const;

  [[nodiscard]] bool shouldRenderUI() const;

  virtual void onKeyEvent(Key key, KeyAction action);

  virtual void handleWindowResize(int width, int height);

protected:
  int windowWidth{}, windowHeight{};
  bool showUI = true;
  bool showCursor = true;
  std::unordered_map<Key, std::function<void()>> callbacks;

  void bind(const Key key, std::function<void()> callback) {
    callbacks[key] = std::move(callback);
  }
};