#pragma once

#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <string>

class Window {
public:
  explicit Window(std::string title = "Renderer", int width = 800,
                  int height = 600);

  ~Window();

  void update() const;

  void clear();

  void beginUI() const;

  void endUI() const;

  [[nodiscard]] bool shouldClose() const;

  [[nodiscard]] uint32_t getWidth() const { return width; }
  [[nodiscard]] uint32_t getHeight() const { return height; }

  [[nodiscard]] bool isPressed(int key) const;

  void setShowCursor(bool show) const;

private:
  static constexpr auto clear_color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
  const std::string title;
  int width;
  int height;

  GLFWwindow* window{};


  static void logError(int error, const char* description);
  static void resize(GLFWwindow* window, int width, int height);
};