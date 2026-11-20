#pragma once

#include <algorithm>
#include <initializer_list>
#include <ranges>

#include <glad/glad.h>

struct VertexAttribute {
  GLuint index = 0;
  GLint size = 3;
  GLenum type = GL_FLOAT;
  GLboolean normalized = GL_FALSE;
  GLuint relativeOffset = 0;
};

class Buffer {
public:
  Buffer() {
    glCreateBuffers(1, &id_);
  }

  Buffer(
      const GLsizeiptr size,
      const void* data,
      const GLenum usage = GL_STATIC_DRAW) {
    glCreateBuffers(1, &id_);
    glNamedBufferData(id_, size, data, usage);
  }

  template <std::ranges::contiguous_range Range>
  explicit Buffer(const Range& data, const GLenum usage = GL_STATIC_DRAW)
    : Buffer(
          static_cast<GLsizeiptr>(std::ranges::size(data) *
                                  sizeof(std::ranges::range_value_t<Range>)),
          std::ranges::data(data),
          usage) {}

  Buffer(
      const GLenum target,
      const GLsizeiptr size,
      const void* data,
      const GLenum usage = GL_STATIC_DRAW) {
    glCreateBuffers(1, &id_);
    setData(target, size, data, usage);
  }

  template <std::ranges::contiguous_range Range>
  Buffer(
      const GLenum target,
      const Range& data,
      const GLenum usage = GL_STATIC_DRAW)
    : Buffer(
          target,
          static_cast<GLsizeiptr>(std::ranges::size(data) *
                                  sizeof(std::ranges::range_value_t<Range>)),
          std::ranges::data(data),
          usage) {}

  ~Buffer() {
    if (id_ != 0) {
      glDeleteBuffers(1, &id_);
    }
  }

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  Buffer(Buffer&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
  }

  Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {
      if (id_ != 0) {
        glDeleteBuffers(1, &id_);
      }
      id_ = other.id_;
      other.id_ = 0;
    }
    return *this;
  }

  void bind(const GLenum target) const {
    glBindBuffer(target, id_);
  }

  void unbind(const GLenum target) const {
    glBindBuffer(target, 0);
  }

  void setData(
      const GLsizeiptr size,
      const void* data,
      const GLenum usage = GL_STATIC_DRAW) const {
    glNamedBufferData(id_, size, data, usage);
  }

  template <std::ranges::contiguous_range Range>
  void setData(const Range& data, const GLenum usage = GL_STATIC_DRAW) const {
    setData(
        static_cast<GLsizeiptr>(std::ranges::size(data) *
                                sizeof(std::ranges::range_value_t<Range>)),
        std::ranges::data(data),
        usage);
  }

  void setData(
      const GLenum target,
      const GLsizeiptr size,
      const void* data,
      const GLenum usage = GL_STATIC_DRAW) const {
    glBindBuffer(target, id_);
    glBufferData(target, size, data, usage);
  }

  [[nodiscard]] GLuint id() const {
    return id_;
  }

private:
  GLuint id_ = 0;
};

class VertexArray {
public:
  VertexArray() {
    glCreateVertexArrays(1, &id_);
  }

  VertexArray(
      const Buffer& vbo,
      const GLsizei stride,
      const std::initializer_list<VertexAttribute> attributes,
      const Buffer* ebo = nullptr)
    : VertexArray() {
    configure(vbo, stride, attributes, ebo);
  }

  ~VertexArray() {
    if (id_ != 0) {
      glDeleteVertexArrays(1, &id_);
    }
  }

  VertexArray(const VertexArray&) = delete;
  VertexArray& operator=(const VertexArray&) = delete;

  VertexArray(VertexArray&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
  }

  VertexArray& operator=(VertexArray&& other) noexcept {
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

  void attachVertexBuffer(
      const Buffer& buffer,
      const GLsizei stride,
      const GLuint bindingIndex = 0,
      const GLintptr offset = 0) const {
    glVertexArrayVertexBuffer(id_, bindingIndex, buffer.id(), offset, stride);
  }

  void attachElementBuffer(const Buffer& buffer) const {
    glVertexArrayElementBuffer(id_, buffer.id());
  }

  void setAttributeFormat(
      const GLuint attribIndex,
      const GLint size,
      const GLenum type,
      const GLboolean normalized = GL_FALSE,
      const GLuint relativeOffset = 0) const {
    glEnableVertexArrayAttrib(id_, attribIndex);
    glVertexArrayAttribFormat(id_, attribIndex, size, type, normalized,
                              relativeOffset);
    glVertexArrayAttribBinding(id_, attribIndex, 0);
  }

  void configure(
      const Buffer& vbo,
      const GLsizei stride,
      const std::initializer_list<VertexAttribute> attributes,
      const Buffer* ebo = nullptr) const {
    attachVertexBuffer(vbo, stride);
    if (ebo != nullptr) {
      attachElementBuffer(*ebo);
    }
    std::ranges::for_each(attributes, [this](const VertexAttribute& attr) {
      setAttributeFormat(attr.index, attr.size, attr.type, attr.normalized,
                         attr.relativeOffset);
    });
  }

  void setAttributePointer(
      const GLuint index,
      const GLint size,
      const GLenum type,
      const GLboolean normalized,
      const GLsizei stride,
      const void* pointer) const {
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    glEnableVertexAttribArray(index);
  }

  void drawArrays(const GLenum mode, const GLint first, const GLsizei count) const {
    bind();
    glDrawArrays(mode, first, count);
    unbind();
  }

  void drawElements(
      const GLenum mode,
      const GLsizei count,
      const GLenum type,
      const void* indices = nullptr) const {
    bind();
    glDrawElements(mode, count, type, indices);
    unbind();
  }

  [[nodiscard]] GLuint id() const {
    return id_;
  }

private:
  GLuint id_ = 0;
};