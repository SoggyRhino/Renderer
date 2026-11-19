#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

inline void appendGridFace(
    std::vector<glm::vec3>& outPoints,
    const glm::vec3& origin,
    const glm::vec3& uAxis,
    const glm::vec3& vAxis,
    const float uLength,
    const float vLength,
    const float spacing) {
  if (spacing <= 0.0F || uLength < 0.0F || vLength < 0.0F) {
    return;
  }

  const auto sampleCount = [spacing](const float length) {
    return std::max(1, static_cast<int>(length / spacing) + 1);
  };
  const auto stepSize = [](const float length, const int count) {
    return count > 1 ? (length / static_cast<float>(count - 1)) : 0.0F;
  };

  const int uCount = sampleCount(uLength);
  const int vCount = sampleCount(vLength);
  const float uStep = stepSize(uLength, uCount);
  const float vStep = stepSize(vLength, vCount);

  outPoints.reserve(
      outPoints.size() +
      (static_cast<std::size_t>(uCount) * static_cast<std::size_t>(vCount)));

  for (int uIndex = 0; uIndex < uCount; ++uIndex) {
    const glm::vec3 rowOrigin =
        origin + uAxis * (static_cast<float>(uIndex) * uStep);

    for (int vIndex = 0; vIndex < vCount; ++vIndex) {
      outPoints.push_back(
          rowOrigin + vAxis * (static_cast<float>(vIndex) * vStep));
    }
  }
}
