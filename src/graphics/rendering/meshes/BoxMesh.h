#pragma once

#include "graphics/rendering/wrappers/Buffers.h"

class BoxMesh {
public:
  BoxMesh();
  ~BoxMesh() = default;

  BoxMesh(const BoxMesh&) = delete;
  BoxMesh& operator=(const BoxMesh&) = delete;

  BoxMesh(BoxMesh&&) noexcept = default;
  BoxMesh& operator=(BoxMesh&&) noexcept = default;

  void draw() const;

private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;
};
