#pragma once

#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Program.h"

struct ParticleMaterial {
  const Program& program;
  glm::vec3 color{0.2F, 0.65F, 1.0F};
  float pointSize = 8.0F;

  void bind(
      const glm::mat4& model,
      const glm::mat4& view,
      const glm::mat4& projection) const {
    program.use();
    program.setMatx4fv("model", model);
    program.setMatx4fv("view", view);
    program.setMatx4fv("projection", projection);
    program.set3fv("uColor", color);
    program.setFloat("uPointSize", pointSize);
  }
};