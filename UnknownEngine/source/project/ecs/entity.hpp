#pragma once
#include "family.hpp"
#include "platform/type.hpp"
#include "mask.hpp"
#include "ecsHelper.hpp"

namespace unknown::ecs
{
    class IEntity
    {
    public:
        EntityID _id;
        ComponentsMask _components_mask;
    };

    struct EntityHandle
    {
        EntityHandle(EntityID entityId, ClassID classId) : _entity_id(entityId), _class_id(classId) {}
        EntityID _entity_id;
        ClassID _class_id;
    };

    template <typename T>
    class Entity : public IEntity
    {
    public:
        static const ClassID _class_id;
        Entity()
        {
            LocalID num = _static_id_counter++;
            _id = ecs_unique_id_encode(ECS_ENTITY_DEPARTMENT, _class_id, num);

            //_id = EntityID(_static_id_counter++);
        }

    private:
        static LocalID _static_id_counter;
    };

    template <typename T>
    const ClassID Entity<T>::_class_id = FamilyID<IEntity>::Get<T>();

    template <typename T>
    LocalID Entity<T>::_static_id_counter = 0u;
}