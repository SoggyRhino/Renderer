//
// Created by Ian on 10/26/2026.
//

#include "SquareTestScene.h"

#include "Paths.h"
#include "../utils/Buffers.h"


SquareTestScene::SquareTestScene()
    : program(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D), VAO(genVertexArray()), VBO(genBuffer()) {
    const float vertices[] = {
        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 1.0f, 1.0f, 0.0f
    };

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

SquareTestScene::~SquareTestScene() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void SquareTestScene::update(double dt) {
}

void SquareTestScene::render() const {
    this->program.use();

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
