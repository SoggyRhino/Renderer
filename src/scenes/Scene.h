#pragma once

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    virtual void update(double dt) = 0;
    virtual void render() const = 0;
};