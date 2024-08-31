#pragma once

#include "memory/allocator.hpp"
#include "entity.hpp"
#include "platform/type.hpp"
#include <unordered_map>
#include "ecsdefines.hpp"

#include "debug/log.hpp"

namespace unknown::ecs
{
    class IEntityContainer
    {
    public:
        memory::PoolAllocator _entity_allocator;

        typedef std::unordered_map<EntityID, IEntity *> EntityMap;
        EntityMap _entity_map;

        IEntityContainer(u32 allocatorSize, u32 blockSize) : _entity_allocator(allocatorSize, blockSize) {}
        ~IEntityContainer()
        {
            for (auto it : _entity_map)
            {
                //INFO_LOG("Destroy Entity : id - {} -", it.second->_id);
                _entity_allocator.Free(it.second);
            }
        }
    };

    template <typename T>
    class EntityContainer : public IEntityContainer
    {
    public:
        EntityContainer() : IEntityContainer(sizeof(T) * MAX_ENTITY_COUNT_PER_TYPE, sizeof(T)) {}

        static const ClassID _class_id;

        template <typename... Args>
        EntityHandle CreateEntity(Args... args)
        {
            T *entity = memory::Allocate<T>(_entity_allocator, std::forward<Args>(args)...);

            _entity_map[entity->_id] = entity;

            return EntityHandle(entity->_id, entity->_class_id);
        }

        void DeleteEntity(const EntityID &id)
        {
            T *entity = static_cast<T *>(GetEntity<T>(id));
            if (entity)
            {
                memory::Delete<T>(_entity_allocator, entity);
                _entity_map.erase(id);
            }
        }

        IEntity *GetEntity(const EntityID &id)
        {
            if (_entity_map.find(id) == _entity_map.end())
                return nullptr;

            return _entity_map[id];
        }
    };

    template <typename T>
    const ClassID EntityContainer<T>::_class_id = FamilyID<IEntityContainer>::Get<T>();

    class EntityManager
    {
        typedef std::unordered_map<ClassID, IEntityContainer *> EntityContainerMap;

        EntityContainerMap containerMap;

        template <typename T>
        EntityContainer<T>* GetEntityContainer()
        {
            auto it = containerMap.find(T::_class_id);
            EntityContainer<T> *container = nullptr;

            if (it == containerMap.end())
            {
                container = new EntityContainer<T>();
                containerMap[T::_class_id] = container;
            }
            else
            {
                container = static_cast<EntityContainer<T> *>(it->second);
            }

            return container;
        }

    public:
        ~EntityManager()
        {
            for (auto it : containerMap)
            {
                delete it.second;
            }
        }

        template <typename T, typename... Args>
        EntityHandle CreateEntity(Args... args)
        {
            EntityContainer<T> *container = GetEntityContainer<T>();

            return container->CreateEntity(std::forward<Args>(args)...);
        }

        template <typename T>
        EntityHandle CreateEntity()
        {
            EntityContainer<T> *container = GetEntityContainer<T>();

            return container->CreateEntity();
        }

        template <typename T>
        void DeleteEntity(const EntityID &id)
        {
            EntityContainer<T> *container = GetEntityContainer<T>();

            container->DeleteEntity(id);
        }
        
        template<typename T>
        IEntity* GetEntity(const EntityID& id)
        {
            EntityContainer<T>* container = GetEntityContainer<T>();
            return container->GetEntity(id);
        }
    };
}