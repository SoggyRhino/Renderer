#pragma once

#include <cstddef>
#include <span>

#include <glm/glm.hpp>

#include "physics/objects/PhysicsObject.h"

namespace SPH {
class FluidModel;
class Simulation;
}

class FluidObject final : public PhysicsObject {
public:
  FluidObject(
      SPH::Simulation& sim,
      const glm::vec3& minCorner,
      const glm::vec3& maxCorner,
      float particleRadius,
      float xsphViscosity = 0.02F);

  void resetPhysics() override {
  }

  [[nodiscard]] std::size_t getParticleCount() const;
  [[nodiscard]] glm::vec3 getParticlePosition(std::size_t index) const;
  [[nodiscard]] std::span<const glm::vec3> getParticlePositions() const;

private:
  [[nodiscard]] static SPH::FluidModel& createFluidModel(
      SPH::Simulation& sim,
      const glm::vec3& minCorner,
      const glm::vec3& maxCorner,
      float particleRadius,
      float xsphViscosity);

  glm::vec3 minCorner{0.0F};
  glm::vec3 maxCorner{0.0F};
  SPH::FluidModel& fluidModel;
};