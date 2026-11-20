#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SPlisHSPlasH/Common.h"

inline Vector3r toSph(const glm::vec3& vec) {
  return {static_cast<Real>(vec.x), static_cast<Real>(vec.y),
          static_cast<Real>(vec.z)};
}

inline glm::vec3 toGlm(const Vector3r& vec) {
  return {static_cast<float>(vec.x()), static_cast<float>(vec.y()),
          static_cast<float>(vec.z())};
}

inline Quaternionr toSph(const glm::quat& quat) {
  return {static_cast<Real>(quat.w), static_cast<Real>(quat.x),
          static_cast<Real>(quat.y), static_cast<Real>(quat.z)};
}

inline glm::quat toGlm(const Quaternionr& quat) {
  return {static_cast<float>(quat.w()), static_cast<float>(quat.x()),
          static_cast<float>(quat.y()), static_cast<float>(quat.z())};
}