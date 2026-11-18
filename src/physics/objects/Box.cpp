#include "physics/objects/Box.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "SPlisHSPlasH/BoundaryModel_Akinci2012.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/StaticRigidBody.h"

#include "graphics/rendering/wrappers/Program.h"

namespace {
Vector3r toSph(const glm::vec3& v) { return {v.x, v.y, v.z}; }

glm::vec3 toGlm(const Vector3r& v) {
  return {float(v.x()), float(v.y()), float(v.z())};
}

Quaternionr toSph(const glm::quat& q) { return {q.w, q.x, q.y, q.z}; }

class DynamicRigidBody final : public SPH::StaticRigidBody {
public:
  bool isDynamic() const override { return true; }
};

void appendGridFace(
    std::vector<glm::vec3>& out,
    const glm::vec3& origin,
    const glm::vec3& uDir,
    const glm::vec3& vDir,
    const float uLen,
    const float vLen,
    const float spacing) {
  const int nu = std::max(1, static_cast<int>(uLen / spacing) + 1);
  const int nv = std::max(1, static_cast<int>(vLen / spacing) + 1);
  for (int iu = 0; iu < nu; ++iu) {
    const float u = (nu == 1)
                      ? 0.0f
                      : uLen * (static_cast<float>(iu) / static_cast<float>(
                                  nu - 1));
    for (int iv = 0; iv < nv; ++iv) {
      const float v = (nv == 1)
                        ? 0.0f
                        : vLen * (static_cast<float>(iv) / static_cast<float>(
                                    nv - 1));
      out.push_back(origin + uDir * u + vDir * v);
    }
  }
}
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

Box Box::fromMinMax(
    const glm::vec3& minCorner,
    const glm::vec3& maxCorner,
    const bool isDynamic,
    const float density) {
  const glm::vec3 mn = glm::min(minCorner, maxCorner);
  const glm::vec3 mx = glm::max(minCorner, maxCorner);
  return Box(0.5f * (mn + mx), 0.5f * (mx - mn), isDynamic, density);
}

void Box::rebuildMesh() {
  const float hx = halfExtents.x;
  const float hy = halfExtents.y;
  const float hz = halfExtents.z;

  const std::vector<glm::vec3> vertices = {
      {-hx, -hy, -hz}, {hx, -hy, -hz}, {hx, -hy, hz}, {-hx, -hy, hz},
      {-hx, hy, -hz}, {hx, hy, -hz}, {hx, hy, hz}, {-hx, hy, hz},
  };
  const std::vector<uint32_t> triIndices = {
      0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
      0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2,
      0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5,
  };
  const std::vector<uint32_t> lineIndices = {
      0, 1, 1, 2, 2, 3, 3, 0,
      4, 5, 5, 6, 6, 7, 7, 4,
      0, 4, 1, 5, 2, 6, 3, 7,
  };

  vbo.setData(
      GL_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3)),
      vertices.data(),
      GL_STATIC_DRAW);

  solidVAO.bind();
  vbo.bind(GL_ARRAY_BUFFER);
  solidEBO.setData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(triIndices.size() * sizeof(uint32_t)),
      triIndices.data(),
      GL_STATIC_DRAW);
  solidVAO.setAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                               nullptr);
  solidVAO.unbind();
  solidIndexCount = static_cast<GLsizei>(triIndices.size());

  wireVAO.bind();
  vbo.bind(GL_ARRAY_BUFFER);
  wireEBO.setData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(lineIndices.size() * sizeof(uint32_t)),
      lineIndices.data(),
      GL_STATIC_DRAW);
  wireVAO.setAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                              nullptr);
  wireVAO.unbind();
  wireIndexCount = static_cast<GLsizei>(lineIndices.size());
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

  if (wireframe) {
    wireVAO.drawElements(GL_LINES, wireIndexCount, GL_UNSIGNED_INT);
  } else {
    solidVAO.drawElements(GL_TRIANGLES, solidIndexCount, GL_UNSIGNED_INT);
  }
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
  const glm::vec3 size = 2.0f * halfExtents;
  mass = std::max(density * size.x * size.y * size.z, 1e-6f);
  const float avgSizeSq = (size.x * size.x + size.y * size.y + size.z * size.z)
                          / 3.0f;
  const float inertia = (mass * avgSizeSq) / 6.0f;
  invInertia = inertia > 1e-8f ? 1.0f / inertia : 0.0f;
}

void Box::initPhysics(
    SPH::Simulation& sim,
    const float particleRadius,
    const float boundarySpacingFactor) {
  computeMassAndInertia();
  boundaryModel = nullptr;
  if (!collisionEnabled) {
    return;
  }

  const std::vector<glm::vec3> localPts = buildBoundaryParticles(
      boundarySpacingFactor * particleRadius);
  std::vector<Vector3r> sphPts;
  sphPts.reserve(localPts.size());
  for (const glm::vec3& p : localPts) {
    sphPts.push_back(toSph(dynamic ? p : (position + rotation * p)));
  }

  SPH::RigidBodyObject* body = dynamic
                                 ? static_cast<SPH::RigidBodyObject*>(new
                                   DynamicRigidBody())
                                 : static_cast<SPH::RigidBodyObject*>(new
                                   SPH::StaticRigidBody());

  auto* model = new SPH::BoundaryModel_Akinci2012();
  model->initModel(body, static_cast<unsigned int>(sphPts.size()),
                   sphPts.data());
  sim.addBoundaryModel(model);
  boundaryModel = model;

  if (dynamic) {
    syncBoundaryParticles();
  }
}

void Box::resetPhysics() {
  position = initialPosition;
  rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  velocity = glm::vec3(0.0f);
  angularVelocity = glm::vec3(0.0f);

  if (boundaryModel != nullptr) {
    boundaryModel->clearForceAndTorque();
    if (dynamic) {
      syncBoundaryParticles();
    }
  }
}

void Box::clearBoundaryForceAndTorque() const {
  if (boundaryModel != nullptr) {
    boundaryModel->clearForceAndTorque();
  }
}

void Box::integrate(const float stepDt, const glm::vec3& gravity) {
  if (!dynamic || stepDt <= 0.0f) {
    return;
  }

  glm::vec3 fluidForce(0.0f);
  glm::vec3 fluidTorque(0.0f);
  if (boundaryModel != nullptr) {
    Vector3r sphForce, sphTorque;
    boundaryModel->getForceAndTorque(sphForce, sphTorque);
    boundaryModel->clearForceAndTorque();
    fluidForce = toGlm(sphForce);
    fluidTorque = toGlm(sphTorque);
  }

  velocity += (gravity + fluidForce / std::max(mass, 1e-6f)) * stepDt;
  angularVelocity = (angularVelocity + fluidTorque * invInertia * stepDt) *
                    0.995f;
  position += velocity * stepDt;

  const glm::quat angVelQ(0.0f, angularVelocity.x, angularVelocity.y,
                          angularVelocity.z);
  rotation = glm::normalize(rotation + 0.5f * stepDt * (angVelQ * rotation));
}

void Box::resolveStaticCollision(const Box& staticBox,
                                 const float particleRadius) {
  if (!dynamic || !staticBox.isCollisionEnabled()) {
    return;
  }

  const glm::vec3 delta = position - staticBox.position;
  const glm::vec3 overlap = (staticBox.halfExtents + halfExtents +
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

void Box::syncBoundaryParticles() const {
  if (boundaryModel == nullptr || !dynamic) {
    return;
  }

  const Vector3r pos = toSph(position);
  const Quaternionr rotQ = toSph(rotation);
  const Matrix3r rot = rotQ.toRotationMatrix();
  const Vector3r vel = toSph(velocity);
  const Vector3r angVel = toSph(angularVelocity);

  if (SPH::RigidBodyObject* rbo = boundaryModel->getRigidBodyObject()) {
    rbo->setPosition(pos);
    rbo->setRotation(rotQ);
  }

  const unsigned int count = boundaryModel->numberOfParticles();
  for (unsigned int j = 0; j < count; ++j) {
    const Vector3r worldPos = rot * boundaryModel->getPosition0(j) + pos;
    boundaryModel->setPosition(j, worldPos);
    boundaryModel->setVelocity(j, angVel.cross(worldPos - pos) + vel);
  }
}