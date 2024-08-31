#pragma once
#include "ecs/component.hpp"
#include "core/math.hpp"
#include "context/engineContext.hpp"

namespace unknown::ecs
{
    struct CCamera final : public Component<CCamera>
    {
    public:
        float near = 0.1f;
        float far = 1000.f;
        float fov_degree = 70.f;

        ProjectionType projection = ProjectionType::Perspective;
    };
}