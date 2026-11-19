#pragma once

#include <glm/glm.hpp>

class PhysicsEngine;
class Program;

namespace SPH {
class Simulation;
}

class PhysicsObject {
public:
  virtual ~PhysicsObject() = default;

  PhysicsObject(const PhysicsObject&) = delete;
  PhysicsObject& operator=(const PhysicsObject&) = delete;
  PhysicsObject(PhysicsObject&&) = delete;
  PhysicsObject& operator=(PhysicsObject&&) = delete;

  virtual void render(
      const Program& program,
      const glm::mat4& parentTransform = glm::mat4(1.0f)) const = 0;

  //todo redo how rendering is handled
  void setVisible(const bool isVisible) { visible = isVisible; }
  [[nodiscard]] bool isVisible() const { return visible; }

  void setColor(const glm::vec3& newColor) { color = newColor; }
  [[nodiscard]] const glm::vec3& getColor() const { return color; }

  [[nodiscard]] virtual bool isDynamic() const { return false; }

protected:
  PhysicsObject() = default;

  friend class PhysicsEngine;

  virtual void initPhysics(SPH::Simulation& sim, float particleRadius,
                           float boundarySpacingFactor) = 0;
  virtual void resetPhysics() = 0;

  virtual void integrate(float /*stepDt*/, const glm::vec3& /*gravity*/) {
  }

  virtual void resolveStaticCollision(const PhysicsObject& /*staticObject*/,
                                      float /*particleRadius*/) {
  }

  virtual void syncBoundaryParticles() {
  }

  glm::vec3 color{1.0f, 1.0f, 1.0f};
  bool visible = true;
};