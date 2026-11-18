#include "physics/PhysicsEngine.h"

#include <algorithm>
#include <chrono>

#include "SPlisHSPlasH/BoundaryModel_Akinci2012.h"
#include "SPlisHSPlasH/DFSPH/TimeStepDFSPH.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/StaticRigidBody.h"
#include "SPlisHSPlasH/TimeManager.h"
#include "SPlisHSPlasH/TimeStep.h"
#include "Utilities/Counting.h"
#include "Utilities/Logger.h"
#include "Utilities/Timing.h"

#include "physics/objects/Box.h"
#include "physics/objects/FluidObject.h"
#include "physics/objects/PhysicsObject.h"

INIT_LOGGING
INIT_TIMING
INIT_COUNTING

namespace {
void appendBoundaryFace(
    std::vector<Vector3r>& out,
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
      const glm::vec3 p = origin + uDir * u + vDir * v;
      out.emplace_back(p.x, p.y, p.z);
    }
  }
}
} // namespace

PhysicsEngine::PhysicsEngine(const PhysicsConfig& engineConfig) : config(
    engineConfig) {
}

PhysicsEngine::~PhysicsEngine() {
  if (SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }
}

void PhysicsEngine::registerObject(PhysicsObject& object) {
  if (auto* fluid = dynamic_cast<FluidObject*>(&object)) {
    fluids.push_back(fluid);
  } else if (auto* box = dynamic_cast<Box*>(&object)) {
    if (box->isDynamic()) {
      dynamicBoxes.push_back(box);
    } else {
      staticBoxes.push_back(box);
    }
  }
  initialized = false;
}

void PhysicsEngine::setWorldBounds(const glm::vec3& minCorner,
                                   const glm::vec3& maxCorner) {
  hasWorldBounds = true;
  worldBoundsMin = glm::min(minCorner, maxCorner);
  worldBoundsMax = glm::max(minCorner, maxCorner);
  initialized = false;
}

void PhysicsEngine::initWorldBoundsBoundary(SPH::Simulation& sim) const {
  if (!hasWorldBounds) {
    return;
  }

  const float spacing = config.boundarySpacingFactor * config.particleRadius;
  const glm::vec3 span = worldBoundsMax - worldBoundsMin;
  std::vector<Vector3r> pts;

  // Floor
  appendBoundaryFace(pts, worldBoundsMin, {1, 0, 0}, {0, 0, 1}, span.x, span.z,
                     spacing);

  // Side walls (open top)
  const float wallY = span.y - spacing;
  if (wallY >= 0.0f) {
    appendBoundaryFace(
        pts,
        {worldBoundsMin.x, worldBoundsMin.y + spacing, worldBoundsMin.z},
        {1, 0, 0}, {0, 1, 0}, span.x, wallY, spacing);
    appendBoundaryFace(
        pts,
        {worldBoundsMin.x, worldBoundsMin.y + spacing, worldBoundsMax.z},
        {1, 0, 0}, {0, 1, 0}, span.x, wallY, spacing);

    const float innerZ = span.z - 2.0f * spacing;
    if (innerZ >= 0.0f) {
      appendBoundaryFace(
          pts,
          {worldBoundsMin.x, worldBoundsMin.y + spacing,
           worldBoundsMin.z + spacing},
          {0, 0, 1}, {0, 1, 0}, innerZ, wallY, spacing);
      appendBoundaryFace(
          pts,
          {worldBoundsMax.x, worldBoundsMin.y + spacing,
           worldBoundsMin.z + spacing},
          {0, 0, 1}, {0, 1, 0}, innerZ, wallY, spacing);
    }
  }

  auto* model = new SPH::BoundaryModel_Akinci2012();
  model->initModel(new SPH::StaticRigidBody(),
                   static_cast<unsigned int>(pts.size()), pts.data());
  sim.addBoundaryModel(model);
}

void PhysicsEngine::init() {
  if (SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  sim->init(config.particleRadius, false);
  sim->setBoundaryHandlingMethod(SPH::BoundaryHandlingMethods::Akinci2012);
  sim->setSimulationMethod(static_cast<int>(SPH::SimulationMethods::DFSPH));
  sim->setValue(SPH::Simulation::CFL_METHOD, 0);
  sim->setValue(SPH::Simulation::STEPS_PER_Z_SORT, 25u);

  Vector3r grav(config.gravity.x, config.gravity.y, config.gravity.z);
  sim->setVecValue<Real>(SPH::Simulation::GRAVITATION, grav.data());
  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);

  if (SPH::TimeStep* timeStep = sim->getTimeStep()) {
    timeStep->setValue(SPH::TimeStepDFSPH::USE_DIVERGENCE_SOLVER, false);
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ERROR, 1.5f);
    timeStep->setValue(SPH::TimeStepDFSPH::MIN_ITERATIONS, 1u);
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ITERATIONS, 4u);
  }

  for (FluidObject* fluid : fluids) {
    fluid->initPhysics(*sim, config.particleRadius,
                       config.boundarySpacingFactor);
  }
  // Static boundaries must be registered before dynamic ones for Akinci2012 volume init
  initWorldBoundsBoundary(*sim);
  for (Box* box : staticBoxes) {
    box->initPhysics(*sim, config.particleRadius,
                     config.boundarySpacingFactor);
  }
  for (Box* box : dynamicBoxes) {
    box->initPhysics(*sim, config.particleRadius,
                     config.boundarySpacingFactor);
  }

  if (SPH::TimeStep* timeStep = sim->getTimeStep()) {
    timeStep->resize();
  }
  sim->setSimulationInitialized(true);
  for (unsigned int i = 0; i < sim->numberOfBoundaryModels(); ++i) {
    sim->getBoundaryModel(i)->deferredInit();
  }
  sim->performNeighborhoodSearchSort();
  sim->updateBoundaryVolume();

  preSettleFluids();
  for (FluidObject* fluid : fluids) {
    fluid->syncGpuBuffer();
  }

  simAccumulator = 0.0;
  lastSimSpeedRatio = 0.0f;
  lastStepTimeMs = 0.0f;
  lastSubsteps = 0;
  initialized = true;
}

void PhysicsEngine::preSettleFluids() {
  if (config.preSettleSteps <= 0 || fluids.empty()) {
    return;
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  SPH::TimeStep* timeStep = sim->getTimeStep();
  if (timeStep == nullptr) {
    return;
  }

  for (int s = 0; s < config.preSettleSteps; ++s) {
    timeStep->step();
    for (FluidObject* fluid : fluids) {
      if (hasWorldBounds) {
        fluid->clampToBounds(worldBoundsMin, worldBoundsMax,
                             config.particleRadius, config.maxParticleSpeed);
      }
      if (fluid->preSettle) {
        fluid->applyVelocityDamping(0.85f);
      }
    }
  }

  for (FluidObject* fluid : fluids) {
    if (fluid->preSettle) {
      fluid->saveSettledStateAsInitial();
    }
  }
  for (Box* box : dynamicBoxes) {
    box->clearBoundaryForceAndTorque();
  }

  sim->reset();
  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);
}

void PhysicsEngine::step(const double dt) {
  if (paused) {
    simAccumulator = 0.0;
    return;
  }
  if (!initialized) {
    init();
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  SPH::TimeStep* timeStep = sim->getTimeStep();
  if (timeStep == nullptr) {
    return;
  }

  constexpr double kMaxFrameDt = 0.05;
  constexpr int kMaxSubstepsPerFrame = 10;

  const double scaledDt = std::min(dt, kMaxFrameDt) * static_cast<double>(
                            timeScale);
  simAccumulator = std::min(simAccumulator + scaledDt,
                            kMaxFrameDt * static_cast<double>(timeScale));

  SPH::TimeManager* tm = SPH::TimeManager::getCurrent();
  const Real simTimeBefore = tm->getTime();
  const auto wallStart = std::chrono::steady_clock::now();

  int substeps = 0;
  while (simAccumulator > 0.0 && substeps < kMaxSubstepsPerFrame) {
    for (Box* box : dynamicBoxes) {
      box->clearBoundaryForceAndTorque();
    }

    const Real prevTime = tm->getTime();
    timeStep->step();

    if (hasWorldBounds) {
      for (FluidObject* fluid : fluids) {
        fluid->clampToBounds(worldBoundsMin, worldBoundsMax,
                             config.particleRadius, config.maxParticleSpeed);
      }
    }

    const float stepDt = static_cast<float>(tm->getTime() - prevTime);
    for (Box* box : dynamicBoxes) {
      box->integrate(stepDt, config.gravity);
      for (const Box* staticBox : staticBoxes) {
        box->resolveStaticCollision(*staticBox, config.particleRadius);
      }
      if (hasWorldBounds) {
        box->clampToBounds(worldBoundsMin, worldBoundsMax,
                           config.particleRadius);
      }
      box->syncBoundaryParticles();
    }

    simAccumulator -= static_cast<double>(stepDt);
    ++substeps;
  }

  if (substeps > 0) {
    for (FluidObject* fluid : fluids) {
      fluid->syncGpuBuffer();
    }
    const auto wallEnd = std::chrono::steady_clock::now();
    lastStepTimeMs = std::chrono::duration<float, std::milli>(
        wallEnd - wallStart).count();
    lastSubsteps = substeps;
    if (dt > 0.0) {
      lastSimSpeedRatio = static_cast<float>(tm->getTime() - simTimeBefore) /
                          static_cast<float>(dt);
    }
  }
}

void PhysicsEngine::reset() {
  simAccumulator = 0.0;
  lastSimSpeedRatio = 0.0f;
  lastStepTimeMs = 0.0f;
  lastSubsteps = 0;

  if (!initialized) {
    init();
    return;
  }

  for (Box* box : staticBoxes) {
    box->resetPhysics();
  }
  for (Box* box : dynamicBoxes) {
    box->resetPhysics();
  }

  SPH::Simulation::getCurrent()->reset();
  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);

  for (FluidObject* fluid : fluids) {
    fluid->resetPhysics();
  }
}

float PhysicsEngine::getSimTime() const {
  return SPH::Simulation::hasCurrent()
           ? static_cast<float>(SPH::TimeManager::getCurrent()->getTime())
           : 0.0f;
}