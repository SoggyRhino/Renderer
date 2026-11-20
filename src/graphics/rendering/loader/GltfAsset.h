#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/rendering/materials/GltfMaterial.h"
#include "graphics/rendering/meshes/GLTFMesh.h"
#include "graphics/rendering/wrappers/Texture.h"

struct GltfMeshInstance {
  std::string name;
  std::shared_ptr<GLTFMesh> mesh;
  std::shared_ptr<GltfMaterial> material;
  glm::mat4 transform{1.0F};
};

struct GltfMaterialBatch {
  std::shared_ptr<GltfMaterial> material;
  std::vector<GltfMeshInstance> instances;
};

struct GltfAsset {
  std::vector<std::shared_ptr<Texture>> textures;
  std::vector<std::shared_ptr<GltfMaterial>> materials;
  std::vector<std::shared_ptr<GLTFMesh>> meshes;
  std::vector<GltfMaterialBatch> batches;

  [[nodiscard]] std::size_t instanceCount() const {
    std::size_t total = 0;
    for (const auto& batch : batches) {
      total += batch.instances.size();
    }
    return total;
  }

  [[nodiscard]] const GltfMeshInstance* findInstance(
      const std::string_view instanceName) const {
    for (const auto& [material, instances] : batches) {
      for (const auto& instance : instances) {
        if (instance.name == instanceName) {
          return &instance;
        }
      }
    }
    return nullptr;
  }
};