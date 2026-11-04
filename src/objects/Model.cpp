#include "Model.h"

#include <filesystem>
#include <format>
#include <stdexcept>

#include "tiny_gltf_v3.h"

Model::Model(const std::string& filename) {
  tg3_parse_options opts;
  tg3_parse_options_init(&opts);

  tg3_error_stack errors;
  tg3_error_stack_init(&errors);

  tg3_model model = {.arena_ = nullptr};

  const int err = tg3_parse_file(&model, &errors, filename.c_str(),
                                 filename.size(), &opts);
  if (err != 0) {
    std::string details;
    for (uint32_t i = 0; i < errors.count; ++i) {
      if (i > 0) details += " | ";
      details += errors.entries[i].message
                   ? errors.entries[i].message
                   : "Unknown error";
    }
    tg3_error_stack_free(&errors);
    throw std::runtime_error(std::format(
        "Could not load GLTF model '{}' (Code: {}): {}",
        filename, err, details.empty() ? "no details" : details));
  }

  std::filesystem::path baseDir = std::filesystem::path(filename).parent_path();
  textures.reserve(model.images_count);

  for (uint32_t i = 0; i < model.images_count; ++i) {
    const tg3_image& img = model.images[i];
    std::unique_ptr<Texture> tex = nullptr;

    if (img.uri.data && img.uri.len > 0) {
      std::string uri(img.uri.data, img.uri.len);
      std::filesystem::path imgPath = baseDir / uri;
      tex = std::make_unique<Texture>(imgPath);
    } else if (img.buffer_view >= 0 && static_cast<uint32_t>(img.buffer_view) <
               model.buffer_views_count) {
      const tg3_buffer_view& view = model.buffer_views[img.buffer_view];
      if (view.buffer >= 0 && static_cast<uint32_t>(view.buffer) < model.
          buffers_count) {
        const tg3_buffer& buf = model.buffers[view.buffer];
        if (buf.data.data) {
          tex = std::make_unique<Texture>(buf.data.data + view.byte_offset,
                                          view.byte_length);
        }
      }
    }
    textures.push_back(std::move(tex));
  }

  auto getTextureByGltfIndex = [&](int32_t texIndex) -> const Texture* {
    if (texIndex >= 0 && static_cast<uint32_t>(texIndex) < model.
        textures_count) {
      const tg3_texture& tex = model.textures[texIndex];
      if (tex.source >= 0 && static_cast<uint32_t>(tex.source) < textures.
          size()) {
        return textures[tex.source].get();
      }
    }
    return nullptr;
  };

  for (uint32_t m = 0; m < model.meshes_count; ++m) {
    const tg3_mesh& mesh = model.meshes[m];
    for (uint32_t p = 0; p < mesh.primitives_count; ++p) {
      const tg3_primitive& prim = mesh.primitives[p];
      const Texture* baseColorTex = nullptr;

      if (prim.material >= 0 && static_cast<uint32_t>(prim.material) < model.
          materials_count) {
        const tg3_material& mat = model.materials[prim.material];
        const int32_t texIdx = mat.pbr_metallic_roughness.base_color_texture.
                                   index;
        if (texIdx >= 0) {
          baseColorTex = getTextureByGltfIndex(texIdx);
        }
      }

      meshes.emplace_back(model, prim, baseColorTex);
    }
  }

  tg3_model_free(&model);
  tg3_error_stack_free(&errors);
}

void Model::draw(const Program& program) const {
  for (const auto& mesh : meshes) {
    mesh.draw(program);
  }
}