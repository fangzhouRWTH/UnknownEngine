#pragma once
#include "core/math.hpp"
#include "platform/type.hpp"

#include "family.hpp"

namespace unknown::ecs
{
    struct IComponent
    {
    public:
        ComponentID _id;
    };

    class ComponentHandle
    {
    public:
        ComponentHandle(ComponentID componentId, ClassID classId) : _component_id(componentId), _class_id(classId) {}
        ComponentID _component_id;
        ClassID _class_id;
    };

    template <typename ComponentClass>
    struct Component : public IComponent
    {
    public:
        static const ClassID _class_id;

        Component() { _id = _static_id_counter++; }

    private:
        static u32 _static_id_counter;
    };

    template<typename ComponentClass>
    const ClassID Component<ComponentClass>::_class_id = FamilyID<IComponent>::Get<ComponentClass>();

    template<typename ComponentClass>
    u32 Component<ComponentClass>::_static_id_counter = 0u;
}