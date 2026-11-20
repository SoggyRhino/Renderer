#pragma once

#include <cstddef>
#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Box;
class FluidObject;

class BoxView {
public:
  BoxView() = default;
  explicit BoxView(const Box& target) : box(&target) {}

  [[nodiscard]] bool valid() const { return box != nullptr; }
  [[nodiscard]] explicit operator bool() const { return valid(); }

  [[nodiscard]] glm::vec3 position() const;
  [[nodiscard]] glm::quat rotation() const;
  [[nodiscard]] glm::vec3 halfExtents() const;
  [[nodiscard]] glm::mat4 modelMatrix() const;

private:
  const Box* box = nullptr;
};

class FluidView {
public:
  FluidView() = default;
  explicit FluidView(const FluidObject& target) : fluid(&target) {}

  [[nodiscard]] bool valid() const { return fluid != nullptr; }
  [[nodiscard]] explicit operator bool() const { return valid(); }

  [[nodiscard]] std::span<const glm::vec3> particles() const;
  [[nodiscard]] std::size_t particleCount() const;

private:
  const FluidObject* fluid = nullptr;
};
