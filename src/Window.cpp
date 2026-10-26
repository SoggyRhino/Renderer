#include "Window.h"
#include <stdexcept>


Window::Window(std::string title, int height, int width) : title(title), height(height), width(width) {
    if (!glfwInit()) {
        throw std::runtime_error("Could not init GLFW");
    }

    glfwSetErrorCallback([](int error, const char *description) {
        printf("Error: %s\n", description);
    });
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);

    if (!window) {
        throw std::runtime_error("Could not create Window");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, const int w, const int h) {
        glViewport(0, 0, w, h);
    });
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    glfwShowWindow(window);
}



Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::update() const {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Window::clear() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}