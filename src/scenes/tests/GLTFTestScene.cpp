#include "GLTFTestScene.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"
#include "graphics/rendering/wrappers/RenderState.h"

GLTFTestScene::GLTFTestScene(const int windowWidth, const int windowHeight,
                             const std::string& filename) : Scene(windowWidth,
    windowHeight),
  program(MODEL_VERT, MODEL_FRAG),
  model(Model{filename}) {
  RenderState::apply({
      .clearColor = {0.1f, 0.1f, 0.15f, 1.0f},
      .depthTest = true,
      .blend = true,
      .cullFace = true,
  });
}

void GLTFTestScene::updateInput(const double dt, const InputState& input) {
  uint8_t controls = Camera::NONE;

  if (input.held(Key::W)) controls |= Camera::FORWARD;
  if (input.held(Key::S)) controls |= Camera::BACKWARD;
  if (input.held(Key::A)) controls |= Camera::LEFT;
  if (input.held(Key::D)) controls |= Camera::RIGHT;
  if (input.held(Key::Space)) controls |= Camera::UP;
  if (input.held(Key::LeftShift)) controls |= Camera::DOWN;

  camera.updateControls(dt, controls);
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
  program.use();

  program.setMatx4fv("model", rotationMatrix);
  program.setMatx4fv("view", camera.getViewMatrix());
  program.setMatx4fv("projection", camera.getProjectionMatrix());

  model.draw(program);
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