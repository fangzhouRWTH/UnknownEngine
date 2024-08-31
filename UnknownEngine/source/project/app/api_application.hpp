#pragma once
#include "context/engineContext.hpp"
#include "ecs/ecs.hpp"

namespace unknown
{
    class IApplication
    {
    public:
        virtual void AddSystems(ecs::Initializer & Initializer) = 0;
        virtual void Initialize(EngineContext &context) = 0;
        virtual void Update(const EngineContext &context) = 0;
        virtual void Shutdown() = 0;

    private:
    };
}