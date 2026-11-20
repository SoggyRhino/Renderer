#pragma once

#include "../Scene.h"
#include "graphics/rendering/loader/GltfAsset.h"
#include "graphics/rendering/wrappers/Program.h"
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

class AllModelsTestScene : public Scene {
public:
  AllModelsTestScene(
      int windowWidth,
      int windowHeight,
      size_t initialIndex = 0);

  ~AllModelsTestScene() override = default;

  void update(double dt) override;

  void render() const override;

  void renderUI() override;

  bool loadModel(size_t index);

  [[nodiscard]] int getCurrentModelIndex() const {
    return currentModelIndex;
  }

  [[nodiscard]] const GltfAsset* getAsset() const {
    return asset ? &(*asset) : nullptr;
  }

private:
  const Program program;
  std::optional<GltfAsset> asset;

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