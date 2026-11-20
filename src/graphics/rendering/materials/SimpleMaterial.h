#pragma once

#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Program.h"

struct SimpleMaterial {
  const Program& program;
  glm::vec3 color{1.0F, 1.0F, 1.0F};

  void bind(
      const glm::mat4& model,
      const glm::mat4& view,
      const glm::mat4& projection) const {
    program.use();
    program.setMatx4fv("model", model);
    program.setMatx4fv("view", view);
    program.setMatx4fv("projection", projection);
    program.set3fv("uColor", color);
  }
};