#pragma once

#include "entity.hpp"
#include "platform/type.hpp"
#include "mask.hpp"
#include "context/engineContext.hpp"

#include <vector>

namespace unknown::ecs
{
    class ECSManager;
    class SystemManager;

    class ISystem
    {
    public:
        friend class SystemManager;
        
        virtual void Init() = 0;
        virtual void Update(EngineContext & context) = 0;
        // ECSManager* ecsManagerPtr = nullptr;
        SystemID _id;
        ComponentsMask _components_mask;
        ISystem *Require(ClassID componentClassId)
        {
            _components_mask.Set(componentClassId);
            return this;
        }

    protected:
        ECSManager *_ecs_manager_ptr;

    private:
        virtual void Setup() = 0;
    };

    class SystemHandle
    {
    public:
        SystemHandle(SystemID systemId, ClassID classId) : _system_id(systemId), _class_id(classId) {}
        SystemID _system_id;
        ClassID _class_id;
    };

    template <typename T>
    class System : public ISystem
    {
    public:
        static const ClassID _class_id;
        std::vector<EntityID> _entities;

        System()
        {
            _id = _static_id_counter++;
        }

        virtual void Init() {};
        virtual void Update(EngineContext & context) {};

        void RegisterEntity(const EntityID &entityId)
        {
            _entities.push_back(entityId);
        }

        void UnregisterEntity(const EntityID &entityId)
        {
            for (u32 i = 0; i < _entities.size(); i++)
            {
                if (_entities[i] = entityId)
                {
                    _entities.erase(_entities.begin() + i);
                }
            }
        }

    private:
        virtual void Setup() {};
        static SystemID _static_id_counter;
    };

    template <typename T>
    const ClassID System<T>::_class_id = FamilyID<ISystem>::Get<T>();

    template <typename T>
    SystemID System<T>::_static_id_counter = 0u;
}