#include "Window.h"
#include <stdexcept>
#include <utility>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <format>

Window::Window(std::string title, int width, int height): title(std::move(title)), width(width), height(height) {
    if (!glfwInit()) {
        const char* description = nullptr;
        int errorCode = glfwGetError(&description);

        throw std::runtime_error(std::format(
            "Could not init GLFW (Code: 0x{:08X}, Reason: {})",
            errorCode,
            description ? description : "Unknown error"
        ));
    }

    glfwSetErrorCallback([](int error, const char *description) {
        printf("Error: %s\n", description);
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);

    if (!window) {
        const char* description = nullptr;
        int errorCode = glfwGetError(&description);

        throw std::runtime_error(std::format(
            "Could not create Window \"{}\" ({}x{}) [Code: 0x{:08X}, Reason: {}]",
            this->title,
            width,
            height,
            errorCode,
            description ? description : "Unknown error"
        ));
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, const int w, const int h) {
        glViewport(0, 0, w, h);
    });
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glfwShowWindow(window);
}


Window::~Window() {
    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::update() const {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Window::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::beginUI() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::endUI() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
