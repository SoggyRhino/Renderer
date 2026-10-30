#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

#include <iostream>
#include <format>

#include "stb_image.h"

Texture::Texture(const std::filesystem::path &filePath) {
    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(filePath.string().c_str(), &width_, &height_, &channels_, 0);
    if (!data) {
        std::cerr << std::format("Warning: Failed to load texture '{}': {}\n",
                                 filePath.string(), stbi_failure_reason());
        return;
    }
    initGL(data);
    stbi_image_free(data);
}

Texture::Texture(const unsigned char *bufferData, size_t bufferSize) {
    if (!bufferData || bufferSize == 0) return;

    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load_from_memory(bufferData, static_cast<int>(bufferSize),
                                                &width_, &height_, &channels_, 0);
    if (!data) {
        std::cerr << std::format("Warning: Failed to load texture from memory: {}\n",
                                 stbi_failure_reason());
        return;
    }
    initGL(data);
    stbi_image_free(data);
}

Texture::~Texture() {
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

Texture::Texture(Texture &&other) noexcept
    : id_(other.id_),
      width_(other.width_),
      height_(other.height_),
      channels_(other.channels_) {
    other.id_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
        }
        id_ = other.id_;
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;

        other.id_ = 0;
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
    }
    return *this;
}

void Texture::initGL(const unsigned char *data) {
    GLenum internalFormat = GL_RGBA;
    GLenum dataFormat = GL_RGBA;

    switch (channels_) {
        case 1:
            internalFormat = GL_RED;
            dataFormat = GL_RED;
            break;
        case 2:
            internalFormat = GL_RG;
            dataFormat = GL_RG;
            break;
        case 3:
            internalFormat = GL_RGB;
            dataFormat = GL_RGB;
            break;
        case 4:
            internalFormat = GL_RGBA;
            dataFormat = GL_RGBA;
            break;
        default:
            std::cerr << std::format("Warning: Unsupported channel count: {}\n", channels_);
            return;
    }

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(GLuint unit) const {
    if (id_ == 0) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture::unbind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}
