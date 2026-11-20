#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window.h"
#include <stdexcept>
#include <utility>
#include <format>

Window::Window(const WindowConfig& config) :
  title(config.title), width(config.width), height(config.height) {
  if (!glfwInit()) {
    const char* description = nullptr;
    int errorCode = glfwGetError(&description);

    throw std::runtime_error(
        std::format(
            "Could not init GLFW (Code: 0x{:08X}, Reason: {})",
            errorCode,
            description ? description : "Unknown error"
            ));
  }

  glfwSetErrorCallback(logError);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);

  window = glfwCreateWindow(
      this->width,
      this->height,
      title.c_str(),
      nullptr,
      nullptr);

  if (!window) {
    const char* description = nullptr;
    int errorCode = glfwGetError(&description);

    throw std::runtime_error(
        std::format(
            "Could not create Window \"{}\" ({}x{}) [Code: 0x{:08X}, Reason: {}]",
            this->title,
            this->width,
            this->height,
            errorCode,
            description ? description : "Unknown error"
            ));
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetKeyCallback(window, key);
  glfwSetCursorPosCallback(window, cursorPos);
  glfwSetMouseButtonCallback(window, mouseButton);
  gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
  glfwSwapInterval(config.vsync ? 1 : 0);

  glfwShowWindow(window);
}


Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::update() const {
  glfwSwapBuffers(window);
}

void Window::pollEvents() {
  glfwPollEvents();

  if (capturesKeyboard && capturesKeyboard()) {
    inputState.clear();
  }
}

bool Window::shouldClose() const {
  return glfwWindowShouldClose(window);
}

void Window::setShowCursor(const bool show) {
  showCursor = show;
  if (!showCursor) {
    rightMouseLook = false;
  }
  glfwSetInputMode(
      window,
      GLFW_CURSOR,
      (showCursor && !rightMouseLook) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
  if (onMouseLookReset) {
    onMouseLookReset();
  }
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
  onResize = std::move(callback);
}

void Window::setKeyCallback(std::function<void(Key, KeyAction)> callback) {
  onKey = std::move(callback);
}

void Window::setMouseCallback(std::function<void(float, float)> callback) {
  onMouse = std::move(callback);
}

void Window::setMouseLookResetCallback(std::function<void()> callback) {
  onMouseLookReset = std::move(callback);
}

void Window::setKeyboardCaptureQuery(std::function<bool()> query) {
  capturesKeyboard = std::move(query);
}

void Window::setMouseCaptureQuery(std::function<bool()> query) {
  capturesMouse = std::move(query);
}

void Window::logError(int error, const char* description) {
  printf("Error %d: %s\n", error, description);
}

void Window::resize(GLFWwindow* window, const int w, const int h) {
  auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (!self) {
    return;
  }

  self->width = w;
  self->height = h;
  glViewport(0, 0, w, h);

  if (self->onResize) {
    self->onResize(w, h);
  }
}

void Window::key(
    GLFWwindow* window,
    const int glfwKey,
    const int,
    const int glfwAction,
    const int) {
  auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (!self) {
    return;
  }

  if (self->capturesKeyboard && self->capturesKeyboard()) {
    return;
  }

  Key key;
  switch (glfwKey) {
    case GLFW_KEY_W:
      key = Key::W;
      break;
    case GLFW_KEY_A:
      key = Key::A;
      break;
    case GLFW_KEY_S:
      key = Key::S;
      break;
    case GLFW_KEY_D:
      key = Key::D;
      break;
    case GLFW_KEY_SPACE:
      key = Key::Space;
      break;
    case GLFW_KEY_LEFT_SHIFT:
      key = Key::LeftShift;
      break;
    case GLFW_KEY_ESCAPE:
      key = Key::Escape;
      break;
    case GLFW_KEY_TAB:
      key = Key::Tab;
      break;
    default:
      return;
  }

  KeyAction action;
  switch (glfwAction) {
    case GLFW_PRESS:
      action = KeyAction::Press;
      self->inputState.setHeld(key, true);
      break;
    case GLFW_RELEASE:
      action = KeyAction::Release;
      self->inputState.setHeld(key, false);
      break;
    case GLFW_REPEAT:
      action = KeyAction::Repeat;
      break;
    default:
      return;
  }

  if (self->onKey) {
    self->onKey(key, action);
  }
}

void Window::cursorPos(
    GLFWwindow* window,
    const double xpos,
    const double ypos) {
  const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (!self || (!self->rightMouseLook && self->showCursor)) {
    return;
  }

  if (self->onMouse) {
    self->onMouse(static_cast<float>(xpos), static_cast<float>(ypos));
  }
}

void Window::mouseButton(
    GLFWwindow* window,
    const int button,
    const int action,
    const int) {
  auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (!self || button != GLFW_MOUSE_BUTTON_RIGHT) {
    return;
  }

  if (action == GLFW_PRESS) {
    if (!self->showCursor || (self->capturesMouse && self->capturesMouse())) {
      return;
    }
    self->rightMouseLook = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (self->onMouseLookReset) {
      self->onMouseLookReset();
    }
  } else if (action == GLFW_RELEASE && self->rightMouseLook) {
    self->rightMouseLook = false;
    glfwSetInputMode(
        window,
        GLFW_CURSOR,
        self->showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    if (self->onMouseLookReset) {
      self->onMouseLookReset();
    }
  }
}