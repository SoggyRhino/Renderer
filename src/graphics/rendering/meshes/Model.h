#pragma once

#include <memory>
#include <string>
#include <vector>

#include "graphics/rendering/materials/GltfMaterial.h"
#include "graphics/rendering/meshes/Mesh.h"
#include "graphics/rendering/wrappers/Program.h"
#include "graphics/rendering/wrappers/Texture.h"

struct ModelPart {
  Mesh mesh;
  GltfMaterial material;
};

class Model {
public:
  Model(const std::string& filename, const Program& program);
  ~Model() = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

  Model(Model&&) noexcept = default;
  Model& operator=(Model&&) noexcept = default;

  [[nodiscard]] const std::vector<ModelPart>& getParts() const {
    return parts;
  }

private:

  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<ModelPart> parts;
};