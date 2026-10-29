#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include <format>
#include <ranges>
#include <stdexcept>

#include "tiny_gltf_v3.h"

#include "Program.h"
#include "Model.h"


namespace {
    const std::unordered_map<std::string, GLuint> kAttributeLocations = {
        {"POSITION", 0},
        {"NORMAL", 1},
        {"TEXCOORD_0", 2},
    };
}

Model::Model(const std::string &filename) {
    tg3_parse_options opts;
    tg3_parse_options_init(&opts);

    tg3_error_stack errors;
    tg3_error_stack_init(&errors);

    tg3_model model = {.arena_ = nullptr};

    const int err = tg3_parse_file(&model, &errors, filename.c_str(), filename.size(), &opts);
    if (err != 0) {
        std::string details;
        for (uint32_t i = 0; i < errors.count; ++i) {
            if (i > 0) details += " | ";
            details += errors.entries[i].message ? errors.entries[i].message : "Unknown error";
        }
        tg3_error_stack_free(&errors);
        throw std::runtime_error(std::format("Could not load GLTF model '{}' (Code: {}): {}",
                                             filename, err, details.empty() ? "no details" : details));
    }

    const tg3_primitive *prim = &model.meshes[0].primitives[0];

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    for (uint32_t i = 0; i < prim->attributes_count; ++i) {
        const std::string name(prim->attributes[i].key.data, prim->attributes[i].key.len);
        const auto loc = kAttributeLocations.find(name);
        if (loc == kAttributeLocations.end()) continue;

        const tg3_accessor *acc = &model.accessors[prim->attributes[i].value];
        const tg3_buffer_view *view = &model.buffer_views[acc->buffer_view];
        const tg3_buffer *buf = &model.buffers[view->buffer];

        GLuint vbo;
        glGenBuffers(1, &vbo);
        VBOs.push_back(vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, view->byte_length,
                     buf->data.data + view->byte_offset, GL_STATIC_DRAW);

        glVertexAttribPointer(loc->second, acc->type, acc->component_type,
                              acc->normalized ? GL_TRUE : GL_FALSE,
                              view->byte_stride,
                              reinterpret_cast<void *>(static_cast<intptr_t>(acc->byte_offset)));
        glEnableVertexAttribArray(loc->second);

        attributes[name] = loc->second;
    }

    const tg3_accessor *idxAcc = &model.accessors[prim->indices];
    const tg3_buffer_view *idxView = &model.buffer_views[idxAcc->buffer_view];
    const tg3_buffer *idxBuf = &model.buffers[idxView->buffer];

    indexCount = idxAcc->count;
    indexType = idxAcc->component_type;
    indexOffset = static_cast<uintptr_t>(idxAcc->byte_offset);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxView->byte_length,
                 idxBuf->data.data + idxView->byte_offset, GL_STATIC_DRAW);

    glBindVertexArray(0);

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

Model::~Model() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(static_cast<GLsizei>(VBOs.size()), VBOs.data());
    glDeleteVertexArrays(1, &VAO);
    for (const auto &id: textures | std::views::values) glDeleteTextures(1, &id);
}

void Model::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), indexType,
                   reinterpret_cast<void *>(indexOffset));
    glBindVertexArray(0);
}