#pragma once
#include "ecs/system.hpp"

namespace unknown::ecs
{
    class PreprocessSystem : public System<PreprocessSystem>
    {
    public:
        virtual void Init() override;
        virtual void Update(float dt) override;

    private:
        virtual void Setup() override;
    }
}