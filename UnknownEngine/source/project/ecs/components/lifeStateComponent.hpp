#pragma once

#include "ecs/component.hpp"
#include "core/math.hpp"

namespace unknown::ecs
{
    struct CLifeState final : public Component<CLifeState>
    {
        public:
            float life;
            bool temperal;
            float lifetime;
    };
}