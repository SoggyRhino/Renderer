#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

enum class Key : uint8_t {
  W,
  A,
  S,
  D,
  Space,
  LeftShift,
  Escape,
  Tab,
  Count
};

enum class KeyAction : uint8_t {
  Press,
  Release,
  Repeat
};

class InputState {
public:
  [[nodiscard]] bool held(const Key key) const {
    return heldKeys[static_cast<size_t>(key)];
  }

private:
  std::array<bool, static_cast<size_t>(Key::Count)> heldKeys{};

  void setHeld(const Key key, const bool isHeld) {
    heldKeys[static_cast<size_t>(key)] = isHeld;
  }

  void clear() {
    heldKeys.fill(false);
  }

  friend class Window;
};