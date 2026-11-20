#include "GLTFTestScene.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"
#include "graphics/rendering/loader/GltfLoader.h"
#include "graphics/rendering/wrappers/RenderState.h"

GLTFTestScene::GLTFTestScene(
    const int windowWidth,
    const int windowHeight,
    const std::string& filename) : Scene(
                                       windowWidth,
                                       windowHeight),
                                   program(MODEL_VERT, MODEL_FRAG),
                                   asset(GltfLoader::load(filename, program)),
                                   cameraController(windowWidth, windowHeight) {
  RenderState::apply(
  {
      .clearColor = {0.1f, 0.1f, 0.15f, 1.0f},
      .depthTest = true,
      .blend = true,
      .cullFace = true,
  });
}

void GLTFTestScene::updateInput(const double dt, const InputState& input) {
  cameraController.update(dt, input);
}

void GLTFTestScene::handleMouseMovement(const float x, const float y) {
  cameraController.onMouseMovement(x, y);
}

void GLTFTestScene::resetMouseMovement() {
  cameraController.resetMouse();
}

void GLTFTestScene::handleWindowResize(const int width, const int height) {
  Scene::handleWindowResize(width, height);
  cameraController.onResize(width, height);
}

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
}

void GLTFTestScene::render() const {
  const glm::mat4& view = cameraController.getCamera().getViewMatrix();
  const glm::mat4& projection = cameraController.getCamera().
                                                 getProjectionMatrix();

  for (const auto& [material, instances] : asset.batches) {
    for (const auto& instance : instances) {
      material->bind(rotationMatrix * instance.transform, view, projection);
      instance.mesh->draw();
    }
  }
}

void GLTFTestScene::renderUI() {
  const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
  ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

  ImGui::Begin("GLTF Test Settings");
  ImGui::SliderFloat("Speed", &speed, 0.0f, 360.0f);
  ImGui::Checkbox("Pause", &pause);
  ImGui::End();
}