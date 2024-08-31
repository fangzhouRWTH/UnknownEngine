#pragma once

#include "iobject.hpp"
#include "core/math.hpp"

namespace unknown
{
    class Camera : public IObject
    {
    public:
        
    private:
        Vec3f mPosition = {0.f,0.f,0.f};
        Vec3f mUp = {0.f,0.f,1.f};
        Vec3f mForward = {1.f,0.f,0.f};
    };
}