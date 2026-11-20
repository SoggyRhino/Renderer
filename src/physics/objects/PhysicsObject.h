#pragma once

#include <glm/glm.hpp>

class PhysicsObject {
public:
  virtual ~PhysicsObject() = default;

  PhysicsObject(const PhysicsObject&) = delete;
  PhysicsObject& operator=(const PhysicsObject&) = delete;
  PhysicsObject(PhysicsObject&&) = delete;
  PhysicsObject& operator=(PhysicsObject&&) = delete;

  [[nodiscard]] virtual bool isDynamic() const { return false; }

  virtual void resetPhysics() = 0;

  virtual void integrate(float /*stepDt*/, const glm::vec3& /*gravity*/) {
  }

  virtual void resolveStaticCollision(const PhysicsObject& /*staticObject*/,
                                      float /*particleRadius*/) {
  }

  virtual void syncBoundaryParticles() {
  }

protected:
  PhysicsObject() = default;
};