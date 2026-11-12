#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "ImGuiLayer.h"
#include "graphics/runtime/Window.h"

ImGuiLayer::ImGuiLayer(Window& window) : window(window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window.nativeHandle(), true);
  ImGui_ImplOpenGL3_Init("#version 460");

  window.setKeyboardCaptureQuery(
      [] {
        return ImGui::GetIO().WantCaptureKeyboard;
      });
}

ImGuiLayer::~ImGuiLayer() {
  window.setKeyboardCaptureQuery({});
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::begin() const {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::end() const {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}