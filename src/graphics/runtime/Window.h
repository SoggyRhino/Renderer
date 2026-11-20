#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

#include "Input.h"

class ImGuiLayer;

struct WindowConfig {
  std::string title = "Renderer";
  int width = 800;
  int height = 600;
  bool resizable = true;
  bool decorated = true;
  bool vsync = true;
};

class Window {
public:
  explicit Window(const WindowConfig& config = {});

  ~Window();

  void update() const;

  void pollEvents();

  [[nodiscard]] bool shouldClose() const;

  [[nodiscard]] int getWidth() const {
    return width;
  }

  [[nodiscard]] int getHeight() const {
    return height;
  }

  [[nodiscard]] const InputState& input() const {
    return inputState;
  }

  void setShowCursor(bool show);

  void setResizeCallback(std::function<void(int, int)> callback);

  void setKeyCallback(std::function<void(Key, KeyAction)> callback);

  void setMouseCallback(std::function<void(float, float)> callback);

  void setMouseLookResetCallback(std::function<void()> callback);

  void setKeyboardCaptureQuery(std::function<bool()> query);

  void setMouseCaptureQuery(std::function<bool()> query);

private:
  const std::string title;
  int width;
  int height;

  GLFWwindow* window{};
  InputState inputState;
  bool showCursor = true;
  bool rightMouseLook = false;
  std::function<void(int, int)> onResize;
  std::function<void(Key, KeyAction)> onKey;
  std::function<void(float, float)> onMouse;
  std::function<void()> onMouseLookReset;
  std::function<bool()> capturesKeyboard;
  std::function<bool()> capturesMouse;

  [[nodiscard]] GLFWwindow* nativeHandle() const {
    return window;
  }

  static void logError(int error, const char* description);
  static void resize(GLFWwindow* window, int width, int height);
  static void key(
      GLFWwindow* window,
      int key,
      int scancode,
      int action,
      int mods);
  static void cursorPos(GLFWwindow* window, double xpos, double ypos);
  static void mouseButton(
      GLFWwindow* window,
      int button,
      int action,
      int mods);

  friend class ImGuiLayer;
};