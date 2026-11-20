#pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "graphics/rendering/LightingManager.h"
#include "graphics/rendering/wrappers/Program.h"
#include "graphics/rendering/wrappers/Texture.h"

struct GltfMaterial {
  const Program& program;
  std::shared_ptr<const Texture> baseColorTexture;

  void bind(
      const glm::mat4& model,
      const glm::mat4& view,
      const glm::mat4& projection,
      const std::vector<Light>& lights = {}) const {
    program.use();
    program.setMatx4fv("model", model);
    program.setMatx4fv("view", view);
    program.setMatx4fv("projection", projection);
    LightingManager::apply(program, lights);

    const bool hasTex = baseColorTexture != nullptr && baseColorTexture->
                        isValid();
    program.setBool("hasBaseColor", hasTex);
    if (hasTex) {
      baseColorTexture->bind(0);
      program.setInt("baseColor", 0);
    } else {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
};