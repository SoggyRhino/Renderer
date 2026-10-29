#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "glad/glad.h"


class Program;

class Model {
public:
    explicit Model(const std::string &filename);

    ~Model();

    Model(const Model &) = delete;

    Model &operator=(const Model &) = delete;

    void draw() const;

    std::unordered_map<std::string, GLuint> textures;

private:
    GLuint VAO = 0, EBO = 0;
    std::vector<GLuint> VBOs;

    uint64_t indexCount = 0;
    int32_t indexType = 0;
    uintptr_t indexOffset = 0;

    std::unordered_map<std::string, GLuint> attributes;
};
