#pragma once
#include "api_application.hpp"

namespace unknown
{
    class Application_Default final : public IApplication
    {
    public:
        virtual void AddSystems(ecs::Initializer & Initializer) override;
        virtual void Initialize(EngineContext &context) override;
        virtual void Update(const EngineContext & context) override;
        virtual void Shutdown() override;

    private:
    };
}