#include "SquareTestScene.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "shaders.h"

SquareTestScene::SquareTestScene()
    : program(SIMPLE_2D_VERT, SIMPLE_2D_FRAG) {
    constexpr float vertices[] = {
        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 1.0f, 1.0f, 0.0f
    };

    VAO.bind();

    VBO.setData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    VAO.setAttributePointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    VAO.setAttributePointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));

    VAO.unbind();
}

void SquareTestScene::update(double dt) {
    if (!pause) {
        currentAngle += speed * static_cast<float>(dt);

        currentAngle = std::fmod(currentAngle, 360.0f);

        rotationMatrix = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(currentAngle),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
    }
}

void SquareTestScene::render() const {
    this->program.use();

    program.setMatx4fv("uRotation", rotationMatrix);

    VAO.drawArrays(GL_TRIANGLES, 0, 6);
}

void SquareTestScene::renderUI() {
    ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
    ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Square Test Settings");
    ImGui::SliderFloat("Speed", &speed, 0.0f, 360.0f);
    ImGui::Checkbox("Pause", &pause);
    ImGui::End();
}
