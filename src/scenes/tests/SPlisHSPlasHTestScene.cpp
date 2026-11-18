#include "SPlisHSPlasHTestScene.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/rendering/wrappers/RenderState.h"
#include "shaders.h"

namespace {
constexpr float kParticleRadius = 0.035f;
constexpr float kBoxHalfExtent = 0.8f;
constexpr float kBoxHeight = 1.6f;
constexpr float kFluidMargin = 1.25f * kParticleRadius;
constexpr float kPoolHeight = 0.50f;
constexpr float kCubeHalfExtent = 0.2f;
constexpr float kCubeStartY = 1.05f;
constexpr float kCubeDensity = 1200.0f;
} // namespace

SPlisHSPlasHTestScene::SPlisHSPlasHTestScene(
    const int windowWidth,
    const int windowHeight)
  : Scene(windowWidth, windowHeight),
    program(SPH_SIMPLE_VERT, SPH_SIMPLE_FRAG),
    cameraController(windowWidth, windowHeight),
    boxOutline(Box::fromMinMax(
        {-kBoxHalfExtent, 0.0f, -kBoxHalfExtent},
        {kBoxHalfExtent, kBoxHeight, kBoxHalfExtent})),
    cube({0.0f, kCubeStartY, 0.0f}, glm::vec3(kCubeHalfExtent), true, kCubeDensity),
    water(
        {-kBoxHalfExtent + kFluidMargin, kFluidMargin, -kBoxHalfExtent + kFluidMargin},
        {kBoxHalfExtent - kFluidMargin, kPoolHeight, kBoxHalfExtent - kFluidMargin}) {
  RenderState::apply({
      .clearColor = {0.08f, 0.09f, 0.12f, 1.0f},
      .depthTest = true,
      .blend = true,
      .cullFace = false,
  });
  glEnable(GL_PROGRAM_POINT_SIZE);

  // Position the simulation box in front of the default camera with a slight tilt
  sceneTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.75f, -4.0f));
  sceneTransform = glm::rotate(sceneTransform, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  sceneTransform = glm::rotate(sceneTransform, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  boxOutline.setCollisionEnabled(false);
  boxOutline.setWireframe(true);
  boxOutline.setColor({0.75f, 0.78f, 0.85f});

  cube.setColor({0.95f, 0.45f, 0.2f});
  cube.setVisible(showCube);

  water.setColor({0.2f, 0.65f, 1.0f});
  water.setPointSize(pointSize);

  engine.setWorldBounds(
      {-kBoxHalfExtent, 0.0f, -kBoxHalfExtent},
      {kBoxHalfExtent, kBoxHeight, kBoxHalfExtent});

  engine.registerObject(cube);
  engine.registerObject(water);
  engine.init();
}

void SPlisHSPlasHTestScene::updateInput(const double dt, const InputState& input) {
  cameraController.update(dt, input);
}

void SPlisHSPlasHTestScene::handleWindowResize(const int width, const int height) {
  Scene::handleWindowResize(width, height);
  cameraController.onResize(width, height);
}

void SPlisHSPlasHTestScene::update(const double dt) {
  cube.setVisible(showCube);
  water.setPointSize(pointSize);
  engine.setTimeScale(timeScale);
  engine.setPaused(pausePhysics);
  engine.step(dt);
}

void SPlisHSPlasHTestScene::render() const {
  program.use();
  program.setMatx4fv("view", cameraController.getCamera().getViewMatrix());
  program.setMatx4fv("projection", cameraController.getCamera().getProjectionMatrix());

  boxOutline.render(program, sceneTransform);
  water.render(program, sceneTransform);
  cube.render(program, sceneTransform);
}

void SPlisHSPlasHTestScene::renderUI() {
  const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
  ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(340, 245), ImGuiCond_FirstUseEver);

  ImGui::Begin("SPlisHSPlasH Test");
  ImGui::Checkbox("Pause Physics", &pausePhysics);
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    engine.reset();
  }
  ImGui::Checkbox("Show Falling Cube", &showCube);
  ImGui::SliderFloat("Point Size", &pointSize, 2.0f, 20.0f);
  ImGui::SliderFloat("Time Scale", &timeScale, 0.1f, 2.0f, "%.2fx");
  ImGui::Text("Particles: %zu", water.getParticleCount());
  ImGui::Text("Sim Time: %.3f s", static_cast<double>(engine.getSimTime()));
  ImGui::Text(
      "Step: %.2f ms (%d substeps, %.2fx realtime)",
      static_cast<double>(engine.getLastStepTimeMs()),
      engine.getLastSubsteps(),
      static_cast<double>(engine.getLastSimSpeedRatio()));
  ImGui::End();
}
