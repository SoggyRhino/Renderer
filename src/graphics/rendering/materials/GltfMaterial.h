#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Program.h"
#include "graphics/rendering/wrappers/Texture.h"

struct GltfMaterial {
  const Program& program;
  const Texture* baseColorTexture = nullptr;

  void bind(
      const glm::mat4& model,
      const glm::mat4& view,
      const glm::mat4& projection) const {
    program.use();
    program.setMatx4fv("model", model);
    program.setMatx4fv("view", view);
    program.setMatx4fv("projection", projection);

    const bool hasTex = baseColorTexture != nullptr && baseColorTexture->isValid();
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
