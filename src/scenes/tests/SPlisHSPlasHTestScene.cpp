#include "SPlisHSPlasHTestScene.h"

#include <algorithm>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/rendering/wrappers/RenderState.h"
#include "shaders.h"

namespace {
constexpr float kMinParticleRadius = 0.015F;
constexpr float kMaxParticleRadius = 0.080F;
constexpr float kBoxHalfExtent = 0.8F;
constexpr float kBoxHeight = 1.6F;
constexpr float kWallHalfThickness = 0.05F;
constexpr float kFluidMarginFactor = 1.25F;
constexpr float kPoolHeight = 0.50F;
constexpr float kCubeHalfExtent = 0.2F;
constexpr float kCubeStartY = 5.05F;
constexpr float kCubeDensity = 1200.0F;
} // namespace

SPlisHSPlasHTestScene::PhysicsScene SPlisHSPlasHTestScene::buildPhysicsScene(
    const float radius) {
  const float safeRadius =
      std::clamp(radius, kMinParticleRadius, kMaxParticleRadius);
  const float fluidMargin = kFluidMarginFactor * safeRadius;

  PhysicsEngine::Builder builder(
      PhysicsConfig{.particleRadius = safeRadius});

  // Floor
  builder.addBox(
      {0.0F, -kWallHalfThickness, 0.0F},
      {kBoxHalfExtent + 2.0F * kWallHalfThickness, kWallHalfThickness,
       kBoxHalfExtent + 2.0F * kWallHalfThickness});
  // -X wall
  builder.addBox(
      {-(kBoxHalfExtent + kWallHalfThickness), 0.5F * kBoxHeight, 0.0F},
      {kWallHalfThickness, 0.5F * kBoxHeight, kBoxHalfExtent});
  // +X wall
  builder.addBox(
      {kBoxHalfExtent + kWallHalfThickness, 0.5F * kBoxHeight, 0.0F},
      {kWallHalfThickness, 0.5F * kBoxHeight, kBoxHalfExtent});
  // -Z wall
  builder.addBox(
      {0.0F, 0.5F * kBoxHeight, -(kBoxHalfExtent + kWallHalfThickness)},
      {kBoxHalfExtent + 2.0F * kWallHalfThickness, 0.5F * kBoxHeight,
       kWallHalfThickness});
  // +Z wall
  builder.addBox(
      {0.0F, 0.5F * kBoxHeight, kBoxHalfExtent + kWallHalfThickness},
      {kBoxHalfExtent + 2.0F * kWallHalfThickness, 0.5F * kBoxHeight,
       kWallHalfThickness});

  BoxView cube = builder.addBox(
      {0.0F, kCubeStartY, 0.0F},
      glm::vec3(kCubeHalfExtent),
      true,
      kCubeDensity);

  builder.setWorldBounds(
      {-kBoxHalfExtent + 0.5F * safeRadius, 0.5F * safeRadius,
       -kBoxHalfExtent + 0.5F * safeRadius},
      {kBoxHalfExtent - 0.5F * safeRadius, kBoxHeight * 2.0F,
       kBoxHalfExtent - 0.5F * safeRadius});

  const FluidView water = builder.addFluid(
      {-kBoxHalfExtent + fluidMargin, fluidMargin,
       -kBoxHalfExtent + fluidMargin},
      {kBoxHalfExtent - fluidMargin, kPoolHeight,
       kBoxHalfExtent - fluidMargin});

  return {
      .engine = builder.build(),
      .cube = cube,
      .water = water,
  };
}

SPlisHSPlasHTestScene::SPlisHSPlasHTestScene(
    const int windowWidth,
    const int windowHeight)
  : Scene(windowWidth, windowHeight),
    program(SPH_SIMPLE_VERT, SPH_SIMPLE_FRAG),
    sphereProgram(SPH_COLORED_SPHERES_VERT, SPH_COLORED_SPHERES_FRAG),
    cameraController(windowWidth, windowHeight),
    cubeMaterial{program, {0.95F, 0.45F, 0.2F}},
    waterMaterial{
        .program = sphereProgram,
        .particleRadius = particleRadius,
        .viewportHeight = static_cast<float>(windowHeight),
        .minHeight = kFluidMarginFactor * particleRadius,
        .maxHeight = kPoolHeight * 1.45F,
    },
    physics(buildPhysicsScene(particleRadius)) {
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

  particleMesh.upload(physics.water.particles());
  lightingManager.addLight({2.0F, 3.0F, 2.0F}, {1.0F, 1.0F, 1.0F}, 3.0F);
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
  waterMaterial.viewportHeight = static_cast<float>(height);
}

void SPlisHSPlasHTestScene::update(const double dt) {
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
  const auto& lights = lightingManager.getLights();

  waterMaterial.bind(sceneTransform, view, projection, lights);
  particleMesh.draw();

  cubeMaterial.bind(sceneTransform * physics.cube.modelMatrix(), view,
                    projection, lights);
  boxMesh.draw();
}

void SPlisHSPlasHTestScene::renderUI() {
  const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
  ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(340, 245), ImGuiCond_FirstUseEver);

  ImGui::Begin("SPlisHSPlasH Test");
  ImGui::Checkbox("Pause Physics", &pausePhysics);
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    particleRadius = std::clamp(particleRadius, kMinParticleRadius,
                                kMaxParticleRadius);
    physics = buildPhysicsScene(particleRadius);
    waterMaterial.particleRadius = particleRadius;
    waterMaterial.minHeight = kFluidMarginFactor * particleRadius;
    particleMesh.upload(physics.water.particles());
  }
  ImGui::SliderFloat("Particle Radius", &particleRadius, kMinParticleRadius,
                     kMaxParticleRadius, "%.3f");
  ImGui::SliderFloat("Time Scale", &timeScale, 0.1F, 2.0F, "%.2fx");
  ImGui::Text("Particles: %zu", physics.water.particleCount());
  ImGui::Text("Sim Time: %.3f s",
              static_cast<double>(physics.engine.getSimTime()));
  ImGui::End();
}