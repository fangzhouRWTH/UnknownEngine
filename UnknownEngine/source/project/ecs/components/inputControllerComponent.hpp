#pragma once
#include "ecs/component.hpp"
#include "core/math.hpp"

#include "ecs/components/transformComponent.hpp"
#include "framework/framework.hpp"

namespace unknown::ecs
{
    struct CInputController final : public Component<CInputController>
    {
    public:
        enum class ControllTarget
        {
            DynamicComponent,
            TransformComponent,
        };
        ControllTarget target = ControllTarget::TransformComponent;
        void (*controllTransformComponent)(CTransform *transformComponent, const input::KeyEvents &keyEvent,
                                           const input::CursorPosition &cursor, float dt) = nullptr;
    };
}