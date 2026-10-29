#pragma once

#include "Scene.h"
#include "utils/Program.h"
#include <glm/glm.hpp>

#include "Paths.h"
#include "../objects/Model.h"

class GLTFTestScene : public Scene {
public:
    explicit GLTFTestScene(const std::string &filename) : program(MODEL_VERTEX_SHADER_3D, MODEL_FRAGMENT_SHADER_3D),
                                                          model(Model{filename}) {
    }

    void update(double dt) override;

    void render() const override;

    void renderUI() override;

private:
    const Program program;
    Model model;

    float speed = 90.0f;
    float currentAngle = 0.0f;
    bool pause = false;

    glm::vec3 cameraPos{0.0f, 0.0f, 5.0f};
    glm::mat4 rotationMatrix{1.0f};
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
};
