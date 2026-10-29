#include "Mesh.h"

#include <string>
#include <unordered_map>
#include "tiny_gltf_v3.h"

namespace {
    const std::unordered_map<std::string, GLuint> kAttributeLocations = {
        {"POSITION", 0},
        {"NORMAL", 1},
        {"TEXCOORD_0", 2},
    };

    GLint getNumComponents(const int32_t tg3Type) {
        switch (tg3Type) {
            case TG3_TYPE_SCALAR: return 1;
            case TG3_TYPE_VEC2:   return 2;
            case TG3_TYPE_VEC3:   return 3;
            case TG3_TYPE_VEC4:   return 4;
            default:              return 0;
        }
    }
}

Mesh::Mesh(const tg3_model &model, const tg3_primitive &prim) {
    mode = (prim.mode >= 0) ? static_cast<GLenum>(prim.mode) : GL_TRIANGLES;

    VAO.bind();

    GLsizei vertexCount = 0;

    for (uint32_t i = 0; i < prim.attributes_count; ++i) {
        const std::string name(prim.attributes[i].key.data, prim.attributes[i].key.len);
        const auto loc = kAttributeLocations.find(name);
        if (loc == kAttributeLocations.end()) continue;

        if (prim.attributes[i].value < 0 ||
            static_cast<uint32_t>(prim.attributes[i].value) >= model.accessors_count) {
            continue;
        }

        const tg3_accessor *acc = &model.accessors[prim.attributes[i].value];

        if (name == "POSITION") {
            vertexCount = static_cast<GLsizei>(acc->count);
        }

        if (acc->buffer_view < 0 ||
            static_cast<uint32_t>(acc->buffer_view) >= model.buffer_views_count) {
            continue;
        }

        const tg3_buffer_view *view = &model.buffer_views[acc->buffer_view];
        if (view->buffer < 0 ||
            static_cast<uint32_t>(view->buffer) >= model.buffers_count) {
            continue;
        }

        const tg3_buffer *buf = &model.buffers[view->buffer];
        if (!buf->data.data) continue;

        const GLint numComponents = getNumComponents(acc->type);
        if (numComponents == 0) continue;

        Buffer &vbo = VBOs.emplace_back();
        vbo.setData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(view->byte_length),
                    buf->data.data + view->byte_offset, GL_STATIC_DRAW);

        glVertexAttribPointer(
            loc->second,
            numComponents,
            static_cast<GLenum>(acc->component_type),
            acc->normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(view->byte_stride),
            reinterpret_cast<void *>(static_cast<intptr_t>(acc->byte_offset))
        );
        glEnableVertexAttribArray(loc->second);
    }

    if (prim.indices >= 0 && static_cast<uint32_t>(prim.indices) < model.accessors_count) {
        const tg3_accessor *idxAcc = &model.accessors[prim.indices];
        if (idxAcc->buffer_view >= 0 &&
            static_cast<uint32_t>(idxAcc->buffer_view) < model.buffer_views_count) {
            const tg3_buffer_view *idxView = &model.buffer_views[idxAcc->buffer_view];
            if (idxView->buffer >= 0 &&
                static_cast<uint32_t>(idxView->buffer) < model.buffers_count) {
                const tg3_buffer *idxBuf = &model.buffers[idxView->buffer];
                if (idxBuf->data.data) {
                    hasIndices = true;
                    count = static_cast<GLsizei>(idxAcc->count);
                    indexType = static_cast<GLenum>(idxAcc->component_type);
                    indexOffset = static_cast<uintptr_t>(idxAcc->byte_offset);

                    EBO.emplace();
                    EBO->setData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(idxView->byte_length),
                                 idxBuf->data.data + idxView->byte_offset, GL_STATIC_DRAW);
                }
            }
        }
    }

    if (!hasIndices) {
        count = vertexCount;
    }

    VAO.unbind();
}

void Mesh::draw() const {
    if (count == 0) return;

    VAO.bind();
    if (hasIndices) {
        glDrawElements(mode, count, indexType, reinterpret_cast<void *>(indexOffset));
    } else {
        glDrawArrays(mode, 0, count);
    }
    VAO.unbind();
}
