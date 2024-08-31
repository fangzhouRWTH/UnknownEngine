#pragma once
#include "platform/type.hpp"
#include "entityManager.hpp"
#include "componentManager.hpp"
#include "systemManager.hpp"
#include "memory/allocator.hpp"
#include "core/structure.hpp"
#include "context/engineContext.hpp"

#include <string>
#include <unordered_map>

#include <memory>
#include <vector>
#include <unordered_set>
#include <list>

namespace unknown::ecs
{
    class ECSManager
    {

    public:
        ECSManager();

        void Initialize();

        ~ECSManager()
        {
            memory::Delete<EntityManager>(mEntityManagerAllocator, mpEntityManager);
            memory::Delete<ComponentManager>(mComponentManagerAllocator, mpComponentManager);
            memory::Delete<SystemManager>(mSystemManagerAllocator, mpSystemManager);
        }

        template <typename T, typename... Args>
        EntityHandle CreateEntity(Args... args)
        {
            return mpEntityManager->CreateEntity<T>(std::forward<Args>(args)...);
        }

        template <typename E, typename C, typename S>
        void DeleteEntity(const EntityID entityId, ComponentHandle &componentHandle, const SystemID &systemId)
        {
            mpSystemManager->UnregisterEntity<S>(systemId, entityId);
            mpComponentManager->DeleteComponent<C>(componentHandle, entityId);
            mpEntityManager->DeleteEntity<E>(entityId);
        }

        template <typename E, typename C, typename... Args>
        ComponentHandle CreateComponent(EntityID entityId, Args... args)
        {
            E *entity = static_cast<E *>(mpEntityManager->GetEntity<E>(entityId));
            ClassID componentClassId = C::_class_id;
            ComponentsMask &cMask = entity->_components_mask;
            assert(!cMask.IsSet(componentClassId));
            cMask.Set(componentClassId);
            return mpComponentManager->CreateComponent<E, C>(entityId, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        SystemHandle CreateSystem(Args... args)
        {
            return mpSystemManager->CreateSystem<T>(std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        SystemHandle CreateNamedSystem(const std::string &name, Args... args)
        {
            auto it = mNamedSystemMap.find(name);
            assert(it == mNamedSystemMap.end());
            SystemHandle sh = CreateSystem<T>(std::forward<Args>(args)...);
            mNamedSystemMap.insert({name, sh});
            return sh;
        }

        template <typename T>
        IEntity *GetEntity(const EntityID &entityId)
        {
            return mpEntityManager->GetEntity<T>(entityId);
        }

        template <typename C>
        IComponent *GetComponent(const EntityID &entityId)
        {
            return mpComponentManager->GetComponent<C>(entityId);
        }

        template <typename S>
        ISystem *GetSystem(const SystemID &systemId)
        {
            return mpSystemManager->GetSystem<S>(systemId);
        }

        template <typename S>
        SystemHandle GetSystemHandle(const std::string &name)
        {
            auto it = mNamedSystemMap.find(name);
            assert(it != mNamedSystemMap.end());

            return it->second;
        }

        template <typename S, typename E>
        void RegisterEntity(const SystemID &systemId, const EntityID &entityId)
        {
            ComponentsMask cMask = GetEntity<E>(entityId)->_components_mask;
            ComponentsMask sys_cMask = GetSystem<S>(systemId)->_components_mask;
            assert(cMask.IsSet(sys_cMask));
            mpSystemManager->RegisterEntity<S>(systemId, entityId);
        }

        template <typename T>
        void UnregisterEntity(const SystemID &systemId, const EntityID &entityId)
        {
            mpSystemManager->UnregisterEntity<T>(systemId, entityId);
        }

        bool OverrideSystemOrder(std::vector<SystemHandle> order)
        {
            return mpSystemManager->OverrideSystemOrder(order);
        }

        void Update(EngineContext &context)
        {
            mpSystemManager->Update(context);
        }

    private:
        memory::PoolAllocator mEntityManagerAllocator;
        memory::PoolAllocator mComponentManagerAllocator;
        memory::PoolAllocator mSystemManagerAllocator;

        std::unordered_map<std::string, SystemHandle> mNamedSystemMap;
        EntityManager *mpEntityManager;
        ComponentManager *mpComponentManager;
        SystemManager *mpSystemManager;
    };

    class Initializer
    {
    private:
        structure::NodeGraph<std::string, SystemHandle> systemGraph;

    public:
        Initializer(std::shared_ptr<ECSManager> ecsManager) : manager(ecsManager) {}

        template <typename S, typename... Args>
        SystemHandle AddSystem(const std::string &sysName, Args... args)
        {
            auto handle = manager->CreateNamedSystem<S>(sysName, std::forward<Args>(args)...);

            systemGraph.AddNode(sysName, handle);

            return handle;
        }

        void AddDependency(const std::string &sysName, const std::string &dependsOn)
        {
            systemGraph.AddEdge(dependsOn, sysName);
        }

        bool BuildGraph()
        {
            if (!systemGraph.Build())
                return false;
            std::vector<SystemHandle> order = systemGraph.GetTopologicContentOrder();
            return manager->OverrideSystemOrder(order);
        }

    private:
        std::shared_ptr<ECSManager> manager;
        std::unordered_map<std::string, ecs::SystemHandle> sysMap;
    };
}