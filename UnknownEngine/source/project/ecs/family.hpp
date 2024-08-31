#pragma once
#include "platform/type.hpp"

namespace unknown::ecs
{
    template <typename T>
    class FamilyID
    {
    public:
        template <typename Class>
        static const ClassID Get()
        {
            static const ClassID id = _registered_class_count++;
            return id;
        }

        static const ClassID GetRegisteredClassCount()
        {
            return _registered_class_count;
        }

    private:
        static u32 _registered_class_count;
    };

    template<typename Class>
    u32 FamilyID<Class>::_registered_class_count = 0u;
}