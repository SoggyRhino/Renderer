#include "physics/objects/FluidObject.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "SPlisHSPlasH/FluidModel.h"
#include "SPlisHSPlasH/NeighborhoodSearch.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/XSPH.h"

static_assert(std::is_same_v<Real, float> &&
              sizeof(Vector3r) == sizeof(glm::vec3) &&
              alignof(Vector3r) == alignof(glm::vec3),
              "Vector3r and glm::vec3 must be layout-compatible for zero-copy span")
;

FluidObject::FluidObject(
    SPH::Simulation& sim,
    const glm::vec3& minCornerIn,
    const glm::vec3& maxCornerIn,
    const float particleRadius,
    const float xsphViscosity,
    const bool shouldPresettle)
  : minCorner(glm::min(minCornerIn, maxCornerIn)),
    maxCorner(glm::max(minCornerIn, maxCornerIn)),
    presettle(shouldPresettle),
    fluidModel(createFluidModel(
        sim, minCorner, maxCorner, particleRadius, xsphViscosity)) {
}

void FluidObject::setActive(const bool isActive) {
  if (active == isActive) {
    return;
  }
  active = isActive;
  syncActiveState();
}

void FluidObject::syncActiveState() {
  if (!SPH::Simulation::hasCurrent()) {
    return;
  }
  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  if (!sim->isSimulationInitialized() || sim->getNeighborhoodSearch() == nullptr) {
    return;
  }
  fluidModel.setNumActiveParticles(
      active ? fluidModel.getNumActiveParticles0() : 0U);
  sim->getNeighborhoodSearch()->setNeighborhoodSearchActive(
      fluidModel.getPointSetIndex(), active, active);
}

void FluidObject::resetPhysics() {
  syncActiveState();
}

void FluidObject::applyVelocityDamping(const float factor) const {
  const unsigned int count = fluidModel.numActiveParticles();
  for (unsigned int i = 0; i < count; ++i) {
    fluidModel.getVelocity(i) *= static_cast<Real>(factor);
  }
}

void FluidObject::saveSettledStateAsInitial() const {
  const unsigned int count = fluidModel.numActiveParticles();
  for (unsigned int i = 0; i < count; ++i) {
    fluidModel.setPosition0(i, fluidModel.getPosition(i));
    fluidModel.setVelocity0(i, Vector3r::Zero());
    fluidModel.setVelocity(i, Vector3r::Zero());
    fluidModel.getAcceleration(i).setZero();
  }
}

void FluidObject::enforceLimits(
    const float maxSpeed,
    const std::optional<PhysicsBounds>& worldBounds) const {
  constexpr float kSafetyExtent = 100.0F;
  const glm::vec3 safetyMin(-kSafetyExtent);
  const glm::vec3 safetyMax(kSafetyExtent);

  const glm::vec3 minBound = worldBounds
      ? glm::clamp(glm::min(worldBounds->min, worldBounds->max),
                   safetyMin, safetyMax)
      : safetyMin;
  const glm::vec3 maxBound = worldBounds
      ? glm::clamp(glm::max(worldBounds->min, worldBounds->max),
                   safetyMin, safetyMax)
      : safetyMax;

  const bool hasSpeedLimit = std::isfinite(maxSpeed) && maxSpeed > 0.0F;
  const Real maxSpeedSq = hasSpeedLimit ? maxSpeed * maxSpeed : 0.0F;

  const unsigned int count = fluidModel.numActiveParticles();
  for (unsigned int i = 0; i < count; ++i) {
    Vector3r& pos = fluidModel.getPosition(i);
    Vector3r& vel = fluidModel.getVelocity(i);

    const bool finitePos = std::isfinite(pos.x()) && std::isfinite(pos.y()) &&
                           std::isfinite(pos.z());
    const bool finiteVel = std::isfinite(vel.x()) && std::isfinite(vel.y()) &&
                           std::isfinite(vel.z());
    if (!finitePos || !finiteVel) {
      pos = fluidModel.getPosition0(i);
      vel.setZero();
    }

    for (int axis = 0; axis < 3; ++axis) {
      if (pos[axis] < minBound[axis]) {
        pos[axis] = minBound[axis];
        if (vel[axis] < 0.0F) {
          vel[axis] = 0.0F;
        }
      } else if (pos[axis] > maxBound[axis]) {
        pos[axis] = maxBound[axis];
        if (vel[axis] > 0.0F) {
          vel[axis] = 0.0F;
        }
      }
    }

    if (hasSpeedLimit) {
      if (const Real speedSq = vel.squaredNorm(); speedSq > maxSpeedSq) {
        vel *= maxSpeed / std::sqrt(speedSq);
      }
    }
  }
}

std::size_t FluidObject::getParticleCount() const {
  return static_cast<std::size_t>(fluidModel.numActiveParticles());
}

glm::vec3 FluidObject::getParticlePosition(const std::size_t index) const {
  if (index >= fluidModel.numActiveParticles()) {
    return glm::vec3(0.0F);
  }
  const Vector3r& p = fluidModel.getPosition(static_cast<unsigned int>(index));
  return {p.x(), p.y(), p.z()};
}

std::span<const glm::vec3> FluidObject::getParticlePositions() const {
  const unsigned int count = fluidModel.numActiveParticles();
  if (count == 0) {
    return {};
  }
  return {
      reinterpret_cast<const glm::vec3*>(fluidModel.getPosition(0).data()),
      count
  };
}

SPH::FluidModel& FluidObject::createFluidModel(
    SPH::Simulation& sim,
    const glm::vec3& minCorner,
    const glm::vec3& maxCorner,
    const float particleRadius,
    const float xsphViscosity) {
  const float diam = 2.0F * particleRadius;
  const glm::vec3 span = glm::max(maxCorner - minCorner, glm::vec3(0.0F));
  const int stepsX = std::max(1, static_cast<int>(span.x / diam) + 1);
  const int stepsY = std::max(1, static_cast<int>(span.y / diam) + 1);
  const int stepsZ = std::max(1, static_cast<int>(span.z / diam) + 1);

  const glm::vec3 center = 0.5F * (minCorner + maxCorner);
  const float startX = center.x - 0.5F * static_cast<float>(stepsX - 1) * diam;
  const float startZ = center.z - 0.5F * static_cast<float>(stepsZ - 1) * diam;

  std::vector<Vector3r> sphPositions;
  sphPositions.reserve(static_cast<std::size_t>(stepsX) * stepsY * stepsZ);
  for (int ix = 0; ix < stepsX; ++ix) {
    const float x = startX + static_cast<float>(ix) * diam;
    for (int iy = 0; iy < stepsY; ++iy) {
      const float y = minCorner.y + static_cast<float>(iy) * diam;
      for (int iz = 0; iz < stepsZ; ++iz) {
        const float z = startZ + static_cast<float>(iz) * diam;
        sphPositions.emplace_back(x, y, z);
      }
    }
  }

  const auto nParticles = static_cast<unsigned int>(sphPositions.size());
  std::vector<Vector3r> sphVelocities(nParticles, Vector3r::Zero());
  std::vector<unsigned int> sphObjectIds(nParticles, 0);

  const unsigned int index = sim.numberOfFluidModels();
  sim.addFluidModel("Water", nParticles, sphPositions.data(),
                    sphVelocities.data(), sphObjectIds.data(), 0);

  SPH::FluidModel* model = sim.getFluidModel(index);
  if (model == nullptr) {
    throw std::runtime_error("Failed to create SPH::FluidModel");
  }

  model->setViscosityMethod(0);
  if (SPH::XSPH* xsph = model->getXSPH()) {
    xsph->setValue(SPH::XSPH::FLUID_COEFFICIENT, xsphViscosity);
  }
  return *model;
}