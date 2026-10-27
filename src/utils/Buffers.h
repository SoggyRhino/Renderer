#pragma once
#include "glad/glad.h"


inline GLuint genVertexArray() {
    GLuint id = 0;
    glGenVertexArrays(1, &id);
    return id;
}

inline GLuint genBuffer() {
    GLuint id = 0;
    glGenBuffers(1, &id);
    return id;
}
