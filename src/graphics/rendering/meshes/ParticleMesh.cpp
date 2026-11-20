#include "graphics/rendering/meshes/ParticleMesh.h"

#include <utility>

ParticleMesh::ParticleMesh() {
  vbo.setData(0, nullptr, GL_DYNAMIC_DRAW);
  vao.configure(vbo, sizeof(glm::vec3),
                {{.index = 0, .size = 3, .type = GL_FLOAT}});
}

ParticleMesh::ParticleMesh(ParticleMesh&& other) noexcept
  : vao(std::move(other.vao)),
    vbo(std::move(other.vbo)),
    particleCount(std::exchange(other.particleCount, 0)),
    capacityBytes(std::exchange(other.capacityBytes, 0)) {
}

ParticleMesh& ParticleMesh::operator=(ParticleMesh&& other) noexcept {
  if (this != &other) {
    vao = std::move(other.vao);
    vbo = std::move(other.vbo);
    particleCount = std::exchange(other.particleCount, 0);
    capacityBytes = std::exchange(other.capacityBytes, 0);
  }
  return *this;
}

void ParticleMesh::upload(const std::span<const glm::vec3> positions) {
  particleCount = static_cast<GLsizei>(positions.size());
  if (particleCount <= 0 || vbo.id() == 0) {
    return;
  }

  const std::size_t byteSize = positions.size_bytes();
  if (byteSize <= capacityBytes) {
    glNamedBufferSubData(vbo.id(), 0, static_cast<GLsizeiptr>(byteSize),
                         positions.data());
  } else {
    capacityBytes = byteSize;
    vbo.setData(static_cast<GLsizeiptr>(byteSize), positions.data(),
                GL_DYNAMIC_DRAW);
  }
}

void ParticleMesh::draw() const {
  if (particleCount <= 0 || vao.id() == 0) {
    return;
  }
  vao.drawArrays(GL_POINTS, 0, particleCount);
}
