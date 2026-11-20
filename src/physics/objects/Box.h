#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "physics/objects/PhysicsObject.h"

namespace SPH {
class BoundaryModel_Akinci2012;
class Simulation;
}

class Box final : public PhysicsObject {
public:
  Box(
      SPH::Simulation& sim,
      const glm::vec3& position,
      const glm::vec3& halfExtents,
      float particleRadius,
      float boundarySpacingFactor,
      bool isDynamic = false,
      float density = 1000.0F);

  [[nodiscard]] bool isDynamic() const override { return dynamic; }

  [[nodiscard]] const glm::vec3& getPosition() const { return position; }
  [[nodiscard]] const glm::quat& getRotation() const { return rotation; }
  [[nodiscard]] const glm::vec3& getHalfExtents() const { return halfExtents; }

  void resetPhysics() override;
  void integrate(float stepDt, const glm::vec3& gravity) override;
  void resolveStaticCollision(const PhysicsObject& staticObject,
                              float particleRadius) override;
  void syncBoundaryParticles() override;

private:
  [[nodiscard]] static std::vector<glm::vec3> buildBoundaryParticles(
      const glm::vec3& halfExtents,
      float spacing);
  [[nodiscard]] static SPH::BoundaryModel_Akinci2012& createBoundaryModel(
      SPH::Simulation& sim,
      const glm::vec3& position,
      const glm::quat& rotation,
      const glm::vec3& halfExtents,
      bool isDynamic,
      float particleRadius,
      float boundarySpacingFactor);
  void computeMassAndInertia();

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

  SPH::BoundaryModel_Akinci2012& boundaryModel;
};