#pragma once

#include "ecs/component.hpp"
#include "core/math.hpp"

namespace unknown::ecs
{
    struct CTransform final : public Component<CTransform>
    {
    public:
        CTransform()
        {
            transform = Mat4f::Identity();
        }

        CTransform(Mat4f trans) : transform(trans) {}

        Mat4f transform;
        float yaw = 0.f;
        float pitch = 0.f;
        float roll = 0.f;
    };
}