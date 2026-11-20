#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <glad/glad.h>
#include "tiny_gltf_v3.h"
#include "graphics/rendering/wrappers/Buffers.h"

class Mesh {
public:
  Mesh(const tg3_model& model, const tg3_primitive& prim);
  ~Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&& other) noexcept = default;
  Mesh& operator=(Mesh&& other) noexcept = default;

  void draw() const;

  [[nodiscard]] GLuint getVAO() const {
    return VAO.id();
  }

  [[nodiscard]] GLsizei getCount() const {
    return count;
  }

  [[nodiscard]] bool getHasIndices() const {
    return hasIndices;
  }

private:
  VertexArray VAO;
  std::optional<Buffer> EBO;
  std::vector<Buffer> VBOs;

  GLsizei count = 0;
  GLenum indexType = GL_UNSIGNED_INT;
  uintptr_t indexOffset = 0;
  GLenum mode = GL_TRIANGLES;
  bool hasIndices = false;
};