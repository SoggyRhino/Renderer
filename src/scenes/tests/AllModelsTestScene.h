#pragma once

#include "../Scene.h"
#include "utils/Program.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "../../objects/Model.h"

class AllModelsTestScene : public Scene {
public:
    AllModelsTestScene();
    explicit AllModelsTestScene(size_t initialIndex);
    ~AllModelsTestScene() override = default;

    void update(double dt) override;

    void render() const override;

    void renderUI() override;

    bool loadModel(size_t index);

    [[nodiscard]] int getCurrentModelIndex() const { return currentModelIndex; }
    [[nodiscard]] const Model *getModel() const { return model.get(); }

private:
    const Program program;
    std::unique_ptr<Model> model;

    int currentModelIndex = 0;
    std::vector<std::string> modelNames;
    std::string statusMessage;
    bool loadFailed = false;
    char filterBuf[64] = "";

    float speed = 90.0f;
    float currentAngle = 0.0f;
    bool pause = false;

    glm::vec3 cameraPos{0.0f, 0.0f, 5.0f};
    glm::mat4 rotationMatrix{1.0f};
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};

    void initModelNames();
};
