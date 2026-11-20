#pragma once

#include <cstddef>
#include <span>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Buffers.h"

class ParticleMesh {
public:
  ParticleMesh();
  ~ParticleMesh() = default;

  ParticleMesh(const ParticleMesh&) = delete;
  ParticleMesh& operator=(const ParticleMesh&) = delete;

  ParticleMesh(ParticleMesh&& other) noexcept;
  ParticleMesh& operator=(ParticleMesh&& other) noexcept;

  void upload(std::span<const glm::vec3> positions);
  void draw() const;

  [[nodiscard]] GLsizei count() const { return particleCount; }

private:
  VertexArray vao;
  Buffer vbo;
  GLsizei particleCount = 0;
  std::size_t capacityBytes = 0;
};
