#include "physics/PhysicsEngine.h"

#include <algorithm>
#include <utility>

#include "SPlisHSPlasH/DFSPH/TimeStepDFSPH.h"
#include "SPlisHSPlasH/Simulation.h"
#include "SPlisHSPlasH/TimeManager.h"
#include "SPlisHSPlasH/TimeStep.h"
#include "Utilities/Counting.h"
#include "Utilities/Logger.h"
#include "Utilities/Timing.h"

#include "physics/objects/FluidObject.h"
#include "physics/objects/PhysicsObject.h"

INIT_LOGGING
INIT_TIMING
INIT_COUNTING

namespace {
void appendBoundaryFaceSamples(
    std::vector<Vector3r>& outPositions,
    const glm::vec3& origin,
    const glm::vec3& uAxis,
    const glm::vec3& vAxis,
    const float uLength,
    const float vLength,
    const float particleSpacing) {
  if (particleSpacing <= 0.0F) {
    return;
  }

  const auto sampleCount = [particleSpacing](const float length) {
    return std::max(1, static_cast<int>(length / particleSpacing) + 1);
  };
  const auto stepSize = [](const float length, const int count) {
    return count > 1 ? (length / static_cast<float>(count - 1)) : 0.0F;
  };

  const int uCount = sampleCount(uLength);
  const int vCount = sampleCount(vLength);
  const float uStep = stepSize(uLength, uCount);
  const float vStep = stepSize(vLength, vCount);

  outPositions.reserve(
      outPositions.size() +
      (static_cast<std::size_t>(uCount) * static_cast<std::size_t>(vCount)));

  for (int uIndex = 0; uIndex < uCount; ++uIndex) {
    const glm::vec3 rowOrigin =
        origin + uAxis * (static_cast<float>(uIndex) * uStep);

    for (int vIndex = 0; vIndex < vCount; ++vIndex) {
      const glm::vec3 samplePos =
          rowOrigin + vAxis * (static_cast<float>(vIndex) * vStep);
      outPositions.emplace_back(samplePos.x, samplePos.y, samplePos.z);
    }
  }
}
}

PhysicsEngine::PhysicsEngine(
    const PhysicsConfig& engineConfig,
    const std::initializer_list<std::reference_wrapper<PhysicsObject>> objects)
  : config(engineConfig) {
  for (const auto& object : objects) {
    registerObject(object.get());
  }
  init();
}

PhysicsEngine::~PhysicsEngine() {
  if (SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }
}

void PhysicsEngine::registerObject(PhysicsObject& object) {
  if (auto* fluid = dynamic_cast<FluidObject*>(&object)) {
    fluids.push_back(fluid);
  } else if (object.isDynamic()) {
    dynamicObjects.push_back(&object);
  } else {
    staticObjects.push_back(&object);
  }
}

void PhysicsEngine::init() {
  if (SPH::Simulation::hasCurrent()) {
    delete SPH::Simulation::getCurrent();
  }

  SPH::Simulation* sim = SPH::Simulation::getCurrent();
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
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ERROR, 1.5f);
    timeStep->setValue(SPH::TimeStepDFSPH::MIN_ITERATIONS, 1u);
    timeStep->setValue(SPH::TimeStepDFSPH::MAX_ITERATIONS, 4u);
  }

  auto const initAll = [&](const auto& objects) {
    for (PhysicsObject* obj : objects) {
      obj->initPhysics(*sim, config.particleRadius,
                       config.boundarySpacingFactor);
    }
  };

  initAll(fluids);
  initAll(staticObjects);
  initAll(dynamicObjects);

  if (SPH::TimeStep* timeStep = sim->getTimeStep()) {
    timeStep->resize();
  }

  sim->setSimulationInitialized(1);
  for (unsigned int i = 0; i < sim->numberOfBoundaryModels(); ++i) {
    sim->getBoundaryModel(i)->deferredInit();
  }
  sim->performNeighborhoodSearchSort();
  sim->updateBoundaryVolume();

  std::ranges::for_each(fluids, &FluidObject::syncGpuBuffer);

  simAccumulator = 0.0;
}

void PhysicsEngine::step(const double dt) {
  if (paused) {
    simAccumulator = 0.0;
    return;
  }

  SPH::TimeStep* timeStep = SPH::Simulation::getCurrent()->getTimeStep();
  if (timeStep == nullptr) {
    return;
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

    for (PhysicsObject* object : dynamicObjects) {
      object->integrate(stepDt, config.gravity);
      resolveStaticCollisions(*object);
      object->syncBoundaryParticles();
    }

    simAccumulator -= static_cast<double>(stepDt);
    stepped = true;
  }

  if (stepped) {
    std::ranges::for_each(fluids, &FluidObject::syncGpuBuffer);
  }
}

void PhysicsEngine::resolveStaticCollisions(PhysicsObject& dynamicObject) {
  for (const PhysicsObject* staticObject : staticObjects) {
    dynamicObject.resolveStaticCollision(*staticObject, config.particleRadius);
  }
}

void PhysicsEngine::reset() {
  simAccumulator = 0.0;

  std::ranges::for_each(staticObjects, &PhysicsObject::resetPhysics);
  std::ranges::for_each(dynamicObjects, &PhysicsObject::resetPhysics);

  SPH::Simulation::getCurrent()->reset();
  SPH::TimeManager::getCurrent()->setTimeStepSize(config.fixedTimeStep);

  std::ranges::for_each(fluids, &FluidObject::resetPhysics);
}

float PhysicsEngine::getSimTime() const {
  return SPH::Simulation::hasCurrent()
           ? SPH::TimeManager::getCurrent()->getTime()
           : 0.0F;
}