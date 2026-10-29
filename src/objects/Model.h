#pragma once

#include <string>
#include <vector>

#include "Mesh.h"

class Model {
public:
    explicit Model(const std::string &filename);
    ~Model() = default;

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    Model(Model &&) noexcept = default;
    Model &operator=(Model &&) noexcept = default;

    void draw() const;

    [[nodiscard]] const std::vector<Mesh> &getMeshes() const { return meshes; }
    [[nodiscard]] std::vector<Mesh> &getMeshes() { return meshes; }

private:
    std::vector<Mesh> meshes;
};
