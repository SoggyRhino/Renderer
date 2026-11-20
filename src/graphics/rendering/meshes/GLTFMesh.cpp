#include "graphics/rendering/meshes/GLTFMesh.h"

#include <utility>

GLTFMesh::GLTFMesh(
    const GLenum primitiveMode,
    const GLsizei vertexCount,
    std::vector<GLTFMeshAttribute> attributes,
    std::optional<GLTFMeshIndices> indices)
  : mode(primitiveMode) {
  VAO.bind();

  VBOs.reserve(attributes.size());
  for (GLTFMeshAttribute& attr : attributes) {
    if (attr.vbo == nullptr) {
      continue;
    }

    attr.vbo->bind(GL_ARRAY_BUFFER);
    VAO.setAttributePointer(
        attr.location,
        attr.components,
        attr.componentType,
        attr.normalized,
        attr.stride,
        reinterpret_cast<const void*>(attr.offset));
    VBOs.push_back(std::move(attr.vbo));
  }

  if (indices.has_value() && indices->ebo != nullptr) {
    hasIndices = true;
    count = indices->count;
    indexType = indices->type;
    indexOffset = indices->offset;
    EBO = std::move(indices->ebo);
    EBO->bind(GL_ELEMENT_ARRAY_BUFFER);
  } else {
    count = vertexCount;
  }

  VAO.unbind();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLTFMesh::GLTFMesh(GLTFMesh&& other) noexcept
  : VAO(std::move(other.VAO)),
    EBO(std::move(other.EBO)),
    VBOs(std::move(other.VBOs)),
    count(std::exchange(other.count, 0)),
    indexType(other.indexType),
    indexOffset(std::exchange(other.indexOffset, 0)),
    mode(other.mode),
    hasIndices(std::exchange(other.hasIndices, false)) {
}

GLTFMesh& GLTFMesh::operator=(GLTFMesh&& other) noexcept {
  if (this != &other) {
    VAO = std::move(other.VAO);
    EBO = std::move(other.EBO);
    VBOs = std::move(other.VBOs);
    count = std::exchange(other.count, 0);
    indexType = other.indexType;
    indexOffset = std::exchange(other.indexOffset, 0);
    mode = other.mode;
    hasIndices = std::exchange(other.hasIndices, false);
  }
  return *this;
}

void GLTFMesh::draw() const {
  if (count <= 0 || VAO.id() == 0) {
    return;
  }

  if (hasIndices) {
    VAO.drawElements(
        mode,
        count,
        indexType,
        reinterpret_cast<const void*>(indexOffset));
  } else {
    VAO.drawArrays(mode, 0, count);
  }
}