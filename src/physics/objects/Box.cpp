#include "physics/objects/Box.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <ranges>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>

#include "SPlisHSPlasH/BoundaryModel_Akinci2012.h"
#include "SPlisHSPlasH/RigidBodyObject.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/StaticRigidBody.h"

#include "graphics/rendering/wrappers/Program.h"
#include "physics/BoundarySampling.h"
#include "physics/SphMath.h"
#include "physics/objects/DynamicRigidBody.h"

namespace {
constexpr std::array kUnitVertices = {
    glm::vec3{-1.0F, -1.0F, -1.0F}, glm::vec3{1.0F, -1.0F, -1.0F},
    glm::vec3{1.0F, -1.0F, 1.0F}, glm::vec3{-1.0F, -1.0F, 1.0F},
    glm::vec3{-1.0F, 1.0F, -1.0F}, glm::vec3{1.0F, 1.0F, -1.0F},
    glm::vec3{1.0F, 1.0F, 1.0F}, glm::vec3{-1.0F, 1.0F, 1.0F},
};

constexpr std::array<uint32_t, 36> kBoxIndices = {
    0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
    0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2,
    0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5,
};
} // namespace

Box::Box(
    const glm::vec3& initialPos,
    const glm::vec3& halfExtentsIn,
    const bool isDyn,
    const float bodyDensity)
  : initialPosition(initialPos),
    position(initialPos),
    halfExtents(glm::max(halfExtentsIn, glm::vec3(0.0f))),
    dynamic(isDyn),
    density(bodyDensity) {
  rebuildMesh();
}

void Box::rebuildMesh() {
  std::array<glm::vec3, kUnitVertices.size()> vertices{};
  std::ranges::transform(
      kUnitVertices, vertices.begin(),
      [this](const glm::vec3& corner) { return corner * halfExtents; });

  vbo = Buffer(vertices);
  ebo = Buffer(kBoxIndices);

  vao.configure(vbo, sizeof(glm::vec3),
                {{.index = 0, .size = 3, .type = GL_FLOAT}}, &ebo);
}

void Box::render(const Program& program,
                 const glm::mat4& parentTransform) const {
  if (!visible) {
    return;
  }

  const glm::mat4 model = parentTransform * glm::translate(
                              glm::mat4(1.0f), position) * glm::mat4_cast(
                              rotation);
  program.setMatx4fv("model", model);
  program.set3fv("uColor", color);

  vao.drawElements(GL_TRIANGLES, kBoxIndices.size(),GL_UNSIGNED_INT);
}

std::vector<glm::vec3> Box::buildBoundaryParticles(const float spacing) const {
  std::vector<glm::vec3> pts;
  const glm::vec3 span = 2.0f * halfExtents;

  appendGridFace(pts, {-halfExtents.x, -halfExtents.y, -halfExtents.z},
                 {1, 0, 0}, {0, 0, 1}, span.x, span.z, spacing);
  appendGridFace(pts, {-halfExtents.x, halfExtents.y, -halfExtents.z},
                 {1, 0, 0}, {0, 0, 1}, span.x, span.z, spacing);

  const float innerY = span.y - 2.0f * spacing;
  if (innerY >= 0.0f) {
    appendGridFace(
        pts, {-halfExtents.x, -halfExtents.y + spacing, -halfExtents.z},
        {1, 0, 0}, {0, 1, 0}, span.x, innerY, spacing);
    appendGridFace(
        pts, {-halfExtents.x, -halfExtents.y + spacing, halfExtents.z},
        {1, 0, 0}, {0, 1, 0}, span.x, innerY, spacing);

    const float innerZ = span.z - 2.0f * spacing;
    if (innerZ >= 0.0f) {
      appendGridFace(
          pts, {-halfExtents.x, -halfExtents.y + spacing,
                -halfExtents.z + spacing}, {0, 0, 1}, {0, 1, 0}, innerZ, innerY,
          spacing);
      appendGridFace(
          pts, {halfExtents.x, -halfExtents.y + spacing,
                -halfExtents.z + spacing}, {0, 0, 1}, {0, 1, 0}, innerZ, innerY,
          spacing);
    }
  }
  return pts;
}

void Box::computeMassAndInertia() {
  constexpr float kMinMass = 1e-6F;
  constexpr float kMinInertia = 1e-8F;

  const glm::vec3 size = 2.0F * halfExtents;
  mass = std::max(density * glm::compMul(size), kMinMass);

  const float inertia = (mass * glm::dot(size, size)) / 18.0F;
  invInertia = (inertia > kMinInertia) ? (1.0F / inertia) : 0.0F;
}

void Box::initPhysics(
    SPH::Simulation& sim,
    const float particleRadius,
    const float boundarySpacingFactor) {
  computeMassAndInertia();

  const auto toBoundaryPoint = [this](const glm::vec3& localPoint) {
    return toSph(dynamic ? localPoint : (position + rotation * localPoint));
  };

  auto boundaryPoints =
      buildBoundaryParticles(boundarySpacingFactor * particleRadius) |
      std::views::transform(toBoundaryPoint) |
      std::ranges::to<std::vector>();

  std::unique_ptr<SPH::RigidBodyObject> rigidBody;
  if (dynamic) {
    rigidBody = std::make_unique<DynamicRigidBody>();
  } else {
    rigidBody = std::make_unique<SPH::StaticRigidBody>();
  }

  auto model = std::make_unique<SPH::BoundaryModel_Akinci2012>();
  model->initModel(rigidBody.release(),
                   static_cast<unsigned int>(boundaryPoints.size()),
                   boundaryPoints.data());

  sim.addBoundaryModel(model.get());
  boundaryModel = model.release();

  if (dynamic) {
    syncBoundaryParticles();
  }
}

void Box::resetPhysics() {
  assert(boundaryModel != nullptr);

  position = initialPosition;
  rotation = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
  velocity = glm::vec3{};
  angularVelocity = glm::vec3{};

  boundaryModel->clearForceAndTorque();
  if (dynamic) {
    syncBoundaryParticles();
  }
}

void Box::integrate(const float stepDt, const glm::vec3& gravity) {
  if (!dynamic || stepDt <= 0.0F) {
    return;
  }
  assert(boundaryModel != nullptr);

  Vector3r sphForce;
  Vector3r sphTorque;
  boundaryModel->getForceAndTorque(sphForce, sphTorque);
  boundaryModel->clearForceAndTorque();

  const glm::vec3 fluidForce = toGlm(sphForce);
  const glm::vec3 fluidTorque = toGlm(sphTorque);

  velocity += (gravity + fluidForce / std::max(mass, 1e-6f)) * stepDt;
  angularVelocity = (angularVelocity + fluidTorque * invInertia * stepDt) *
                    0.995f;
  position += velocity * stepDt;

  const glm::quat angVelQ(0.0f, angularVelocity.x, angularVelocity.y,
                          angularVelocity.z);
  rotation = glm::normalize(rotation + 0.5f * stepDt * (angVelQ * rotation));
}

void Box::resolveStaticCollision(const PhysicsObject& staticObject,
                                 const float particleRadius) {
  const auto* staticBox = dynamic_cast<const Box*>(&staticObject);
  if (staticBox == nullptr || !dynamic) {
    return;
  }

  const glm::vec3 delta = position - staticBox->position;
  const glm::vec3 overlap = (staticBox->halfExtents + halfExtents +
                             glm::vec3(particleRadius)) - glm::abs(delta);
  if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f) {
    return;
  }

  int axis = 0;
  if (overlap.y < overlap[axis]) axis = 1;
  if (overlap.z < overlap[axis]) axis = 2;

  const float sign = delta[axis] >= 0.0f ? 1.0f : -1.0f;
  position[axis] += sign * overlap[axis];
  if (velocity[axis] * sign < 0.0f) {
    velocity[axis] = 0.0f;
  }
  if (axis == 1 && sign > 0.0f) {
    velocity.x *= 0.95f;
    velocity.z *= 0.95f;
    angularVelocity *= 0.90f;
  }
}

void Box::clampToBounds(
    const glm::vec3& minCorner,
    const glm::vec3& maxCorner,
    const float particleRadius) {
  if (!dynamic) {
    return;
  }

  const glm::vec3 minAllowed = minCorner + halfExtents + glm::vec3(
                                   particleRadius);
  const glm::vec3 maxAllowed = maxCorner - halfExtents - glm::vec3(
                                   particleRadius);

  for (int i = 0; i < 3; ++i) {
    if (position[i] < minAllowed[i]) {
      position[i] = minAllowed[i];
      if (velocity[i] < 0.0f) velocity[i] = 0.0f;
      if (i == 1) {
        velocity.x *= 0.95f;
        velocity.z *= 0.95f;
        angularVelocity *= 0.90f;
      }
    } else if (position[i] > maxAllowed[i]) {
      position[i] = maxAllowed[i];
      if (velocity[i] > 0.0f) velocity[i] = 0.0f;
    }
  }
}

void Box::syncBoundaryParticles() {
  if (!dynamic) {
    return;
  }
  assert(boundaryModel != nullptr);

  const Vector3r pos = toSph(position);
  const Quaternionr rotQ = toSph(rotation);
  const Matrix3r rot = rotQ.toRotationMatrix();
  const Vector3r vel = toSph(velocity);
  const Vector3r angVel = toSph(angularVelocity);

  SPH::RigidBodyObject* rbo = boundaryModel->getRigidBodyObject();
  assert(rbo != nullptr);
  rbo->setPosition(pos);
  rbo->setRotation(rotQ);

  const unsigned int count = boundaryModel->numberOfParticles();
  for (unsigned int j = 0; j < count; ++j) {
    const Vector3r worldPos = rot * boundaryModel->getPosition0(j) + pos;
    boundaryModel->setPosition(j, worldPos);
    boundaryModel->setVelocity(j, angVel.cross(worldPos - pos) + vel);
  }
}