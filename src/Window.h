#pragma once

//don't remove, imported for side effects
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <string>

class Window {
public:
    explicit Window(std::string title = "Renderer", int width = 800, int height = 600);

    ~Window();

    void update() const;

    void clear() const;

private:
    static constexpr auto clear_color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
    const std::string title{};
    int height{};
    int width{};

    GLFWwindow *window{};
};
