#pragma once

#include <string>

#include "graphics/rendering/loader/GltfAsset.h"
#include "graphics/rendering/wrappers/Program.h"

class GltfLoader {
public:
  [[nodiscard]] static GltfAsset load(
      const std::string& filename,
      const Program& program);
};