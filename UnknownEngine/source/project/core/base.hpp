#pragma once
#include "platform/type.hpp"
#include "core/math.hpp"

namespace unknown
{
    struct Vertex
    {
        Vec3f position;
        float uv_x;
        Vec3f normal;
        float uv_y;
        Vec4f color;
    };
}