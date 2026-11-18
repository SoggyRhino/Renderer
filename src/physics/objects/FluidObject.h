#pragma once

#include <cstddef>
#include <span>

#include <glm/glm.hpp>

#include "graphics/rendering/wrappers/Buffers.h"
#include "physics/objects/PhysicsObject.h"

namespace SPH {
class FluidModel;
class Simulation;
}

class FluidObject final : public PhysicsObject {
public:
  FluidObject(const glm::vec3& minCorner, const glm::vec3& maxCorner);

  void render(
      const Program& program,
      const glm::mat4& parentTransform = glm::mat4(1.0f)) const override;

  void setPointSize(const float size) { pointSize = size; }

  void setXsphViscosity(const float coefficient) {
    xsphViscosity = coefficient;
  }

  void setPreSettle(const bool enabled) { preSettle = enabled; }

  [[nodiscard]] size_t getParticleCount() const {
    return static_cast<size_t>(particleCount);
  }

  [[nodiscard]] glm::vec3 getParticlePosition(size_t index) const;
  [[nodiscard]] std::span<const glm::vec3> getParticlePositions() const;

protected:
  void initPhysics(SPH::Simulation& sim, float particleRadius,
                   float boundarySpacingFactor) override;
  void resetPhysics() override;

private:
  friend class PhysicsEngine;

  void applyVelocityDamping(float factor);
  void saveSettledStateAsInitial();
  void clampToBounds(const glm::vec3& minCorner, const glm::vec3& maxCorner,
                     float particleRadius, float maxSpeed);
  void syncGpuBuffer();

  glm::vec3 minCorner{0.0f};
  glm::vec3 maxCorner{0.0f};
  float xsphViscosity = 0.02f;
  float pointSize = 8.0f;
  bool preSettle = true;

  SPH::FluidModel* fluidModel = nullptr;

  VertexArray particlesVAO;
  Buffer particlesVBO;
  GLsizei particleCount = 0;
  size_t particleBufferCapacityBytes = 0;
};