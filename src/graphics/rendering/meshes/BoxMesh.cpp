#include "graphics/rendering/meshes/BoxMesh.h"

#include <array>
#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

namespace {
struct BoxVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;
};

constexpr std::array<BoxVertex, 24> kUnitCubeVertices = {
    // +Z (Front)
    BoxVertex{{-1.0F, -1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}},
    BoxVertex{{1.0F, -1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 0.0F}},
    BoxVertex{{1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 1.0F}},
    BoxVertex{{-1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}},

    // -Z (Back)
    BoxVertex{{1.0F, -1.0F, -1.0F}, {0.0F, 0.0F, -1.0F}, {0.0F, 0.0F}},
    BoxVertex{{-1.0F, -1.0F, -1.0F}, {0.0F, 0.0F, -1.0F}, {1.0F, 0.0F}},
    BoxVertex{{-1.0F, 1.0F, -1.0F}, {0.0F, 0.0F, -1.0F}, {1.0F, 1.0F}},
    BoxVertex{{1.0F, 1.0F, -1.0F}, {0.0F, 0.0F, -1.0F}, {0.0F, 1.0F}},

    // +X (Right)
    BoxVertex{{1.0F, -1.0F, 1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 0.0F}},
    BoxVertex{{1.0F, -1.0F, -1.0F}, {1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}},
    BoxVertex{{1.0F, 1.0F, -1.0F}, {1.0F, 0.0F, 0.0F}, {1.0F, 1.0F}},
    BoxVertex{{1.0F, 1.0F, 1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 1.0F}},

    // -X (Left)
    BoxVertex{{-1.0F, -1.0F, -1.0F}, {-1.0F, 0.0F, 0.0F}, {0.0F, 0.0F}},
    BoxVertex{{-1.0F, -1.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}},
    BoxVertex{{-1.0F, 1.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}, {1.0F, 1.0F}},
    BoxVertex{{-1.0F, 1.0F, -1.0F}, {-1.0F, 0.0F, 0.0F}, {0.0F, 1.0F}},

    // +Y (Top)
    BoxVertex{{-1.0F, 1.0F, 1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F}},
    BoxVertex{{1.0F, 1.0F, 1.0F}, {0.0F, 1.0F, 0.0F}, {1.0F, 0.0F}},
    BoxVertex{{1.0F, 1.0F, -1.0F}, {0.0F, 1.0F, 0.0F}, {1.0F, 1.0F}},
    BoxVertex{{-1.0F, 1.0F, -1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F}},

    // -Y (Bottom)
    BoxVertex{{-1.0F, -1.0F, -1.0F}, {0.0F, -1.0F, 0.0F}, {0.0F, 0.0F}},
    BoxVertex{{1.0F, -1.0F, -1.0F}, {0.0F, -1.0F, 0.0F}, {1.0F, 0.0F}},
    BoxVertex{{1.0F, -1.0F, 1.0F}, {0.0F, -1.0F, 0.0F}, {1.0F, 1.0F}},
    BoxVertex{{-1.0F, -1.0F, 1.0F}, {0.0F, -1.0F, 0.0F}, {0.0F, 1.0F}},
};

constexpr std::array<uint32_t, 36> kUnitCubeIndices = {
    0,  1,  2,  0,  2,  3,   // +Z
    4,  5,  6,  4,  6,  7,   // -Z
    8,  9,  10, 8,  10, 11,  // +X
    12, 13, 14, 12, 14, 15,  // -X
    16, 17, 18, 16, 18, 19,  // +Y
    20, 21, 22, 20, 22, 23,  // -Y
};
} // namespace

BoxMesh::BoxMesh()
  : vbo(kUnitCubeVertices),
    ebo(kUnitCubeIndices) {
  vao.configure(
      vbo, sizeof(BoxVertex),
      {
          {.index = 0,
           .size = 3,
           .type = GL_FLOAT,
           .relativeOffset = offsetof(BoxVertex, position)},
          {.index = 1,
           .size = 3,
           .type = GL_FLOAT,
           .relativeOffset = offsetof(BoxVertex, normal)},
          {.index = 2,
           .size = 2,
           .type = GL_FLOAT,
           .relativeOffset = offsetof(BoxVertex, texCoord)},
      },
      &ebo);
}

void BoxMesh::draw() const {
  vao.drawElements(GL_TRIANGLES, static_cast<GLsizei>(kUnitCubeIndices.size()),
                   GL_UNSIGNED_INT);
}