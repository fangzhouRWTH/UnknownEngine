#pragma once

#include "ecs/system.hpp"

#include "debug/log.hpp"

namespace unknown::ecs
{
    class SRender : public System<SRender>
    {
    public:
        SRender();

        virtual void Init() override;

        virtual void Update(EngineContext & context) override;

    private:
        virtual void Setup() override;
    };
}