#pragma once

class Window;

class ImGuiLayer {
public:
  explicit ImGuiLayer(Window& window);
  ~ImGuiLayer();

  ImGuiLayer(const ImGuiLayer&) = delete;
  ImGuiLayer& operator=(const ImGuiLayer&) = delete;

  void begin() const;
  void end() const;

private:
  Window& window;
};