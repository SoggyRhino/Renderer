#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "graphics/rendering/wrappers/Buffers.h"
#include "physics/objects/PhysicsObject.h"

namespace SPH {
class BoundaryModel_Akinci2012;
class Simulation;
}

class Box final : public PhysicsObject {
public:
  Box(
      const glm::vec3& position,
      const glm::vec3& halfExtents,
      bool isDynamic = false,
      float density = 1000.0F);

  void render(
      const Program& program,
      const glm::mat4& parentTransform = glm::mat4(1.0f)) const override;

  [[nodiscard]] bool isDynamic() const override { return dynamic; }
  [[nodiscard]] bool isCollisionEnabled() const { return collisionEnabled; }

  void setWireframe(const bool enabled) { wireframe = enabled; }
  void setCollisionEnabled(const bool enabled) { collisionEnabled = enabled; }

  [[nodiscard]] const glm::vec3& getPosition() const { return position; }
  [[nodiscard]] const glm::quat& getRotation() const { return rotation; }
  [[nodiscard]] const glm::vec3& getHalfExtents() const { return halfExtents; }

protected:
  void initPhysics(SPH::Simulation& sim, float particleRadius,
                   float boundarySpacingFactor) override;
  void resetPhysics() override;
  void integrate(float stepDt, const glm::vec3& gravity) override;
  void resolveStaticCollision(const PhysicsObject& staticObject,
                              float particleRadius) override;
  void syncBoundaryParticles() override;

private:
  friend class PhysicsEngine;

  void rebuildMesh();
  [[nodiscard]] std::vector<glm::vec3> buildBoundaryParticles(
      float spacing) const;
  void computeMassAndInertia();
  void clearBoundaryForceAndTorque() const;
  void clampToBounds(const glm::vec3& minCorner, const glm::vec3& maxCorner,
                     float particleRadius);

  glm::vec3 initialPosition{0.0F};
  glm::vec3 position{0.0F};
  glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F};
  glm::vec3 velocity{0.0F};
  glm::vec3 angularVelocity{0.0F};
  glm::vec3 halfExtents{0.5F};

  bool dynamic = false;
  float density = 1000.0F;
  float mass = 1.0F;
  float invInertia = 0.0F;

  bool collisionEnabled = true;
  bool wireframe = false;

  SPH::BoundaryModel_Akinci2012* boundaryModel = nullptr;

  VertexArray solidVAO;
  VertexArray wireVAO;
  Buffer vbo;
  Buffer solidEBO;
  Buffer wireEBO;
  GLsizei solidIndexCount = 0;
  GLsizei wireIndexCount = 0;
};