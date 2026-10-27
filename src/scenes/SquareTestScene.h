#pragma once
#include "Scene.h"
#include "utils/Program.h"


class SquareTestScene : public Scene {
public:
    SquareTestScene();

    ~SquareTestScene() override;

    void update(double dt) override;

    void render() const override;

private:
    const Program program;
    GLuint VAO, VBO;
};
