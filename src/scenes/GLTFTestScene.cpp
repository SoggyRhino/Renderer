#include "GLTFTestScene.h"

#include <format>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../objects/Model.h"


void GLTFTestScene::update(const double dt) {
    if (!pause) {
        currentAngle += speed * static_cast<float>(dt);
        currentAngle = std::fmod(currentAngle, 360.0f);

        rotationMatrix = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(currentAngle),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }

    viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

void GLTFTestScene::render() const {
    program.use();

    program.setMatx4fv("model", rotationMatrix);
    program.setMatx4fv("view", viewMatrix);
    program.setMatx4fv("projection", projectionMatrix);

    model.draw();
}

void GLTFTestScene::renderUI() {
    const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
    ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("GLTF Test Settings");
    ImGui::SliderFloat("Speed", &speed, 0.0f, 360.0f);
    ImGui::Checkbox("Pause", &pause);

    ImGui::Separator();
    ImGui::Text("Camera Controls");
    ImGui::SliderFloat3("Camera Position (XYZ)", glm::value_ptr(cameraPos), -20.0f, 20.0f);
    ImGui::End();
}
