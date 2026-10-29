#pragma once

#include <glad/glad.h>

class VertexArray {
public:
    VertexArray() {
        glGenVertexArrays(1, &id_);
    }

    ~VertexArray() {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
        }
    }

    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;

    VertexArray(VertexArray &&other) noexcept : id_(other.id_) {
        other.id_ = 0;
    }

    VertexArray &operator=(VertexArray &&other) noexcept {
        if (this != &other) {
            if (id_ != 0) {
                glDeleteVertexArrays(1, &id_);
            }
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    void bind() const {
        glBindVertexArray(id_);
    }

    void unbind() const {
        glBindVertexArray(0);
    }

    [[nodiscard]] GLuint id() const { return id_; }
    [[nodiscard]] GLuint get() const { return id_; }

private:
    GLuint id_ = 0;
};

class Buffer {
public:
    Buffer() {
        glGenBuffers(1, &id_);
    }

    Buffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage = GL_STATIC_DRAW) {
        glGenBuffers(1, &id_);
        setData(target, size, data, usage);
    }

    ~Buffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other) noexcept : id_(other.id_) {
        other.id_ = 0;
    }

    Buffer &operator=(Buffer &&other) noexcept {
        if (this != &other) {
            if (id_ != 0) {
                glDeleteBuffers(1, &id_);
            }
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    void bind(GLenum target) const {
        glBindBuffer(target, id_);
    }

    void unbind(GLenum target) const {
        glBindBuffer(target, 0);
    }

    void setData(GLenum target, GLsizeiptr size, const void *data, GLenum usage = GL_STATIC_DRAW) const {
        glBindBuffer(target, id_);
        glBufferData(target, size, data, usage);
    }

    [[nodiscard]] GLuint id() const { return id_; }
    [[nodiscard]] GLuint get() const { return id_; }

private:
    GLuint id_ = 0;
};
