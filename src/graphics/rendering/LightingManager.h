#pragma once

#include <algorithm>
#include <format>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Program.h"

struct Light {
  glm::vec3 position{0.0F};
  glm::vec3 color{1.0F};
  float intensity = 1.0F;
};

class LightingManager {
public:
  static constexpr std::size_t kMaxLights = 16;

  std::size_t addLight(const Light& light) {
    lights.push_back(light);
    return lights.size() - 1;
  }

  std::size_t addLight(
      const glm::vec3& position,
      const glm::vec3& color = glm::vec3(1.0F),
      const float intensity = 1.0F) {
    lights.push_back(Light{
        .position = position,
        .color = color,
        .intensity = intensity,
    });
    return lights.size() - 1;
  }

  void removeLight(const std::size_t index) {
    if (index < lights.size()) {
      lights.erase(lights.begin() + static_cast<std::ptrdiff_t>(index));
    }
  }

  void clear() {
    lights.clear();
  }

  [[nodiscard]] Light& getLight(const std::size_t index) {
    return lights.at(index);
  }

  [[nodiscard]] const Light& getLight(const std::size_t index) const {
    return lights.at(index);
  }

  [[nodiscard]] std::vector<Light>& getLights() {
    return lights;
  }

  [[nodiscard]] const std::vector<Light>& getLights() const {
    return lights;
  }

  [[nodiscard]] std::size_t count() const {
    return lights.size();
  }

  [[nodiscard]] bool empty() const {
    return lights.empty();
  }

  static void apply(const Program& program, const std::vector<Light>& lights) {
    if (!program.hasUniform("uLightCount")) {
      return;
    }

    const std::size_t uploadCount = std::min(lights.size(), kMaxLights);
    program.setInt("uLightCount", static_cast<int>(uploadCount));

    for (std::size_t i = 0; i < uploadCount; ++i) {
      program.set3fv(std::format("uLights[{}].position", i), lights[i].position);
      program.set3fv(std::format("uLights[{}].color", i), lights[i].color);
      program.setFloat(std::format("uLights[{}].intensity", i),
                       lights[i].intensity);
    }
  }

private:
  std::vector<Light> lights;
};
