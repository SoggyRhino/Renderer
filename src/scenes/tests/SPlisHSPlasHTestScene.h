#pragma once

#include <glm/glm.hpp>

#include "../Scene.h"
#include "graphics/rendering/wrappers/Program.h"
#include "physics/PhysicsEngine.h"
#include "physics/objects/Box.h"
#include "physics/objects/FluidObject.h"
#include "scenes/components/CameraController.h"

class SPlisHSPlasHTestScene : public Scene {
public:
  SPlisHSPlasHTestScene(int windowWidth, int windowHeight);
  ~SPlisHSPlasHTestScene() override = default;

  void updateInput(double dt, const InputState& input) override;
  void handleWindowResize(int width, int height) override;
  void update(double dt) override;
  void render() const override;
  void renderUI() override;

private:
  const Program program;
  CameraController cameraController;

  Box cube;
  FluidObject water;
  PhysicsEngine engine;

  glm::mat4 sceneTransform{1.0f};
  bool pausePhysics = true;
  bool showCube = false;
  float pointSize = 8.0f;
  float timeScale = 1.0f;
};
