#pragma once

#include <glad/glad.h>
#include "Scene.h"
#include "utils/Program.h"

class SquareTestScene : public Scene {
public:
    SquareTestScene();

    ~SquareTestScene() override;

    void update(double dt) override;

    void render() const override;

    void renderUI() override;

private:
    const Program program;
    GLuint VAO, VBO;

    float speed = 90.0f;
    float currentAngle = 0.0f;
    bool pause = false;
    glm::mat4 rotationMatrix{1.0f};
};
