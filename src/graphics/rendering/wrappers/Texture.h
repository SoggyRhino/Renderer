#pragma once

#include <filesystem>
#include <glad/glad.h>

class Texture {
public:
  Texture() = default;
  explicit Texture(const std::filesystem::path& filePath);
  Texture(const unsigned char* bufferData, size_t bufferSize);

  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  void bind(GLuint unit = 0) const;
  void unbind(GLuint unit = 0) const;

  [[nodiscard]] GLuint id() const {
    return id_;
  }

  [[nodiscard]] int getWidth() const {
    return width_;
  }

  [[nodiscard]] int getHeight() const {
    return height_;
  }

  [[nodiscard]] int getChannels() const {
    return channels_;
  }

  [[nodiscard]] bool isValid() const {
    return id_ != 0;
  }

private:
  void initGL(const unsigned char* data);

  GLuint id_ = 0;
  int width_ = 0;
  int height_ = 0;
  int channels_ = 0;
};