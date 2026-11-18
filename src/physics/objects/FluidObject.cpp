#include "physics/objects/FluidObject.h"

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <vector>

#include "SPlisHSPlasH/FluidModel.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/XSPH.h"

#include "graphics/rendering/wrappers/Program.h"

static_assert(std::is_same_v<Real, float> &&
              sizeof(Vector3r) == sizeof(glm::vec3) &&
              alignof(Vector3r) == alignof(glm::vec3),
              "Vector3r and glm::vec3 must be layout-compatible for zero-copy upload/span")
;

FluidObject::FluidObject(const glm::vec3& minCornerIn,
                         const glm::vec3& maxCornerIn)
  : minCorner(glm::min(minCornerIn, maxCornerIn)),
    maxCorner(glm::max(minCornerIn, maxCornerIn)) {
  color = glm::vec3(0.2f, 0.65f, 1.0f);

  particlesVAO.bind();
  particlesVBO.setData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
  particlesVAO.setAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                                   nullptr);
  particlesVAO.unbind();
}

glm::vec3 FluidObject::getParticlePosition(const size_t index) const {
  if (fluidModel == nullptr || index >= fluidModel->numActiveParticles()) {
    return glm::vec3(0.0f);
  }
  const Vector3r& p = fluidModel->getPosition(static_cast<unsigned int>(index));
  return {p.x(), p.y(), p.z()};
}

std::span<const glm::vec3> FluidObject::getParticlePositions() const {
  if (fluidModel == nullptr || fluidModel->numActiveParticles() == 0) {
    return {};
  }
  return {
      reinterpret_cast<const glm::vec3*>(fluidModel->getPosition(0).data()),
      fluidModel->numActiveParticles()
  };
}

void FluidObject::initPhysics(
    SPH::Simulation& sim,
    const float particleRadius,
    const float /*boundarySpacingFactor*/) {
  fluidModel = nullptr;

  const float diam = 2.0f * particleRadius;
  const glm::vec3 span = glm::max(maxCorner - minCorner, glm::vec3(0.0f));
  const int stepsX = std::max(1, static_cast<int>(span.x / diam) + 1);
  const int stepsY = std::max(1, static_cast<int>(span.y / diam) + 1);
  const int stepsZ = std::max(1, static_cast<int>(span.z / diam) + 1);

  const glm::vec3 center = 0.5f * (minCorner + maxCorner);
  const float startX = center.x - 0.5f * static_cast<float>(stepsX - 1) * diam;
  const float startZ = center.z - 0.5f * static_cast<float>(stepsZ - 1) * diam;

  std::vector<Vector3r> sphPositions;
  sphPositions.reserve(static_cast<size_t>(stepsX) * stepsY * stepsZ);
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

  fluidModel = sim.getFluidModel(index);
  if (fluidModel != nullptr) {
    fluidModel->setViscosityMethod(0);
    if (SPH::XSPH* xsph = fluidModel->getXSPH()) {
      xsph->setValue(SPH::XSPH::FLUID_COEFFICIENT, xsphViscosity);
    }
  }
}

void FluidObject::resetPhysics() {
  syncGpuBuffer();
}

void FluidObject::applyVelocityDamping(const float factor) {
  if (fluidModel == nullptr) {
    return;
  }
  for (unsigned int i = 0; i < fluidModel->numActiveParticles(); ++i) {
    fluidModel->getVelocity(i) *= static_cast<Real>(factor);
  }
}

void FluidObject::saveSettledStateAsInitial() {
  if (fluidModel == nullptr) {
    return;
  }
  for (unsigned int i = 0; i < fluidModel->numActiveParticles(); ++i) {
    fluidModel->getPosition0(i) = fluidModel->getPosition(i);
    fluidModel->getVelocity0(i).setZero();
    fluidModel->getVelocity(i).setZero();
  }
}

void FluidObject::clampToBounds(
    const glm::vec3& boundsMin,
    const glm::vec3& boundsMax,
    const float particleRadius,
    const float maxSpeed) {
  if (fluidModel == nullptr) {
    return;
  }

  const Real minX = boundsMin.x + particleRadius;
  const Real maxX = boundsMax.x - particleRadius;
  const Real minY = boundsMin.y + particleRadius;
  const Real maxY = boundsMax.y - particleRadius;
  const Real minZ = boundsMin.z + particleRadius;
  const Real maxZ = boundsMax.z - particleRadius;
  const Real maxSpeed2 = maxSpeed * maxSpeed;
  const int count = static_cast<int>(fluidModel->numActiveParticles());

#pragma omp parallel for schedule(static)
  for (int i = 0; i < count; ++i) {
    Vector3r& x = fluidModel->getPosition(i);
    Vector3r& v = fluidModel->getVelocity(i);

    if (x.x() < minX) {
      x.x() = minX;
      if (v.x() < 0) v.x() = 0;
    } else if (x.x() > maxX) {
      x.x() = maxX;
      if (v.x() > 0) v.x() = 0;
    }

    if (x.y() < minY) {
      x.y() = minY;
      if (v.y() < 0) v.y() = 0;
    } else if (x.y() > maxY) {
      x.y() = maxY;
      if (v.y() > 0) v.y() = 0;
    }

    if (x.z() < minZ) {
      x.z() = minZ;
      if (v.z() < 0) v.z() = 0;
    } else if (x.z() > maxZ) {
      x.z() = maxZ;
      if (v.z() > 0) v.z() = 0;
    }

    const Real speed2 = v.squaredNorm();
    if (speed2 > maxSpeed2) {
      v *= static_cast<Real>(maxSpeed) / std::sqrt(speed2);
    }
  }
}

void FluidObject::syncGpuBuffer() {
  if (fluidModel == nullptr) {
    particleCount = 0;
    return;
  }

  particleCount = static_cast<GLsizei>(fluidModel->numActiveParticles());
  if (particlesVBO.id() != 0 && particleCount > 0) {
    const size_t byteSize = static_cast<size_t>(particleCount) *
                            sizeof(Vector3r);
    const void* data = fluidModel->getPosition(0).data();
    if (byteSize <= particleBufferCapacityBytes) {
      particlesVBO.bind(GL_ARRAY_BUFFER);
      glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(byteSize),
                      data);
    } else {
      particleBufferCapacityBytes = byteSize;
      particlesVBO.setData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(byteSize),
                           data, GL_DYNAMIC_DRAW);
    }
  }
}

void FluidObject::render(const Program& program,
                         const glm::mat4& parentTransform) const {
  if (!visible || particleCount <= 0) {
    return;
  }

  program.setMatx4fv("model", parentTransform);
  program.set3fv("uColor", color);
  program.setFloat("uPointSize", pointSize);
  particlesVAO.drawArrays(GL_POINTS, 0, particleCount);
}