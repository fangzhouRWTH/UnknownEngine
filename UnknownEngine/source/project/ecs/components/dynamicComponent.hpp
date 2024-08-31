#pragma once
#include "ecs/component.hpp"
#include "core/math.hpp"

namespace unknown::ecs
{
    struct CDynamic final : public Component<CDynamic>
    {
    public:
        Vec3f velocity;
        Vec3f acceleration;
    };
}