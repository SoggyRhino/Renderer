#include "physics/PhysicsEngine.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "SPlisHSPlasH/DFSPH/TimeStepDFSPH.h"
#include "SPlisHSPlasH/Simulation.h"
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

PhysicsEngine::SimulationSession::~SimulationSession() {
  if (active && SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }
}

PhysicsEngine::SimulationSession::SimulationSession(
    SimulationSession&& other) noexcept
  : active(std::exchange(other.active, false)) {
}

PhysicsEngine::SimulationSession& PhysicsEngine::SimulationSession::operator=(
    SimulationSession&& other) noexcept {
  if (this != &other) {
    if (active && SPH::Simulation::hasCurrent()) {
      delete SPH::Simulation::getCurrent();
    }
    active = std::exchange(other.active, false);
  }
  return *this;
}

PhysicsEngine::Builder::Builder(const PhysicsConfig& engineConfig)
  : config(engineConfig) {
  if (SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  simulation = SimulationSession(true);

  sim->init(config.particleRadius, false);
  sim->setBoundaryHandlingMethod(SPH::BoundaryHandlingMethods::Akinci2012);
  sim->setSimulationMethod(std::to_underlying(SPH::SimulationMethods::DFSPH));
  sim->setValue(SPH::Simulation::CFL_METHOD, 0);
  sim->setValue(SPH::Simulation::STEPS_PER_Z_SORT, 25u);

  Vector3r grav(config.gravity.x, config.gravity.y, config.gravity.z);
  sim->setVecValue<Real>(SPH::Simulation::GRAVITATION, grav.data());

  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);
  if (SPH::TimeStep* timeStep = sim->getTimeStep()) {
    timeStep->setValue(SPH::TimeStepDFSPH::USE_DIVERGENCE_SOLVER, false);
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ERROR, 1.5F);
    timeStep->setValue(SPH::TimeStepDFSPH::MIN_ITERATIONS, 1u);
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ITERATIONS, 4u);
  }
}

PhysicsEngine::Builder::~Builder() = default;

BoxView PhysicsEngine::Builder::addBox(
    const glm::vec3& position,
    const glm::vec3& halfExtents,
    const bool isDynamic,
    const float density) {
  if (!simulation.active) {
    throw std::logic_error(
        "Cannot add Box after PhysicsEngine::Builder::build()");
  }

  auto box = std::make_unique<Box>(
      *SPH::Simulation::getCurrent(),
      position,
      halfExtents,
      config.particleRadius,
      config.boundarySpacingFactor,
      isDynamic,
      density);

  BoxView view(*box);
  if (isDynamic) {
    dynamicObjects.push_back(std::move(box));
  } else {
    staticObjects.push_back(std::move(box));
  }
  return view;
}

FluidView PhysicsEngine::Builder::addFluid(
    const glm::vec3& minCorner,
    const glm::vec3& maxCorner,
    const float xsphViscosity) {
  if (!simulation.active) {
    throw std::logic_error(
        "Cannot add Fluid after PhysicsEngine::Builder::build()");
  }

  auto fluid = std::make_unique<FluidObject>(
      *SPH::Simulation::getCurrent(),
      minCorner,
      maxCorner,
      config.particleRadius,
      xsphViscosity);

  FluidView view(*fluid);
  fluids.push_back(std::move(fluid));
  return view;
}

PhysicsEngine PhysicsEngine::Builder::build() {
  if (!simulation.active) {
    throw std::logic_error("PhysicsEngine::Builder::build() already called");
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
  if (SPH::TimeStep* timeStep = sim->getTimeStep()) {
    timeStep->resize();
  }

  sim->setSimulationInitialized(1);
  for (unsigned int i = 0; i < sim->numberOfBoundaryModels(); ++i) {
    sim->getBoundaryModel(i)->deferredInit();
  }
  sim->performNeighborhoodSearchSort();
  sim->updateBoundaryVolume();

  return PhysicsEngine(
      config,
      std::move(simulation),
      std::move(staticObjects),
      std::move(dynamicObjects),
      std::move(fluids));
}

PhysicsEngine::PhysicsEngine(
    const PhysicsConfig& engineConfig,
    SimulationSession&& session,
    std::vector<std::unique_ptr<PhysicsObject>>&& statics,
    std::vector<std::unique_ptr<PhysicsObject>>&& dynamics,
    std::vector<std::unique_ptr<FluidObject>>&& fluidBodies)
  : config(engineConfig),
    simulation(std::move(session)),
    staticObjects(std::move(statics)),
    dynamicObjects(std::move(dynamics)),
    fluids(std::move(fluidBodies)) {
}

PhysicsEngine::~PhysicsEngine() = default;

PhysicsEngine::PhysicsEngine(PhysicsEngine&& other) noexcept = default;

PhysicsEngine& PhysicsEngine::operator=(PhysicsEngine&& other) noexcept {
  if (this != &other) {
    // Destroy owned bodies before tearing down the current SPH::Simulation.
    fluids.clear();
    dynamicObjects.clear();
    staticObjects.clear();

    config = other.config;
    simulation = std::move(other.simulation);
    staticObjects = std::move(other.staticObjects);
    dynamicObjects = std::move(other.dynamicObjects);
    fluids = std::move(other.fluids);
    paused = other.paused;
    timeScale = other.timeScale;
    simAccumulator = other.simAccumulator;
  }
  return *this;
}

bool PhysicsEngine::step(const double dt) {
  if (!simulation.active || paused) {
    simAccumulator = 0.0;
    return false;
  }

  SPH::TimeStep* timeStep = SPH::Simulation::getCurrent()->getTimeStep();
  if (timeStep == nullptr) {
    return false;
  }

  constexpr double kMaxFrameDt = 0.05;
  constexpr int kMaxSubstepsPerFrame = 10;

  const double scale = timeScale;
  const double maxAccumulator = kMaxFrameDt * scale;
  const double frameDt = std::min(dt, kMaxFrameDt) * scale;
  simAccumulator = std::min(simAccumulator + frameDt, maxAccumulator);

  SPH::TimeManager* timeManager = SPH::TimeManager::getCurrent();
  bool stepped = false;

  for (int substep = 0;
       simAccumulator > 0.0 && substep < kMaxSubstepsPerFrame;
       ++substep) {
    const float stepDt = timeManager->getTimeStepSize();
    timeStep->step();

    for (const auto& object : dynamicObjects) {
      object->integrate(stepDt, config.gravity);
      resolveStaticCollisions(*object);
      object->syncBoundaryParticles();
    }

    simAccumulator -= static_cast<double>(stepDt);
    stepped = true;
  }

  return stepped;
}

void PhysicsEngine::resolveStaticCollisions(PhysicsObject& dynamicObject) {
  for (const auto& staticObject : staticObjects) {
    dynamicObject.resolveStaticCollision(*staticObject, config.particleRadius);
  }
}

void PhysicsEngine::reset() {
  if (!simulation.active) {
    return;
  }

  simAccumulator = 0.0;

  for (const auto& object : staticObjects) {
    object->resetPhysics();
  }
  for (const auto& object : dynamicObjects) {
    object->resetPhysics();
  }

  SPH::Simulation::getCurrent()->reset();
  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);

  for (const auto& fluid : fluids) {
    fluid->resetPhysics();
  }
}

float PhysicsEngine::getSimTime() const {
  return (simulation.active && SPH::Simulation::hasCurrent())
           ? SPH::TimeManager::getCurrent()->getTime()
           : 0.0F;
}