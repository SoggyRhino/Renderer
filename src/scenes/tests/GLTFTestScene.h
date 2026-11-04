#pragma once

#include "../Scene.h"
#include "utils/Program.h"

#include "../../objects/Model.h"

class GLTFTestScene : public Scene {
public:
  explicit GLTFTestScene(int windowWidth, int windowHeight,
                         const std::string& filename);

  void update(double dt) override;

  void render() const override;

  void renderUI() override;

private:
  const Program program;
  Model model;

  float speed = 90.0f;
  float currentAngle = 0.0f;
  bool pause = false;

  glm::mat4 rotationMatrix{1.0f};
};