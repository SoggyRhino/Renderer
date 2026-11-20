#include "physics/objects/Box.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <ranges>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>

#include "SPlisHSPlasH/BoundaryModel_Akinci2012.h"
#include "SPlisHSPlasH/RigidBodyObject.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/StaticRigidBody.h"

#include "../math/BoundarySampling.h"
#include "../math/SphMath.h"
#include "physics/objects/DynamicRigidBody.h"

Box::Box(
    SPH::Simulation& sim,
    const glm::vec3& initialPos,
    const glm::vec3& halfExtentsIn,
    const float particleRadius,
    const float boundarySpacingFactor,
    const bool isDyn,
    const float bodyDensity)
  : initialPosition(initialPos),
    position(initialPos),
    halfExtents(glm::max(halfExtentsIn, glm::vec3(0.0F))),
    dynamic(isDyn),
    density(bodyDensity),
    boundaryModel(createBoundaryModel(
        sim,
        position,
        rotation,
        halfExtents,
        dynamic,
        particleRadius,
        boundarySpacingFactor)) {
  computeMassAndInertia();
  if (dynamic) {
    syncBoundaryParticles();
  }
}

std::vector<glm::vec3> Box::buildBoundaryParticles(
    const glm::vec3& extents,
    const float spacing) {
  std::vector<glm::vec3> pts;
  const glm::vec3 span = 2.0F * extents;

  appendGridFace(pts, {-extents.x, -extents.y, -extents.z},
                 {1, 0, 0}, {0, 0, 1}, span.x, span.z, spacing);
  appendGridFace(pts, {-extents.x, extents.y, -extents.z},
                 {1, 0, 0}, {0, 0, 1}, span.x, span.z, spacing);

  const float innerY = span.y - 2.0F * spacing;
  if (innerY >= 0.0F) {
    appendGridFace(
        pts, {-extents.x, -extents.y + spacing, -extents.z},
        {1, 0, 0}, {0, 1, 0}, span.x, innerY, spacing);
    appendGridFace(
        pts, {-extents.x, -extents.y + spacing, extents.z},
        {1, 0, 0}, {0, 1, 0}, span.x, innerY, spacing);

    const float innerZ = span.z - 2.0F * spacing;
    if (innerZ >= 0.0F) {
      appendGridFace(
          pts, {-extents.x, -extents.y + spacing, -extents.z + spacing},
          {0, 0, 1}, {0, 1, 0}, innerZ, innerY, spacing);
      appendGridFace(
          pts, {extents.x, -extents.y + spacing, -extents.z + spacing},
          {0, 0, 1}, {0, 1, 0}, innerZ, innerY, spacing);
    }
  }
  return pts;
}

SPH::BoundaryModel_Akinci2012& Box::createBoundaryModel(
    SPH::Simulation& sim,
    const glm::vec3& pos,
    const glm::quat& rot,
    const glm::vec3& extents,
    const bool isDyn,
    const float particleRadius,
    const float boundarySpacingFactor) {
  const auto toBoundaryPoint = [isDyn, &pos, &rot](const glm::vec3& localPoint) {
    return toSph(isDyn ? localPoint : (pos + rot * localPoint));
  };

  auto boundaryPoints =
      buildBoundaryParticles(extents, boundarySpacingFactor * particleRadius) |
      std::views::transform(toBoundaryPoint) |
      std::ranges::to<std::vector>();

  std::unique_ptr<SPH::RigidBodyObject> rigidBody;
  if (isDyn) {
    rigidBody = std::make_unique<DynamicRigidBody>();
  } else {
    rigidBody = std::make_unique<SPH::StaticRigidBody>();
  }

  auto model = std::make_unique<SPH::BoundaryModel_Akinci2012>();
  model->initModel(rigidBody.release(),
                   static_cast<unsigned int>(boundaryPoints.size()),
                   boundaryPoints.data());

  SPH::BoundaryModel_Akinci2012& modelRef = *model;
  sim.addBoundaryModel(model.release());
  return modelRef;
}

void Box::computeMassAndInertia() {
  constexpr float kMinMass = 1e-6F;
  constexpr float kMinInertia = 1e-8F;

  const glm::vec3 size = 2.0F * halfExtents;
  mass = std::max(density * glm::compMul(size), kMinMass);

  const float inertia = (mass * glm::dot(size, size)) / 18.0F;
  invInertia = (inertia > kMinInertia) ? (1.0F / inertia) : 0.0F;
}

void Box::resetPhysics() {
  position = initialPosition;
  rotation = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
  velocity = glm::vec3{};
  angularVelocity = glm::vec3{};

  boundaryModel.clearForceAndTorque();
  if (dynamic) {
    syncBoundaryParticles();
  }
}

void Box::integrate(const float stepDt, const glm::vec3& gravity) {
  if (!dynamic || stepDt <= 0.0F) {
    return;
  }

  Vector3r sphForce;
  Vector3r sphTorque;
  boundaryModel.getForceAndTorque(sphForce, sphTorque);
  boundaryModel.clearForceAndTorque();

  const glm::vec3 fluidForce = toGlm(sphForce);
  const glm::vec3 fluidTorque = toGlm(sphTorque);

  velocity += (gravity + fluidForce / std::max(mass, 1e-6F)) * stepDt;
  angularVelocity = (angularVelocity + fluidTorque * invInertia * stepDt) *
                    0.995F;
  position += velocity * stepDt;

  const glm::quat angVelQ(0.0F, angularVelocity.x, angularVelocity.y,
                          angularVelocity.z);
  rotation = glm::normalize(rotation + 0.5F * stepDt * (angVelQ * rotation));
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
  if (overlap.x <= 0.0F || overlap.y <= 0.0F || overlap.z <= 0.0F) {
    return;
  }

  int axis = 0;
  if (overlap.y < overlap[axis]) axis = 1;
  if (overlap.z < overlap[axis]) axis = 2;

  const float sign = delta[axis] >= 0.0F ? 1.0F : -1.0F;
  position[axis] += sign * overlap[axis];
  if (velocity[axis] * sign < 0.0F) {
    velocity[axis] = 0.0F;
  }
  if (axis == 1 && sign > 0.0F) {
    velocity.x *= 0.95F;
    velocity.z *= 0.95F;
    angularVelocity *= 0.90F;
  }
}

void Box::syncBoundaryParticles() {
  if (!dynamic) {
    return;
  }

  const Vector3r pos = toSph(position);
  const Quaternionr rotQ = toSph(rotation);
  const Matrix3r rot = rotQ.toRotationMatrix();
  const Vector3r vel = toSph(velocity);
  const Vector3r angVel = toSph(angularVelocity);

  SPH::RigidBodyObject* rbo = boundaryModel.getRigidBodyObject();
  assert(rbo != nullptr);
  rbo->setPosition(pos);
  rbo->setRotation(rotQ);

  const unsigned int count = boundaryModel.numberOfParticles();
  for (unsigned int j = 0; j < count; ++j) {
    const Vector3r worldPos = rot * boundaryModel.getPosition0(j) + pos;
    boundaryModel.setPosition(j, worldPos);
    boundaryModel.setVelocity(j, angVel.cross(worldPos - pos) + vel);
  }
}