#pragma once

#include "../Scene.h"
#include "utils/Buffers.h"
#include "utils/Program.h"

class SquareTestScene : public Scene {
public:
  SquareTestScene(uint32_t windowWidth, uint32_t windowHeight);

  ~SquareTestScene() override = default;

  void update(double dt) override;

  void render() const override;

  void renderUI() override;

private:
  const Program program;
  VertexArray VAO;
  Buffer VBO;

  float speed = 90.0f;
  float currentAngle = 0.0f;
  bool pause = false;
  glm::mat4 rotationMatrix{1.0f};
};