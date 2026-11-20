#include "graphics/rendering/loader/GltfLoader.h"

#include <array>
#include <filesystem>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "tiny_gltf_v3.h"

namespace {
const std::unordered_map<std::string_view, GLuint> kAttributeLocations = {
    {"POSITION", 0},
    {"NORMAL", 1},
    {"TEXCOORD_0", 2},
};

struct ParsedGltf {
  tg3_model model{.arena_ = nullptr};

  ParsedGltf() = default;
  ~ParsedGltf() { tg3_model_free(&model); }

  ParsedGltf(const ParsedGltf&) = delete;
  ParsedGltf& operator=(const ParsedGltf&) = delete;

  ParsedGltf(ParsedGltf&& other) noexcept : model(other.model) {
    other.model.arena_ = nullptr;
  }

  ParsedGltf& operator=(ParsedGltf&& other) noexcept {
    if (this != &other) {
      tg3_model_free(&model);
      model = other.model;
      other.model.arena_ = nullptr;
    }
    return *this;
  }
};

struct LoadedPrimitive {
  std::shared_ptr<GLTFMesh> mesh;
  std::size_t materialIndex = 0;
};

GLint getVertexAttribComponents(const int32_t tg3Type) {
  switch (tg3Type) {
    case TG3_TYPE_SCALAR:
    case TG3_TYPE_VEC2:
    case TG3_TYPE_VEC3:
    case TG3_TYPE_VEC4: {
      const int32_t count = tg3_num_components(tg3Type);
      return (count > 0) ? static_cast<GLint>(count) : 0;
    }
    default:
      return 0;
  }
}

std::string toString(const tg3_str& str) {
  return (str.data != nullptr && str.len > 0)
           ? std::string(str.data, str.len)
           : std::string{};
}

std::optional<std::span<const uint8_t>> getBufferViewSpan(
    const tg3_model& model,
    const int32_t bufferViewIdx) {
  if (bufferViewIdx < 0 ||
      std::cmp_greater_equal(bufferViewIdx, model.buffer_views_count)) {
    return std::nullopt;
  }

  const tg3_buffer_view& view = model.buffer_views[bufferViewIdx];
  if (view.buffer < 0 ||
      std::cmp_greater_equal(view.buffer, model.buffers_count)) {
    return std::nullopt;
  }

  const tg3_buffer& buf = model.buffers[view.buffer];
  if (buf.data.data == nullptr ||
      view.byte_length == 0 ||
      view.byte_offset > buf.data.count ||
      view.byte_length > buf.data.count - view.byte_offset) {
    return std::nullopt;
  }

  return std::span(buf.data.data, buf.data.count)
      .subspan(view.byte_offset, view.byte_length);
}

std::optional<std::vector<uint8_t>> decodeBase64DataUri(
    const std::string_view uri) {
  const std::size_t comma = uri.find(',');
  if (comma == std::string_view::npos) {
    return std::nullopt;
  }

  const std::string_view header = uri.substr(0, comma);
  constexpr std::string_view kBase64Tag = ";base64";
  if (!header.ends_with(kBase64Tag)) {
    return std::nullopt;
  }

  static constexpr std::array<int8_t, 256> kDecodeTable = []() {
    std::array<int8_t, 256> table{};
    table.fill(-1);
    for (int i = 0; i < 26; ++i) {
      table[static_cast<std::size_t>('A' + i)] = static_cast<int8_t>(i);
      table[static_cast<std::size_t>('a' + i)] = static_cast<int8_t>(26 + i);
    }
    for (int i = 0; i < 10; ++i) {
      table[static_cast<std::size_t>('0' + i)] = static_cast<int8_t>(52 + i);
    }
    table[static_cast<std::size_t>('+')] = 62;
    table[static_cast<std::size_t>('/')] = 63;
    return table;
  }();

  const std::string_view payload = uri.substr(comma + 1);
  std::vector<uint8_t> out;
  out.reserve((payload.size() / 4) * 3);

  uint32_t accum = 0;
  int bits = 0;
  bool seenPadding = false;
  for (const char ch : payload) {
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
      continue;
    }
    if (ch == '=') {
      seenPadding = true;
      continue;
    }
    if (seenPadding) {
      return std::nullopt;
    }
    const int8_t val = kDecodeTable[static_cast<uint8_t>(ch)];
    if (val < 0) {
      return std::nullopt;
    }
    accum = (accum << 6) | static_cast<uint32_t>(val);
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      out.push_back(static_cast<uint8_t>((accum >> bits) & 0xFFU));
    }
  }

  if (out.empty()) {
    return std::nullopt;
  }
  return out;
}

ParsedGltf parseFile(const std::string& filename) {
  tg3_parse_options opts{};
  tg3_parse_options_init(&opts);

  tg3_error_stack errors{};
  tg3_error_stack_init(&errors);

  ParsedGltf parsed;
  const tg3_error_code err = tg3_parse_file(
      &parsed.model, &errors, filename.c_str(),
      static_cast<uint32_t>(filename.size()), &opts);
  if (err != TG3_OK) {
    const auto details =
        std::span(errors.entries, errors.count) |
        std::views::transform([](const auto& entry) -> std::string_view {
          return entry.message ? entry.message : "Unknown error";
        }) |
        std::views::join_with(std::string_view{" | "}) |
        std::ranges::to<std::string>();

    tg3_error_stack_free(&errors);
    throw std::runtime_error(std::format(
        "Could not load GLTF model '{}' (Code: {}): {}",
        filename, static_cast<int>(err),
        details.empty() ? "no details" : details));
  }

  tg3_error_stack_free(&errors);
  return parsed;
}

std::shared_ptr<Texture> loadImage(
    const tg3_model& model,
    const tg3_image& img,
    const std::filesystem::path& baseDir) {
  if (img.uri.data != nullptr && img.uri.len > 0) {
    const std::string_view uri(img.uri.data, img.uri.len);
    if (tg3_is_data_uri(img.uri.data, img.uri.len) != 0) {
      const auto decoded = decodeBase64DataUri(uri);
      if (!decoded.has_value()) {
        return nullptr;
      }
      auto tex = std::make_shared<Texture>(decoded->data(), decoded->size());
      return tex->isValid() ? tex : nullptr;
    }

    const std::filesystem::path uriPath(std::u8string_view(
        reinterpret_cast<const char8_t*>(uri.data()), uri.size()));
    auto tex = std::make_shared<Texture>(baseDir / uriPath);
    return tex->isValid() ? tex : nullptr;
  }

  const auto bytes = getBufferViewSpan(model, img.buffer_view);
  if (!bytes.has_value()) {
    return nullptr;
  }

  auto tex = std::make_shared<Texture>(bytes->data(), bytes->size());
  return tex->isValid() ? tex : nullptr;
}

std::vector<std::shared_ptr<Texture>> loadImages(
    const tg3_model& model,
    const std::filesystem::path& baseDir) {
  std::vector<std::shared_ptr<Texture>> images;
  images.reserve(model.images_count);

  for (const auto img : std::span(model.images, model.images_count)) {
    images.push_back(loadImage(model, img, baseDir));
  }
  return images;
}

std::vector<std::shared_ptr<GltfMaterial>> loadMaterials(
    const tg3_model& model,
    const std::vector<std::shared_ptr<Texture>>& images,
    const Program& program) {
  std::vector<std::shared_ptr<GltfMaterial>> materials;
  materials.reserve(model.materials_count);

  for (const auto mat : std::span(model.materials, model.materials_count)) {
    std::shared_ptr<const Texture> baseColor;
    const int32_t texIdx =
        mat.pbr_metallic_roughness.base_color_texture.index;
    if (texIdx >= 0 && std::cmp_less(texIdx, model.textures_count)) {
      const int32_t imgIdx = model.textures[texIdx].source;
      if (imgIdx >= 0 && std::cmp_less(imgIdx, images.size())) {
        baseColor = images[static_cast<std::size_t>(imgIdx)];
      }
    }

    materials.push_back(std::make_shared<GltfMaterial>(GltfMaterial{
        .program = program,
        .baseColorTexture = std::move(baseColor),
    }));
  }

  return materials;
}

std::vector<std::shared_ptr<Buffer>> loadBuffers(const tg3_model& model) {
  std::vector isImageView(model.buffer_views_count, false);
  for (uint32_t i = 0; i < model.images_count; ++i) {
    if (model.images[i].buffer_view >= 0 &&
        std::cmp_less(model.images[i].buffer_view, model.buffer_views_count)) {
      isImageView[static_cast<std::size_t>(model.images[i].buffer_view)] = true;
    }
  }

  std::vector<std::shared_ptr<Buffer>> buffers(model.buffer_views_count);
  for (uint32_t i = 0; i < model.buffer_views_count; ++i) {
    if (isImageView[i] ||
        std::cmp_greater(i, std::numeric_limits<int32_t>::max())) {
      continue;
    }

    const auto bytes = getBufferViewSpan(model, static_cast<int32_t>(i));
    if (bytes.has_value()) {
      buffers[i] = std::make_shared<Buffer>(*bytes, GL_STATIC_DRAW);
    }
  }

  return buffers;
}

std::shared_ptr<Buffer> loadSparseAccessor(
    const tg3_model& model,
    const tg3_accessor& acc) {
  const int32_t compSizeI = tg3_component_size(acc.component_type);
  const int32_t numCompI = tg3_num_components(acc.type);
  if (compSizeI <= 0 || numCompI <= 0 || acc.count == 0) {
    return nullptr;
  }

  const auto elemSize =
      static_cast<std::size_t>(compSizeI) * static_cast<std::size_t>(numCompI);
  if (acc.count > std::numeric_limits<std::size_t>::max() / elemSize) {
    return nullptr;
  }

  const auto count = static_cast<std::size_t>(acc.count);
  std::vector<uint8_t> data(count * elemSize, 0);

  if (acc.buffer_view >= 0) {
    const auto baseSpan = getBufferViewSpan(model, acc.buffer_view);
    if (!baseSpan.has_value() || acc.byte_offset > baseSpan->size()) {
      return nullptr;
    }

    const std::size_t rawStride =
        model.buffer_views[acc.buffer_view].byte_stride;
    const std::size_t stride = (rawStride > 0) ? rawStride : elemSize;
    if (stride < elemSize) {
      return nullptr;
    }

    const auto baseOffset = static_cast<std::size_t>(acc.byte_offset);
    for (std::size_t i = 0; i < count; ++i) {
      const std::size_t srcOffset = baseOffset + i * stride;
      if (srcOffset > baseSpan->size() ||
          elemSize > baseSpan->size() - srcOffset) {
        return nullptr;
      }
      std::memcpy(data.data() + i * elemSize, baseSpan->data() + srcOffset,
                  elemSize);
    }
  }

  if (acc.sparse.count < 0) {
    return nullptr;
  }
  const auto sparseCount = static_cast<std::size_t>(acc.sparse.count);
  if (sparseCount == 0) {
    return std::make_shared<Buffer>(data, GL_STATIC_DRAW);
  }

  const auto indicesSpan =
      getBufferViewSpan(model, acc.sparse.indices.buffer_view);
  const auto valuesSpan =
      getBufferViewSpan(model, acc.sparse.values.buffer_view);
  if (!indicesSpan.has_value() || !valuesSpan.has_value()) {
    return nullptr;
  }

  const int32_t idxCompSizeI =
      tg3_component_size(acc.sparse.indices.component_type);
  if (idxCompSizeI <= 0) {
    return nullptr;
  }
  const auto idxCompSize = static_cast<std::size_t>(idxCompSizeI);

  if (acc.sparse.indices.byte_offset > indicesSpan->size() ||
      sparseCount >
          (indicesSpan->size() - acc.sparse.indices.byte_offset) / idxCompSize) {
    return nullptr;
  }
  if (acc.sparse.values.byte_offset > valuesSpan->size() ||
      sparseCount >
          (valuesSpan->size() - acc.sparse.values.byte_offset) / elemSize) {
    return nullptr;
  }

  const uint8_t* idxPtr =
      indicesSpan->data() + acc.sparse.indices.byte_offset;
  const uint8_t* valPtr =
      valuesSpan->data() + acc.sparse.values.byte_offset;

  for (std::size_t s = 0; s < sparseCount; ++s) {
    std::size_t targetIdx = 0;
    switch (acc.sparse.indices.component_type) {
      case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
        targetIdx = idxPtr[s];
        break;
      case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: {
        uint16_t v = 0;
        std::memcpy(&v, idxPtr + s * sizeof(uint16_t), sizeof(uint16_t));
        targetIdx = v;
        break;
      }
      case TG3_COMPONENT_TYPE_UNSIGNED_INT: {
        uint32_t v = 0;
        std::memcpy(&v, idxPtr + s * sizeof(uint32_t), sizeof(uint32_t));
        targetIdx = v;
        break;
      }
      default:
        return nullptr;
    }

    if (targetIdx >= count) {
      return nullptr;
    }
    std::memcpy(data.data() + targetIdx * elemSize, valPtr + s * elemSize,
                elemSize);
  }

  return std::make_shared<Buffer>(data, GL_STATIC_DRAW);
}

std::shared_ptr<GLTFMesh> loadPrimitive(
    const tg3_model& model,
    const tg3_primitive& prim,
    const std::vector<std::shared_ptr<Buffer>>& buffers) {
  const GLenum mode =
      (prim.mode >= 0) ? static_cast<GLenum>(prim.mode) : GL_TRIANGLES;

  GLsizei vertexCount = 0;
  bool hasPosition = false;
  std::vector<GLTFMeshAttribute> attributes;
  attributes.reserve(prim.attributes_count);

  for (uint32_t a = 0; a < prim.attributes_count; ++a) {
    const std::string_view attrName(
        prim.attributes[a].key.data, prim.attributes[a].key.len);
    const auto loc = kAttributeLocations.find(attrName);
    if (loc == kAttributeLocations.end()) {
      continue;
    }

    const int32_t accIdx = prim.attributes[a].value;
    if (accIdx < 0 || std::cmp_greater_equal(accIdx, model.accessors_count)) {
      continue;
    }

    const tg3_accessor& acc = model.accessors[accIdx];
    const GLint components = getVertexAttribComponents(acc.type);
    if (components == 0) {
      continue;
    }

    std::shared_ptr<Buffer> vbo;
    GLsizei stride = 0;
    uintptr_t offset = 0;
    if (acc.sparse.is_sparse != 0) {
      vbo = loadSparseAccessor(model, acc);
    } else if (acc.buffer_view >= 0 &&
               std::cmp_less(acc.buffer_view, buffers.size())) {
      vbo = buffers[static_cast<std::size_t>(acc.buffer_view)];
      stride = static_cast<GLsizei>(
          model.buffer_views[acc.buffer_view].byte_stride);
      offset = static_cast<uintptr_t>(acc.byte_offset);
    }

    if (vbo == nullptr) {
      continue;
    }

    if (attrName == "POSITION") {
      vertexCount = static_cast<GLsizei>(acc.count);
      hasPosition = true;
    }

    attributes.push_back(GLTFMeshAttribute{
        .location = loc->second,
        .components = components,
        .componentType = static_cast<GLenum>(acc.component_type),
        .normalized = static_cast<GLboolean>(
          acc.normalized ? GL_TRUE : GL_FALSE),
        .stride = stride,
        .offset = offset,
        .vbo = std::move(vbo),
    });
  }

  if (!hasPosition || attributes.empty()) {
    return nullptr;
  }

  std::optional<GLTFMeshIndices> indices;
  if (prim.indices >= 0) {
    if (std::cmp_greater_equal(prim.indices, model.accessors_count)) {
      return nullptr;
    }

    const tg3_accessor& idxAcc = model.accessors[prim.indices];
    std::shared_ptr<Buffer> ebo;
    uintptr_t indexOffset = 0;
    if (idxAcc.sparse.is_sparse != 0) {
      ebo = loadSparseAccessor(model, idxAcc);
    } else if (idxAcc.buffer_view >= 0 &&
               std::cmp_less(idxAcc.buffer_view, buffers.size())) {
      ebo = buffers[static_cast<std::size_t>(idxAcc.buffer_view)];
      indexOffset = static_cast<uintptr_t>(idxAcc.byte_offset);
    }

    if (ebo == nullptr) {
      return nullptr;
    }

    indices = GLTFMeshIndices{
        .ebo = std::move(ebo),
        .count = static_cast<GLsizei>(idxAcc.count),
        .type = static_cast<GLenum>(idxAcc.component_type),
        .offset = indexOffset,
    };
  }

  return std::make_shared<GLTFMesh>(
      mode, vertexCount, std::move(attributes), std::move(indices));
}

std::vector<std::vector<LoadedPrimitive>> loadMeshes(
    const tg3_model& model,
    const std::vector<std::shared_ptr<Buffer>>& buffers,
    std::vector<std::shared_ptr<GltfMaterial>>& materials,
    const Program& program) {
  std::optional<std::size_t> defaultMatIndex;
  auto resolveMaterial = [&](const int32_t matIdx) -> std::size_t {
    if (matIdx >= 0 && std::cmp_less(matIdx, materials.size())) {
      return static_cast<std::size_t>(matIdx);
    }
    if (!defaultMatIndex.has_value()) {
      defaultMatIndex = materials.size();
      materials.push_back(std::make_shared<GltfMaterial>(GltfMaterial{
          .program = program,
          .baseColorTexture = nullptr,
      }));
    }
    return *defaultMatIndex;
  };

  std::vector<std::vector<LoadedPrimitive>> meshPrimitives(model.meshes_count);
  for (uint32_t m = 0; m < model.meshes_count; ++m) {
    const tg3_mesh& mesh = model.meshes[m];
    meshPrimitives[m].reserve(mesh.primitives_count);
    for (uint32_t p = 0; p < mesh.primitives_count; ++p) {
      const tg3_primitive& prim = mesh.primitives[p];
      auto loadedMesh = loadPrimitive(model, prim, buffers);
      if (loadedMesh == nullptr) {
        continue;
      }
      meshPrimitives[m].push_back(LoadedPrimitive{
          .mesh = std::move(loadedMesh),
          .materialIndex = resolveMaterial(prim.material),
      });
    }
  }

  return meshPrimitives;
}

glm::mat4 nodeLocalTransform(const tg3_node& node) {
  if (node.has_matrix != 0) {
    glm::mat4 matrix(1.0F);
    for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
        matrix[col][row] = static_cast<float>(node.matrix[col * 4 + row]);
      }
    }
    return matrix;
  }

  const glm::vec3 t(
      static_cast<float>(node.translation[0]),
      static_cast<float>(node.translation[1]),
      static_cast<float>(node.translation[2]));
  // glTF stores quaternions as [x, y, z, w]; glm::quat takes (w, x, y, z).
  const glm::quat r(
      static_cast<float>(node.rotation[3]),
      static_cast<float>(node.rotation[0]),
      static_cast<float>(node.rotation[1]),
      static_cast<float>(node.rotation[2]));
  const glm::vec3 s(
      static_cast<float>(node.scale[0]),
      static_cast<float>(node.scale[1]),
      static_cast<float>(node.scale[2]));

  return glm::translate(glm::mat4(1.0F), t) *
         glm::mat4_cast(r) *
         glm::scale(glm::mat4(1.0F), s);
}

void addMeshInstances(
    const std::vector<LoadedPrimitive>& primitives,
    const std::string& baseName,
    const glm::mat4& worldTransform,
    std::vector<GltfMaterialBatch>& batchesByMaterial) {
  for (std::size_t p = 0; p < primitives.size(); ++p) {
    const LoadedPrimitive& prim = primitives[p];
    auto& batch = batchesByMaterial[prim.materialIndex];
    batch.instances.push_back(GltfMeshInstance{
        .name = (primitives.size() > 1)
                  ? std::format("{}_prim{}", baseName, p)
                  : baseName,
        .mesh = prim.mesh,
        .material = batch.material,
        .transform = worldTransform,
    });
  }
}

void visitNode(
    const tg3_model& model,
    const int32_t nodeIdx,
    const glm::mat4& parentTransform,
    const std::vector<std::vector<LoadedPrimitive>>& meshPrimitives,
    std::vector<uint8_t>& visiting,
    std::vector<GltfMaterialBatch>& batchesByMaterial) {
  if (nodeIdx < 0 || std::cmp_greater_equal(nodeIdx, model.nodes_count)) {
    return;
  }
  const auto idx = static_cast<std::size_t>(nodeIdx);
  if (visiting[idx] != 0) {
    return;
  }
  visiting[idx] = 1;

  const tg3_node& node = model.nodes[idx];
  const glm::mat4 worldTransform = parentTransform * nodeLocalTransform(node);

  if (node.mesh >= 0 && std::cmp_less(node.mesh, meshPrimitives.size())) {
    const auto meshIdx = static_cast<uint32_t>(node.mesh);
    std::string name = toString(node.name);
    if (name.empty()) name = toString(model.meshes[meshIdx].name);
    if (name.empty()) name = std::format("mesh_{}", meshIdx);
    addMeshInstances(meshPrimitives[meshIdx], name, worldTransform,
                     batchesByMaterial);
  }

  for (uint32_t c = 0; c < node.children_count; ++c) {
    visitNode(model, node.children[c], worldTransform, meshPrimitives,
              visiting, batchesByMaterial);
  }

  visiting[idx] = 0;
}

std::vector<GltfMaterialBatch> buildBatches(
    const tg3_model& model,
    const std::vector<std::vector<LoadedPrimitive>>& meshPrimitives,
    const std::vector<std::shared_ptr<GltfMaterial>>& materials) {
  std::vector<GltfMaterialBatch> batchesByMaterial(materials.size());
  for (std::size_t i = 0; i < materials.size(); ++i) {
    batchesByMaterial[i].material = materials[i];
  }

  if (model.scenes_count > 0 && model.nodes_count > 0) {
    const int32_t sceneIdx =
        (model.default_scene >= 0 &&
         std::cmp_less(model.default_scene, model.scenes_count))
            ? model.default_scene
            : 0;
    const tg3_scene& scene = model.scenes[sceneIdx];
    std::vector<uint8_t> visiting(model.nodes_count, 0);
    for (const int32_t nodeIdx : std::span(scene.nodes, scene.nodes_count)) {
      visitNode(model, nodeIdx, glm::mat4(1.0F), meshPrimitives, visiting,
                batchesByMaterial);
    }
  } else if (model.nodes_count > 0) {
    std::vector<uint8_t> hasParent(model.nodes_count, 0);
    for (uint32_t n = 0; n < model.nodes_count; ++n) {
      const tg3_node& node = model.nodes[n];
      for (const int32_t childIdx :
           std::span(node.children, node.children_count)) {
        if (childIdx >= 0 && std::cmp_less(childIdx, model.nodes_count)) {
          hasParent[static_cast<std::size_t>(childIdx)] = 1;
        }
      }
    }

    std::vector<uint8_t> visiting(model.nodes_count, 0);
    for (uint32_t n = 0; n < model.nodes_count; ++n) {
      if (hasParent[n] == 0) {
        visitNode(model, static_cast<int32_t>(n), glm::mat4(1.0F),
                  meshPrimitives, visiting, batchesByMaterial);
      }
    }
  } else {
    for (uint32_t m = 0; const auto& mesh : std::span(
             model.meshes, model.meshes_count)) {
      std::string name = toString(mesh.name);
      if (name.empty()) name = std::format("mesh_{}", m);
      addMeshInstances(meshPrimitives[m], name, glm::mat4(1.0F),
                       batchesByMaterial);
      ++m;
    }
  }

  std::vector<GltfMaterialBatch> nonEmptyBatches;
  nonEmptyBatches.reserve(batchesByMaterial.size());
  for (auto& batch : batchesByMaterial) {
    if (!batch.instances.empty()) {
      nonEmptyBatches.push_back(std::move(batch));
    }
  }
  return nonEmptyBatches;
}
} // namespace

GltfAsset GltfLoader::load(const std::string& filename,
                           const Program& program) {
  const ParsedGltf parsed = parseFile(filename);
  const tg3_model& model = parsed.model;

  auto images =
      loadImages(model, std::filesystem::path(filename).parent_path());
  auto materials = loadMaterials(model, images, program);
  auto const buffers = loadBuffers(model);
  auto const meshPrimitives = loadMeshes(model, buffers, materials, program);

  GltfAsset asset;
  for (auto& tex : images) {
    if (tex != nullptr) {
      asset.textures.push_back(std::move(tex));
    }
  }
  asset.materials = std::move(materials);
  for (const auto& primitives : meshPrimitives) {
    for (const auto& [mesh, materialIndex] : primitives) {
      asset.meshes.push_back(mesh);
    }
  }
  asset.batches = buildBatches(model, meshPrimitives, asset.materials);
  return asset;
}