#pragma once

#include "../Scene.h"
#include "graphics/rendering/loader/GltfAsset.h"
#include "graphics/rendering/wrappers/Program.h"
#include "scenes/components/CameraController.h"


class GLTFTestScene : public Scene {
public:
  explicit GLTFTestScene(
      int windowWidth,
      int windowHeight,
      const std::string& filename);

  void updateInput(double dt, const InputState& input) override;

  void handleMouseMovement(float x, float y) override;

  void resetMouseMovement() override;

  void handleWindowResize(int width, int height) override;

  void update(double dt) override;

  void render() const override;

  void renderUI() override;

private:
  const Program program;
  GltfAsset asset;
  CameraController cameraController;

  float speed = 90.0f;
  float currentAngle = 0.0f;
  bool pause = false;

  glm::mat4 rotationMatrix{1.0f};
};