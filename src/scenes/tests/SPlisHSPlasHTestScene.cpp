#include "SPlisHSPlasHTestScene.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/rendering/wrappers/RenderState.h"
#include "shaders.h"

namespace {
constexpr float kParticleRadius = 0.035F;
constexpr float kBoxHalfExtent = 0.8F;
constexpr float kBoxHeight = 1.6F;
constexpr float kWallHalfThickness = 0.05F;
constexpr float kFluidMargin = 1.25F * kParticleRadius;
constexpr float kPoolHeight = 0.50F;
constexpr float kCubeHalfExtent = 0.2F;
constexpr float kCubeStartY = 1.05F;
constexpr float kCubeDensity = 1200.0F;
} // namespace

SPlisHSPlasHTestScene::PhysicsScene SPlisHSPlasHTestScene::buildPhysicsScene() {
  PhysicsEngine::Builder builder;

  const std::array<BoxView, 5> containers = {
      // Floor
      builder.addBox(
          {0.0F, -kWallHalfThickness, 0.0F},
          {kBoxHalfExtent + 2.0F * kWallHalfThickness, kWallHalfThickness,
           kBoxHalfExtent + 2.0F * kWallHalfThickness}),
      // -X wall
      builder.addBox(
          {-(kBoxHalfExtent + kWallHalfThickness), 0.5F * kBoxHeight, 0.0F},
          {kWallHalfThickness, 0.5F * kBoxHeight, kBoxHalfExtent}),
      // +X wall
      builder.addBox(
          {kBoxHalfExtent + kWallHalfThickness, 0.5F * kBoxHeight, 0.0F},
          {kWallHalfThickness, 0.5F * kBoxHeight, kBoxHalfExtent}),
      // -Z wall
      builder.addBox(
          {0.0F, 0.5F * kBoxHeight, -(kBoxHalfExtent + kWallHalfThickness)},
          {kBoxHalfExtent + 2.0F * kWallHalfThickness, 0.5F * kBoxHeight,
           kWallHalfThickness}),
      // +Z wall
      builder.addBox(
          {0.0F, 0.5F * kBoxHeight, kBoxHalfExtent + kWallHalfThickness},
          {kBoxHalfExtent + 2.0F * kWallHalfThickness, 0.5F * kBoxHeight,
           kWallHalfThickness}),
  };

  BoxView cube = builder.addBox(
      {0.0F, kCubeStartY, 0.0F},
      glm::vec3(kCubeHalfExtent),
      true,
      kCubeDensity);
  cube.setActive(false);

  builder.setWorldBounds(
      {-kBoxHalfExtent + 0.5F * kParticleRadius, 0.5F * kParticleRadius,
       -kBoxHalfExtent + 0.5F * kParticleRadius},
      {kBoxHalfExtent - 0.5F * kParticleRadius, kBoxHeight * 2.0F,
       kBoxHalfExtent - 0.5F * kParticleRadius});

  const FluidView water = builder.addFluid(
      {-kBoxHalfExtent + kFluidMargin, kFluidMargin,
       -kBoxHalfExtent + kFluidMargin},
      {kBoxHalfExtent - kFluidMargin, kPoolHeight,
       kBoxHalfExtent - kFluidMargin});

  return {
      .engine = builder.build(),
      .containerBoxes = containers,
      .cube = cube,
      .water = water,
  };
}

SPlisHSPlasHTestScene::SPlisHSPlasHTestScene(
    const int windowWidth,
    const int windowHeight)
  : Scene(windowWidth, windowHeight),
    program(SPH_SIMPLE_VERT, SPH_SIMPLE_FRAG),
    cameraController(windowWidth, windowHeight),
    containerMaterial{program, {0.35F, 0.38F, 0.45F}},
    cubeMaterial{program, {0.95F, 0.45F, 0.2F}},
    waterMaterial{program, {0.2F, 0.65F, 1.0F}, pointSize},
    physics(buildPhysicsScene()) {
  RenderState::apply({
      .clearColor = {0.08F, 0.09F, 0.12F, 1.0F},
      .depthTest = true,
      .blend = true,
      .cullFace = false,
  });
  glEnable(GL_PROGRAM_POINT_SIZE);

  sceneTransform = glm::translate(glm::mat4(1.0F),
                                  glm::vec3(0.0F, -0.75F, -4.0F));
  sceneTransform = glm::rotate(sceneTransform, glm::radians(20.0F),
                               glm::vec3(1.0F, 0.0F, 0.0F));
  sceneTransform = glm::rotate(sceneTransform, glm::radians(-30.0F),
                               glm::vec3(0.0F, 1.0F, 0.0F));

  physics.cube.setActive(showCube);
  particleMesh.upload(physics.water.particles());
}

void SPlisHSPlasHTestScene::updateInput(const double dt,
                                        const InputState& input) {
  cameraController.update(dt, input);
}

void SPlisHSPlasHTestScene::handleMouseMovement(const float x, const float y) {
  cameraController.onMouseMovement(x, y);
}

void SPlisHSPlasHTestScene::resetMouseMovement() {
  cameraController.resetMouse();
}

void SPlisHSPlasHTestScene::handleWindowResize(const int width,
                                               const int height) {
  Scene::handleWindowResize(width, height);
  cameraController.onResize(width, height);
}

void SPlisHSPlasHTestScene::update(const double dt) {
  waterMaterial.pointSize = pointSize;
  physics.cube.setActive(showCube);
  physics.engine.setTimeScale(timeScale);
  physics.engine.setPaused(pausePhysics);
  if (physics.engine.step(dt)) {
    particleMesh.upload(physics.water.particles());
  }
}

void SPlisHSPlasHTestScene::render() const {
  const glm::mat4& view = cameraController.getCamera().getViewMatrix();
  const glm::mat4& projection = cameraController.getCamera().
                                                 getProjectionMatrix();

  if (showContainer) {
    for (const BoxView& box : physics.containerBoxes) {
      containerMaterial.bind(sceneTransform * box.modelMatrix(), view,
                             projection);
      boxMesh.draw();
    }
  }

  waterMaterial.bind(sceneTransform, view, projection);
  particleMesh.draw();

  if (showCube) {
    cubeMaterial.bind(sceneTransform * physics.cube.modelMatrix(), view,
                      projection);
    boxMesh.draw();
  }
}

void SPlisHSPlasHTestScene::renderUI() {
  const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
  ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(340, 245), ImGuiCond_FirstUseEver);

  ImGui::Begin("SPlisHSPlasH Test");
  ImGui::Checkbox("Pause Physics", &pausePhysics);
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    physics.engine.reset();
    particleMesh.upload(physics.water.particles());
  }
  ImGui::Checkbox("Show Falling Cube", &showCube);
  ImGui::Checkbox("Show Container", &showContainer);
  ImGui::SliderFloat("Point Size", &pointSize, 2.0F, 20.0F);
  ImGui::SliderFloat("Time Scale", &timeScale, 0.1F, 2.0F, "%.2fx");
  ImGui::Text("Particles: %zu", physics.water.particleCount());
  ImGui::Text("Sim Time: %.3f s",
              static_cast<double>(physics.engine.getSimTime()));
  ImGui::End();
}