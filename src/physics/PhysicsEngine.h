#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace SPH {
class Simulation;
}

class Box;
class FluidObject;
class PhysicsObject;

struct PhysicsConfig {
  float particleRadius = 0.035f;
  float boundarySpacingFactor = 1.8f;
  float fixedTimeStep = 0.006f;
  glm::vec3 gravity{0.0f, -9.81f, 0.0f};
  float maxParticleSpeed = 12.0f;
  int preSettleSteps = 25;
};

class PhysicsEngine {
public:
  explicit PhysicsEngine(const PhysicsConfig& config = {});
  ~PhysicsEngine();

  PhysicsEngine(const PhysicsEngine&) = delete;
  PhysicsEngine& operator=(const PhysicsEngine&) = delete;

  void registerObject(PhysicsObject& object);
  void init();
  void step(double dt);
  void reset();

  void setPaused(const bool isPaused) { paused = isPaused; }
  [[nodiscard]] bool isPaused() const { return paused; }

  void setTimeScale(const float scale) { timeScale = scale; }
  void setWorldBounds(const glm::vec3& minCorner, const glm::vec3& maxCorner);

  [[nodiscard]] float getSimTime() const;
  [[nodiscard]] float getLastStepTimeMs() const { return lastStepTimeMs; }
  [[nodiscard]] int getLastSubsteps() const { return lastSubsteps; }
  [[nodiscard]] float getLastSimSpeedRatio() const { return lastSimSpeedRatio; }

private:
  void initWorldBoundsBoundary(SPH::Simulation& sim) const;
  void preSettleFluids();

  PhysicsConfig config{};
  std::vector<Box*> staticBoxes;
  std::vector<Box*> dynamicBoxes;
  std::vector<FluidObject*> fluids;

  bool initialized = false;
  bool paused = false;
  float timeScale = 1.0f;

  bool hasWorldBounds = false;
  glm::vec3 worldBoundsMin{0.0f};
  glm::vec3 worldBoundsMax{0.0f};

  double simAccumulator = 0.0;
  float lastSimSpeedRatio = 0.0f;
  float lastStepTimeMs = 0.0f;
  int lastSubsteps = 0;
};