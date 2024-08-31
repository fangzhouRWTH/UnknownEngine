#pragma once
#include "framework/framework.hpp"
#include "ecs/ecs.hpp"
#include "ecs/system.hpp"
#include "ecs/components/dynamicComponent.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/components/inputControllerComponent.hpp"

namespace unknown::ecs
{
    class SInputController final : public System<SInputController>
    {
    public:
        SInputController()
        {
            Setup();
        }

        virtual void Init() override
        {
        }

        virtual void Update(EngineContext & context) override
        {
            input::KeyEvents keyEvents = FrameworkManager::GetKeysEvents();
            input::CursorPosition cursor = FrameworkManager::GetCursorPosition();
            for (auto id : _entities)
            {
                CInputController *inputComp = static_cast<CInputController *>(_ecs_manager_ptr->GetComponent<CInputController>(id));
                switch (inputComp->target)
                {
                case CInputController::ControllTarget::TransformComponent:
                {
                    // TODO : SAFE
                    CTransform *transComp = static_cast<CTransform *>(_ecs_manager_ptr->GetComponent<CTransform>(id));
                    inputComp->controllTransformComponent(transComp,keyEvents,cursor,context.deltaTime);
                    assert(transComp);
                }
                break;
                case CInputController::ControllTarget::DynamicComponent:
                    assert(false);
                    break;
                default:
                    assert(false);
                }
            }
        }

    private:
        virtual void Setup() override
        {
            Require(CInputController::_class_id);
        }
    };
}