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

  virtual void render(
      const Program& program,
      const glm::mat4& parentTransform = glm::mat4(1.0f)) const = 0;

  void setVisible(const bool isVisible) { visible = isVisible; }
  [[nodiscard]] bool isVisible() const { return visible; }

  void setColor(const glm::vec3& newColor) { color = newColor; }
  [[nodiscard]] const glm::vec3& getColor() const { return color; }

protected:
  PhysicsObject() = default;

  friend class PhysicsEngine;

  virtual void initPhysics(SPH::Simulation& sim, float particleRadius,
                           float boundarySpacingFactor) = 0;
  virtual void resetPhysics() = 0;

  glm::vec3 color{1.0f, 1.0f, 1.0f};
  bool visible = true;
};