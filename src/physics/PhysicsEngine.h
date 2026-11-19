#pragma once

#include <functional>
#include <initializer_list>
#include <vector>

#include <glm/glm.hpp>

namespace SPH {
class Simulation;
}

class FluidObject;
class PhysicsObject;

struct PhysicsConfig {
  float particleRadius = 0.035F;
  float boundarySpacingFactor = 1.8F;
  float fixedTimeStep = 0.006F;
  glm::vec3 gravity{0.0F, -9.81F, 0.0F};
  float maxParticleSpeed = 12.0F;
  int preSettleSteps = 25;
};

class PhysicsEngine {
public:
  explicit PhysicsEngine(
      const PhysicsConfig& engineConfig = {},
      std::initializer_list<std::reference_wrapper<PhysicsObject>> objects = {});

  ~PhysicsEngine();

  PhysicsEngine(const PhysicsEngine&) = delete;
  PhysicsEngine& operator=(const PhysicsEngine&) = delete;

  void step(double dt);
  void reset();

  void setPaused(const bool isPaused) { paused = isPaused; }
  [[nodiscard]] bool isPaused() const { return paused; }

  void setTimeScale(const float scale) { timeScale = scale; }

  [[nodiscard]] float getSimTime() const;

private:
  void registerObject(PhysicsObject& object);
  void init();
  void resolveStaticCollisions(PhysicsObject& dynamicObject);

  PhysicsConfig config{};
  std::vector<PhysicsObject*> staticObjects;
  std::vector<PhysicsObject*> dynamicObjects;
  std::vector<FluidObject*> fluids;

  bool paused = false;
  float timeScale = 1.0f;

  double simAccumulator = 0.0;
};