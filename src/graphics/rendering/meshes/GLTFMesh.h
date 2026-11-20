#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <glad/glad.h>

#include "graphics/rendering/wrappers/Buffers.h"

struct GLTFMeshAttribute {
  GLuint location = 0;
  GLint components = 3;
  GLenum componentType = GL_FLOAT;
  GLboolean normalized = GL_FALSE;
  GLsizei stride = 0;
  uintptr_t offset = 0;
  std::shared_ptr<Buffer> vbo;
};

struct GLTFMeshIndices {
  std::shared_ptr<Buffer> ebo;
  GLsizei count = 0;
  GLenum type = GL_UNSIGNED_INT;
  uintptr_t offset = 0;
};

class GLTFMesh {
public:
  GLTFMesh(
      GLenum primitiveMode,
      GLsizei vertexCount,
      std::vector<GLTFMeshAttribute> attributes,
      std::optional<GLTFMeshIndices> indices = std::nullopt);
  ~GLTFMesh() = default;

  GLTFMesh(const GLTFMesh&) = delete;
  GLTFMesh& operator=(const GLTFMesh&) = delete;

  GLTFMesh(GLTFMesh&& other) noexcept;
  GLTFMesh& operator=(GLTFMesh&& other) noexcept;

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
  std::shared_ptr<Buffer> EBO;
  std::vector<std::shared_ptr<Buffer>> VBOs;

  GLsizei count = 0;
  GLenum indexType = GL_UNSIGNED_INT;
  uintptr_t indexOffset = 0;
  GLenum mode = GL_TRIANGLES;
  bool hasIndices = false;
};