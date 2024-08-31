#pragma once
#include "platform/type.hpp"

#define ECS_ENTITY_DEPARTMENT 1
#define ECS_COMPONENT_DEPARTMENT 2
#define ECS_SYSTEM_DEPARTMENT 3

namespace unknown::ecs
{
    inline UniqueID ecs_unique_id_encode(DepartmentID dpt, ClassID cls, LocalID id)
    {
        UniqueID _dpt = UniqueID(dpt) << 48;
        UniqueID _cls = UniqueID(cls) << 32;
        return _dpt | _cls | UniqueID(id);
    }
}