#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Program.h"
#include <format>
#include <string>

Program::~Program() {
    if (this->id != 0) {
        glDeleteProgram(this->id);
    }
}

Program::Program(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath) {
    // If frag fails, vert's destructor automatically calls glDeleteShader!
    const Shader vert(GL_VERTEX_SHADER, vertPath);
    const Shader frag(GL_FRAGMENT_SHADER, fragPath);

    this->id = glCreateProgram();
    glAttachShader(this->id, vert.id);
    glAttachShader(this->id, frag.id);
    glLinkProgram(this->id);

    GLint success = 0;
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(logLength > 0 ? logLength : 1, '\0');
        glGetProgramInfoLog(this->id, logLength, nullptr, infoLog.data());

        glDeleteProgram(this->id);
        this->id = 0;
        throw std::runtime_error(std::format("Program linking error: {}", infoLog));
    }
}


GLuint Program::createShader(const GLenum type, const std::filesystem::path& shaderPath) {
    std::ifstream file(shaderPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Could not open file: {}", shaderPath.string()));
    }

    const std::string contents((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    const char* source = contents.c_str();

    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(logLength > 0 ? logLength : 1, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());

        glDeleteShader(shader);

        const char* shaderTypeStr = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
        throw std::runtime_error(std::format("{} shader compilation error ({}): {}",
                                             shaderTypeStr, shaderPath.string(), infoLog));
    }
    return shader;
}

GLint Program::getUniformLocation(const std::string& name) const {
    if (const auto it = this->uniforms.find(name); it != this->uniforms.end()) {
        return it->second;
    }

    const GLint location = glGetUniformLocation(this->id, name.c_str());
    if (location == -1) {
        std::cerr << std::format("Warning: Uniform '{}' not found or optimized out.\n", name);
    }

    this->uniforms.emplace(name, location);
    return location;
}

void Program::use() const {
    glUseProgram(this->id);
}

void Program::setBool(const std::string &name, const bool value) const {
    setInt(name, value ? 1 : 0);
}

void Program::setInt(const std::string &name, const int value) const {
    glProgramUniform1i(this->id, getUniformLocation(name), value);
}

void Program::setUInt(const std::string &name, const uint32_t value) const {
    glProgramUniform1ui(this->id, getUniformLocation(name), value);
}

void Program::setFloat(const std::string &name, const float value) const {
    glProgramUniform1f(this->id, getUniformLocation(name), value);
}

void Program::set3fv(const std::string &name, const glm::vec3 value) const {
    glProgramUniform3fv(this->id, getUniformLocation(name), 1, glm::value_ptr(value));
}

void Program::setMatx4fv(const std::string &name, const glm::mat4 &value) const {
    glProgramUniformMatrix4fv(this->id, getUniformLocation(name),
                              1, GL_FALSE,
                              glm::value_ptr(value));
}
