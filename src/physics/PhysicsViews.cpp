#include "PhysicsViews.h"

#include <cassert>

#include <glm/gtc/matrix_transform.hpp>

#include "physics/objects/Box.h"
#include "physics/objects/FluidObject.h"

glm::vec3 BoxView::position() const {
  assert(box != nullptr);
  return box->getPosition();
}

glm::quat BoxView::rotation() const {
  assert(box != nullptr);
  return box->getRotation();
}

glm::vec3 BoxView::halfExtents() const {
  assert(box != nullptr);
  return box->getHalfExtents();
}

glm::mat4 BoxView::modelMatrix() const {
  assert(box != nullptr);
  return glm::translate(glm::mat4(1.0F), box->getPosition()) *
         glm::mat4_cast(box->getRotation()) *
         glm::scale(glm::mat4(1.0F), box->getHalfExtents());
}

std::span<const glm::vec3> FluidView::particles() const {
  if (fluid == nullptr) {
    return {};
  }
  return fluid->getParticlePositions();
}

std::size_t FluidView::particleCount() const {
  if (fluid == nullptr) {
    return 0;
  }
  return fluid->getParticleCount();
}
