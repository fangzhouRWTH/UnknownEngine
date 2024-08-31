#include "family.hpp"
#include "ecs.hpp"

namespace unknown::ecs
{
    // class IEntity;
    // class IComponent;
    // class ISystem;

    // template class FamilyID<IEntity>;
    // template class FamilyID<IComponent>;
    // template class FamilyID<ISystem>;

    ECSManager::ECSManager() : mEntityManagerAllocator(sizeof(EntityManager), sizeof(EntityManager)),
                               mComponentManagerAllocator(sizeof(ComponentManager), sizeof(ComponentManager)),
                               mSystemManagerAllocator(sizeof(SystemManager), sizeof(SystemManager))
    {
        mpEntityManager = memory::Allocate<EntityManager>(mEntityManagerAllocator);
        mpComponentManager = memory::Allocate<ComponentManager>(mComponentManagerAllocator, this);
        mpSystemManager = memory::Allocate<SystemManager>(mSystemManagerAllocator, this);
    }

    void ECSManager::Initialize()
    {
        mpSystemManager->Init();
    }

    //std::shared_ptr<ECSManager> ECSManager::sInstancePtr = nullptr;

    // std::shared_ptr<ECSManager> ECSManager::Get()
    // {
    //     // TODO MT SAFE
    //     if (!sInstancePtr)
    //     {
    //         sInstancePtr = std::shared_ptr<ECSManager>(new ECSManager());
    //     }
    //     return sInstancePtr;
    // }
}