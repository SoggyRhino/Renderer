#include "Model.h"

#include <format>
#include <stdexcept>

#include "tiny_gltf_v3.h"

Model::Model(const std::string &filename) {
    tg3_parse_options opts;
    tg3_parse_options_init(&opts);

    tg3_error_stack errors;
    tg3_error_stack_init(&errors);

    tg3_model model = {.arena_ = nullptr};

    const int err = tg3_parse_file(&model, &errors, filename.c_str(), filename.size(), &opts);
    if (err != 0) {
        std::string details;
        for (uint32_t i = 0; i < errors.count; ++i) {
            if (i > 0) details += " | ";
            details += errors.entries[i].message ? errors.entries[i].message : "Unknown error";
        }
        tg3_error_stack_free(&errors);
        throw std::runtime_error(std::format("Could not load GLTF model '{}' (Code: {}): {}",
                                             filename, err, details.empty() ? "no details" : details));
    }

    for (uint32_t m = 0; m < model.meshes_count; ++m) {
        const tg3_mesh &mesh = model.meshes[m];
        for (uint32_t p = 0; p < mesh.primitives_count; ++p) {
            meshes.emplace_back(model, mesh.primitives[p]);
        }
    }

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

void Model::draw() const {
    for (const auto &mesh : meshes) {
        mesh.draw();
    }
}
