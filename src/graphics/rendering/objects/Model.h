#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Mesh.h"
#include "graphics/rendering/wrappers/Program.h"
#include "graphics/rendering/wrappers/Texture.h"

class Model {
public:
  explicit Model(const std::string& filename);
  ~Model() = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

  Model(Model&&) noexcept = default;
  Model& operator=(Model&&) noexcept = default;

  void draw(const Program& program) const;

  [[nodiscard]] const std::vector<Mesh>& getMeshes() const { return meshes; }

private:
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<Mesh> meshes;
};