#pragma once

#include "SPlisHSPlasH/StaticRigidBody.h"

class DynamicRigidBody final : public SPH::StaticRigidBody {
public:
  [[nodiscard]] bool isDynamic() const override { return true; }
};