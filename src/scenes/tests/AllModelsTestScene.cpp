#include "AllModelsTestScene.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "models.h"
#include "shaders.h"

void AllModelsTestScene::initModelNames() {
  modelNames.clear();
  modelNames.reserve(Models::all.size());
  for (const auto& item : Models::all) {
    std::filesystem::path p(item.value);
    modelNames.push_back(p.stem().string());
  }
}

AllModelsTestScene::AllModelsTestScene(
    const int windowWidth,
    const int windowHeight,
    size_t initialIndex)
  : Scene(windowWidth, windowHeight), program(MODEL_VERT, MODEL_FRAG) {
  initModelNames();
  if constexpr (!Models::all.empty()) {
    const size_t index = (initialIndex < Models::all.size()) ? initialIndex : 0;
    loadModel(index);
  }
}

bool AllModelsTestScene::loadModel(const size_t index) {
  if (index >= Models::all.size()) {
    return false;
  }

  currentModelIndex = static_cast<int>(index);
  const std::string_view pathView = Models::all[index].value;
  const std::string& displayName = modelNames[index];

  try {
    model = std::make_unique<Model>(std::string(pathView));
    loadFailed = false;
    statusMessage = std::format("Loaded: {}", displayName);
    return true;
  } catch (const std::exception& e) {
    model.reset();
    loadFailed = true;
    statusMessage = std::format(
        "Failed to load '{}': {}",
        displayName,
        e.what());
    return false;
  }
}


void AllModelsTestScene::update(const double dt) {
  if (!pause) {
    currentAngle += speed * static_cast<float>(dt);
    currentAngle = std::fmod(currentAngle, 360.0f);

    rotationMatrix = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(currentAngle),
        glm::vec3(0.0f, 1.0f, 0.0f)
        );
  }

  viewMatrix = glm::lookAt(
      cameraPos,
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));
  projectionMatrix = glm::perspective(
      glm::radians(45.0f),
      800.0f / 600.0f,
      0.1f,
      100.0f);
}

void AllModelsTestScene::render() const {
  program.use();

  program.setMatx4fv("model", rotationMatrix);
  program.setMatx4fv("view", viewMatrix);
  program.setMatx4fv("projection", projectionMatrix);

  if (model) {
    model->draw(program);
  }
}

void AllModelsTestScene::renderUI() {
  const ImVec2 topLeft = ImGui::GetMainViewport()->Pos;
  ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(520, 450), ImGuiCond_FirstUseEver);

  ImGui::Begin("All Models Test Settings");

  ImGui::Text("Model Selector");

  ImGui::SetNextItemWidth(250.0f);
  ImGui::InputTextWithHint(
      "##filter",
      "Filter models...",
      filterBuf,
      sizeof(filterBuf));
  if (filterBuf[0] != '\0') {
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) {
      filterBuf[0] = '\0';
    }
  }

  const char* previewValue = (currentModelIndex >= 0 && currentModelIndex <
                              static_cast<int>(modelNames.size()))
                               ? modelNames[currentModelIndex].c_str()
                               : "Select Model...";

  if (ImGui::BeginCombo("Model", previewValue)) {
    for (int i = 0; i < static_cast<int>(Models::all.size()); ++i) {
      if (filterBuf[0] != '\0') {
        auto it = std::search(
            modelNames[i].begin(),
            modelNames[i].end(),
            filterBuf,
            filterBuf + std::strlen(filterBuf),
            [](const char ch1, const char ch2) {
              return std::tolower(static_cast<unsigned char>(ch1)) ==
                     std::tolower(static_cast<unsigned char>(ch2));
            }
            );
        if (it == modelNames[i].end()) {
          continue;
        }
      }

      const bool isSelected = (currentModelIndex == i);
      if (ImGui::Selectable(modelNames[i].c_str(), isSelected)) {
        if (currentModelIndex != i) {
          loadModel(i);
        }
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  if (ImGui::Button("< Prev") && currentModelIndex > 0) {
    loadModel(currentModelIndex - 1);
  }
  ImGui::SameLine();
  if (ImGui::Button("Next >") && currentModelIndex + 1 < static_cast<int>(
        Models::all.size())) {
    loadModel(currentModelIndex + 1);
  }
  ImGui::SameLine();
  ImGui::Text("(%d of %zu)", currentModelIndex + 1, Models::all.size());

  if (loadFailed) {
    ImGui::TextColored(
        ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
        "%s",
        statusMessage.c_str());
  } else if (model) {
    ImGui::TextDisabled("Meshes: %zu", model->getMeshes().size());
  }

  ImGui::Separator();
  ImGui::SliderFloat("Speed", &speed, 0.0f, 360.0f);
  ImGui::Checkbox("Pause", &pause);

  ImGui::Separator();
  ImGui::Text("Camera Controls");
  ImGui::SliderFloat3(
      "Camera Position (XYZ)",
      glm::value_ptr(cameraPos),
      -20.0f,
      20.0f);
  if (ImGui::Button("Reset Camera")) {
    cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
  }

  ImGui::End();
}