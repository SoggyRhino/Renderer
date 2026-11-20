#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "physics/PhysicsViews.h"

class FluidObject;
class PhysicsObject;

struct PhysicsConfig {
  float particleRadius = 0.035F;
  float boundarySpacingFactor = 1.6F;
  float fixedTimeStep = 0.004F;
  glm::vec3 gravity{0.0F, -9.81F, 0.0F};
  float maxParticleSpeed = 12.0F;
  std::optional<PhysicsBounds> worldBounds{};
};

class PhysicsEngine {
  struct SimulationSession {
    bool active = false;

    SimulationSession() = default;

    explicit SimulationSession(const bool isActive) : active(isActive) {
    }

    ~SimulationSession();

    SimulationSession(const SimulationSession&) = delete;
    SimulationSession& operator=(const SimulationSession&) = delete;

    SimulationSession(SimulationSession&& other) noexcept;
    SimulationSession& operator=(SimulationSession&& other) noexcept;
  };

public:
  class Builder {
  public:
    explicit Builder(const PhysicsConfig& engineConfig = {});
    ~Builder();

    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;
    Builder(Builder&&) = delete;
    Builder& operator=(Builder&&) = delete;

    Builder& setWorldBounds(const glm::vec3& minCorner,
                            const glm::vec3& maxCorner);

    BoxView addBox(
        const glm::vec3& position,
        const glm::vec3& halfExtents,
        bool isDynamic = false,
        float density = 1000.0F);

    FluidView addFluid(
        const glm::vec3& minCorner,
        const glm::vec3& maxCorner,
        float xsphViscosity = 0.02F);

    [[nodiscard]] PhysicsEngine build();

  private:
    PhysicsConfig config{};

    SimulationSession simulation;
    std::vector<std::unique_ptr<PhysicsObject>> staticObjects;
    std::vector<std::unique_ptr<PhysicsObject>> dynamicObjects;
    std::vector<std::unique_ptr<FluidObject>> fluids;
  };

  ~PhysicsEngine();

  PhysicsEngine(const PhysicsEngine&) = delete;
  PhysicsEngine& operator=(const PhysicsEngine&) = delete;

  PhysicsEngine(PhysicsEngine&& other) noexcept;
  PhysicsEngine& operator=(PhysicsEngine&& other) noexcept;

  bool step(double dt);
  void reset();

  void setPaused(const bool isPaused) { paused = isPaused; }
  [[nodiscard]] bool isPaused() const { return paused; }

  void setTimeScale(const float scale) { timeScale = scale; }

  [[nodiscard]] float getSimTime() const;

private:
  PhysicsEngine(
      const PhysicsConfig& engineConfig,
      SimulationSession&& session,
      std::vector<std::unique_ptr<PhysicsObject>>&& statics,
      std::vector<std::unique_ptr<PhysicsObject>>&& dynamics,
      std::vector<std::unique_ptr<FluidObject>>&& fluidBodies);

  void resolveStaticCollisions(PhysicsObject& dynamicObject);

  PhysicsConfig config{};
  // Declared before owned objects so objects are destroyed before SPH::Simulation.
  SimulationSession simulation;
  std::vector<std::unique_ptr<PhysicsObject>> staticObjects;
  std::vector<std::unique_ptr<PhysicsObject>> dynamicObjects;
  std::vector<std::unique_ptr<FluidObject>> fluids;

  bool paused = false;
  float timeScale = 1.0F;

  double simAccumulator = 0.0;
};