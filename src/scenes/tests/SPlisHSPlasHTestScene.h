#pragma once

#include <array>

#include <glm/glm.hpp>

#include "../Scene.h"
#include "graphics/rendering/materials/ParticleMaterial.h"
#include "graphics/rendering/materials/SimpleMaterial.h"
#include "graphics/rendering/meshes/BoxMesh.h"
#include "graphics/rendering/meshes/ParticleMesh.h"
#include "graphics/rendering/wrappers/Program.h"
#include "physics/PhysicsEngine.h"
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
  struct PhysicsScene {
    PhysicsEngine engine;
    std::array<BoxView, 5> containerBoxes;
    BoxView cube;
    FluidView water;
  };

  static PhysicsScene buildPhysicsScene();

  const Program program;
  CameraController cameraController;

  BoxMesh boxMesh;
  ParticleMesh particleMesh;

  SimpleMaterial containerMaterial;
  SimpleMaterial cubeMaterial;
  ParticleMaterial waterMaterial;

  PhysicsScene physics;

  glm::mat4 sceneTransform{1.0F};
  bool pausePhysics = true;
  bool showCube = false;
  bool showContainer = false;
  float pointSize = 8.0F;
  float timeScale = 1.0F;
};
