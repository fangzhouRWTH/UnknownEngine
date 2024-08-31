#pragma once
#include "ecs/system.hpp"
#include "debug/log.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/components/dynamicComponent.hpp"

namespace unknown::ecs
{
    class SMovement : public System<SMovement>
    {
    public:
        SMovement()
        {
            Setup();
        }

        virtual void Init() override
        {
            INFO_PRINT("ECS:System:Movement Initialized");
        }

        virtual void Update(float dt) override
        {
            // INFO_LOG("ECS:System:Movement Initialized");
        }

    private:
        virtual void Setup() override
        {
            Require(CTransform::_class_id);
            Require(CDynamic::_class_id);
        }
    };
}