#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Program {
public:
    Program(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath);

    ~Program();

    void use() const;

    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setUInt(const std::string &name, uint32_t value) const;

    void setFloat(const std::string &name, float value) const;

    void set3fv(const std::string &name, glm::vec3 value) const;

    void setMatx4fv(const std::string &name, const glm::mat4 &value) const;

private:
    GLuint id = 0;
    mutable std::unordered_map<std::string, GLint> uniforms;

    GLint getUniformLocation(const std::string &name) const;

    static GLuint createShader(GLenum type, const std::filesystem::path &shaderPath);

    struct Shader {
        GLuint id = 0;

        Shader(const GLenum type, const std::filesystem::path &path)
            : id(createShader(type, path)) {
        }

        ~Shader() {
            if (id != 0)
                glDeleteShader(id);
        }
    };
};
