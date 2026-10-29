#pragma once

#include <vector>
#include <glad/glad.h>

struct tg3_model;
struct tg3_primitive;

class Mesh {
public:
    Mesh(const tg3_model &model, const tg3_primitive &prim);
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other) noexcept;

    void draw() const;

    [[nodiscard]] GLuint getVAO() const { return VAO; }
    [[nodiscard]] GLsizei getCount() const { return count; }
    [[nodiscard]] bool getHasIndices() const { return hasIndices; }

private:
    GLuint VAO = 0;
    GLuint EBO = 0;
    std::vector<GLuint> VBOs;

    GLsizei count = 0;
    GLenum indexType = GL_UNSIGNED_INT;
    uintptr_t indexOffset = 0;
    GLenum mode = GL_TRIANGLES;
    bool hasIndices = false;
};
