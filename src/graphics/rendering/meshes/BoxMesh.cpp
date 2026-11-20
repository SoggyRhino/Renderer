#include "graphics/rendering/meshes/BoxMesh.h"

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

namespace {
constexpr std::array kUnitCubeVertices = {
    glm::vec3{-1.0F, -1.0F, -1.0F}, glm::vec3{1.0F, -1.0F, -1.0F},
    glm::vec3{1.0F, -1.0F, 1.0F}, glm::vec3{-1.0F, -1.0F, 1.0F},
    glm::vec3{-1.0F, 1.0F, -1.0F}, glm::vec3{1.0F, 1.0F, -1.0F},
    glm::vec3{1.0F, 1.0F, 1.0F}, glm::vec3{-1.0F, 1.0F, 1.0F},
};

constexpr std::array<uint32_t, 36> kUnitCubeIndices = {
    0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
    0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2,
    0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5,
};
} // namespace

BoxMesh::BoxMesh()
  : vbo(kUnitCubeVertices),
    ebo(kUnitCubeIndices) {
  vao.configure(vbo, sizeof(glm::vec3),
                {{.index = 0, .size = 3, .type = GL_FLOAT}}, &ebo);
}

void BoxMesh::draw() const {
  vao.drawElements(GL_TRIANGLES, static_cast<GLsizei>(kUnitCubeIndices.size()),
                   GL_UNSIGNED_INT);
}