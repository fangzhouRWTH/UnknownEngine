#pragma once
#include "platform/type.hpp"
#include "system.hpp"
#include "memory/allocator.hpp"
#include "ecsdefines.hpp"
#include "context/engineContext.hpp"

#include <unordered_map>
#include <vector>

namespace unknown::ecs
{
    class ECSManager;

    class ISystemContainer
    {
    public:
        memory::PoolAllocator _system_allocator;
        typedef std::unordered_map<SystemID, ISystem *> SystemMap;

        SystemMap _system_map;

        ISystemContainer(u32 memSize, u32 blockSize) : _system_allocator(memSize, blockSize) {}

        ~ISystemContainer()
        {
            for (auto it : _system_map)
            {
                _system_allocator.Free(it.second);
            }
        }

        ISystem *GetSystem(const SystemID &systemId)
        {
            if (_system_map.find(systemId) == _system_map.end())
                return nullptr;

            return _system_map[systemId];
        }

        //virtual void Init() = 0;
        //virtual void Update(float dt) = 0;
    };

    template <typename T>
    class SystemContainer : public ISystemContainer
    {
    public:
        static const ClassID _class_id;

        SystemContainer() : ISystemContainer(sizeof(T) * MAX_SYSTEM_COUNT_PER_TYPE, sizeof(T)) {}
        ~SystemContainer()
        {
            for (auto it : _system_map)
            {
                _system_allocator.Free(it.second);
            }
        }

        template <typename... Args>
        SystemHandle CreateSystem(Args... args)
        {
            T *system = memory::Allocate<T>(_system_allocator, std::forward<Args>(args)...);
            //_system_order.push_back(system->_id);
            _system_map[system->_id] = system;
            return SystemHandle(system->_id, system->_class_id);
        }

        // void Init()
        // {
        //     // for (auto it : _system_order)
        //     // {
        //     //     auto sys = _system_map[it];
        //     //     sys->Init();
        //     // }
        // }

        // void Update(float dt)
        // {
        //     // for (auto it : _system_order)
        //     // {
        //     //     auto sys = _system_map[it];
        //     //     sys->Update(dt);
        //     // }
        // }
    };

    template <typename T>
    const ClassID SystemContainer<T>::_class_id = FamilyID<ISystemContainer>::Get<T>();

    class SystemManager
    {
    private:
        typedef std::unordered_map<ClassID, ISystemContainer *> SystemContainerMap;
        std::vector<ClassID> _container_order;
        SystemContainerMap _container_map;

        std::vector<ISystem *> _sorted_systems;
        bool isSorted = false;

        ECSManager *_ecs_manager_instance;

        template <typename T>
        SystemContainer<T> *GetSystemContainer()
        {
            auto it = _container_map.find(T::_class_id);
            SystemContainer<T> *container = nullptr;
            if (it == _container_map.end())
            {
                container = new SystemContainer<T>();
                _container_order.push_back(T::_class_id);
                _container_map[T::_class_id] = container;
            }
            else
            {
                container = static_cast<SystemContainer<T> *>(it->second);
            }
            return container;
        }

        ISystemContainer *GetSystemContainer(const ClassID &systemClassId)
        {
            auto it = _container_map.find(systemClassId);
            ISystemContainer *container = nullptr;
            if (it != _container_map.end())
            {
                container = it->second;
            }
            return container;
        }

    public:
        SystemManager(ECSManager *ecsManager) : _ecs_manager_instance(ecsManager) {}
        ~SystemManager()
        {
            for (auto it : _container_map)
            {
                delete it.second;
            }
        }

        bool OverrideSystemOrder(std::vector<SystemHandle> handles)
        {
            _sorted_systems.clear();
            isSorted = false;
            for (auto h : handles)
            {
                if (auto iSysC = GetSystemContainer(h._class_id); iSysC != nullptr)
                {
                    auto iSys = iSysC->GetSystem(h._system_id);
                    if (iSys != nullptr)
                        _sorted_systems.push_back(iSys);
                    else
                    {
                        _sorted_systems.clear();
                        return false;
                    }
                }
                else
                {
                    _sorted_systems.clear();
                    return false;
                }
            }

            isSorted = true;
            return true;
        };

        template <typename T, typename... Args>
        SystemHandle CreateSystem(Args... args)
        {
            SystemContainer<T> *container = GetSystemContainer<T>();

            auto sh = container->template CreateSystem(std::forward<Args>(args)...);
            auto sys = GetSystem<T>(sh._system_id);
            sys->_ecs_manager_ptr = _ecs_manager_instance;
            return sh;
        }

        template <typename T>
        ISystem *GetSystem(const SystemID &systemId)
        {
            SystemContainer<T> *container = GetSystemContainer<T>();
            return container->template GetSystem(systemId);
        }

        template <typename T>
        void RegisterEntity(const SystemID &systemId, const EntityID &entityId)
        {
            SystemContainer<T> *container = GetSystemContainer<T>();
            T *system = (T *)container->template GetSystem(systemId);
            system->RegisterEntity(entityId);
        }

        template <typename T>
        void UnregisterEntity(const SystemID &systemId, const EntityID &entityId)
        {
            SystemContainer<T> *container = GetSystemContainer<T>();
            T *system = (T *)container->template GetSystem<T>(systemId);
            system->UnregisterEntity(entityId);
        }

        void Init()
        {
            assert(isSorted);
            // for (auto it : _container_order)
            // {
            //     auto container = _container_map[it];
            //     container->Init();
            // }
            for (auto it : _sorted_systems)
            {
                it->Init();
            }
        }

        void Update(EngineContext & context)
        {
            // for (auto it : _container_order)
            // {
            //     auto container = _container_map[it];
            //     container->Update(dt);
            // }
            for (auto it : _sorted_systems)
            {
                it->Update(context);
            }
        }
    };
}