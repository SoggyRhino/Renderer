#pragma once

#include <glad/glad.h>
#include <glm/vec4.hpp>

struct PipelineState {
  glm::vec4 clearColor{0.2f, 0.3f, 0.3f, 1.0f};
  bool depthTest = true;
  bool blend = true;
  bool cullFace = false;
  bool wireframe = false;
};

class RenderState {
public:
  static void apply(const PipelineState& state) {
    setClearColor(state.clearColor);
    setDepthTest(state.depthTest);
    setBlend(state.blend);
    setCullFace(state.cullFace);
    setWireframe(state.wireframe);
  }

  static void clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  static void setClearColor(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  static void setDepthTest(const bool enabled) {
    enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
  }

  static void setBlend(const bool enabled) {
    if (enabled) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
      glDisable(GL_BLEND);
    }
  }

  static void setCullFace(const bool enabled) {
    enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  }

  static void setWireframe(const bool enabled) {
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
  }
};