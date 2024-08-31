#pragma once

#include <map>
#include "memory/allocator.hpp"
#include <unordered_map>
#include "component.hpp"
#include "entity.hpp"
#include "ecsdefines.hpp"
#include "ecsHelper.hpp"

namespace unknown::ecs
{
    class ECSManager;

    class IComponentContainer
    {
    public:
        typedef std::map<UniqueID, IComponent *> EntityComponentMap;
        EntityComponentMap _entity_component_map;
        memory::PoolAllocator _component_allocator;
        IComponentContainer(u32 memsize, u32 blockSize) : _component_allocator(memsize, blockSize)
        {
        }

        ~IComponentContainer()
        {
            for (auto it : _entity_component_map)
            {
                _component_allocator.Free(it.second);
            }
        }
    };

    template <typename T>
    class ComponentContainer : public IComponentContainer
    {
    private:
        ECSManager *_ecs_manager_instance;

    public:
        ComponentContainer(ECSManager *ecsptr) : IComponentContainer(sizeof(T) * MAX_COMPONENT_COUNT_PER_TYPE, sizeof(T)) { _ecs_manager_instance = ecsptr; }
        static const ClassID _class_id;

        template <typename... Args>
        ComponentHandle CreateComponent(EntityID entityID, Args... args)
        {
            // auto it = _entity_component_map.find(entityID);
            // if(it!=_entity_component_map.end())
            //     return

            T *component = memory::Allocate<T>(_component_allocator, std::forward<Args>(args)...);
            _entity_component_map[entityID] = component;

            return ComponentHandle(component->_id, component->_class_id);
        }

        //template <typename E>
        ComponentHandle CreateComponent(EntityID entityID)
        {
            T *component = memory::Allocate<T>(_component_allocator);
            _entity_component_map[entityID] = component;

            return ComponentHandle(component->_id, component->_class_id);
        }

        IComponent *GetComponent(const EntityID &entityID)
        {
            if (_entity_component_map.find(entityID) == _entity_component_map.end())
                return nullptr;

            return _entity_component_map[entityID];
        }

        void DeleteComponent(const ComponentHandle &componentHandle, const EntityID &entityID)
        {
            T *component = static_cast<T *>(GetComponent<T>(entityID));
            if (component)
            {
                _entity_component_map.erase(entityID);
                memory::Delete<T>(_component_allocator, component);
            }
        }
    };

    template <typename T>
    const ClassID ComponentContainer<T>::_class_id = FamilyID<IComponentContainer>::Get<T>();

    class ComponentManager
    {
    private:
        ECSManager *_ecs_manager_instance;

    public:
        ComponentManager(ECSManager *ecsInstance) : _ecs_manager_instance(ecsInstance) {}

        typedef std::unordered_map<ClassID, IComponentContainer *> ComponentContainerMap;
        ComponentContainerMap _container_map;

        template <typename C>
        ComponentContainer<C> *GetComponentContainer()
        {
            auto it = _container_map.find(C::_class_id);
            ComponentContainer<C> *container = nullptr;

            if (it == _container_map.end())
            {
                container = new ComponentContainer<C>(_ecs_manager_instance);
                _container_map[C::_class_id] = container;
            }
            else
            {
                container = static_cast<ComponentContainer<C> *>(it->second);
            }

            return container;
        }

        ~ComponentManager()
        {
            for (auto it : _container_map)
            {
                delete it.second;
            }
        }

        template <typename E, typename C, typename... Args>
        ComponentHandle CreateComponent(EntityID entityId, Args... args)
        {
            ComponentContainer<C> *container = GetComponentContainer<C>();
            return container->CreateComponent(entityId, std::forward<Args>(args)...);
        }

        template <typename C>
        void DeleteComponent(const ComponentHandle &componentHandle, const EntityID &entityId)
        {
            ComponentContainer<C> *container = GetComponentContainer<C>();
            container->template DeleteComponent(componentHandle, entityId);
        }

        template <typename C>
        IComponent *GetComponent(const EntityID &entityId)
        {
            ComponentContainer<C> *container = GetComponentContainer<C>();
            return container->template GetComponent(entityId);
        }
    };
}