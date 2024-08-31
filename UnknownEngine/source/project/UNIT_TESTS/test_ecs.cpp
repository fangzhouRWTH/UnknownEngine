#include "catch2/catch_test_macros.hpp"
#include "ecs/ecs.hpp"
#include "ecs/entities/actorEntity.hpp"
#include "ecs/components/lifeStateComponent.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/components/dynamicComponent.hpp"
#include "ecs/systems/movementSystem.hpp"
#include "ecs/systems/renderSystem.hpp"

using namespace unknown;
using namespace ecs;

TEST_CASE("CLASS ID TEST", "[standard]")
{
    EntityHandle eh_player = ECSManager::Get()->CreateEntity<EPlayer>("player1", 20);
    EntityHandle eh_playerx = ECSManager::Get()->CreateEntity<EPlayer>("player2", 50);
    EntityHandle eh_opp = ECSManager::Get()->CreateEntity<EOpponent>("opponent1", 10);

    REQUIRE(static_cast<EPlayer *>(ECSManager::Get()->GetEntity<EPlayer>(eh_player._entity_id))->_class_id == 0u);
    REQUIRE(static_cast<EOpponent *>(ECSManager::Get()->GetEntity<EOpponent>(eh_opp._entity_id))->_class_id == 1u);

    ComponentHandle ch_trans_player = ECSManager::Get()->CreateComponent<CTransform, EPlayer>(eh_player._entity_id);
    ComponentHandle ch_trans_opp = ECSManager::Get()->CreateComponent<CTransform, EOpponent>(eh_opp._entity_id);
    ComponentHandle ch_life_player = ECSManager::Get()->CreateComponent<CLifeState, EPlayer>(eh_player._entity_id);
    ComponentHandle ch_life_opp = ECSManager::Get()->CreateComponent<CLifeState, EOpponent>(eh_opp._entity_id);

    REQUIRE(static_cast<CTransform *>(ECSManager::Get()->GetComponent<CTransform>(eh_player._entity_id))->_class_id == 0u);
    REQUIRE(static_cast<CTransform *>(ECSManager::Get()->GetComponent<CTransform>(eh_opp._entity_id))->_class_id == 0u);
    REQUIRE(static_cast<CLifeState *>(ECSManager::Get()->GetComponent<CLifeState>(eh_player._entity_id))->_class_id == 1u);
    REQUIRE(static_cast<CLifeState *>(ECSManager::Get()->GetComponent<CLifeState>(eh_opp._entity_id))->_class_id == 1u);

    SystemHandle sh_movement = ECSManager::Get()->CreateSystem<SMovement>();
    SystemHandle sh_render = ECSManager::Get()->CreateSystem<SRender>();

    REQUIRE(static_cast<SMovement *>(ECSManager::Get()->GetSystem<SMovement>(sh_movement._system_id))->_class_id == 0u);
    REQUIRE(static_cast<SRender *>(ECSManager::Get()->GetSystem<SRender>(sh_render._system_id))->_class_id == 1u);
}

TEST_CASE("INSTANCE ID TEST", "[standard]")
{
    EntityHandle eh_player1 = ECSManager::Get()->CreateEntity<EPlayer>("player1", 20);
    EntityHandle eh_player2 = ECSManager::Get()->CreateEntity<EPlayer>("player2", 30);

    REQUIRE(static_cast<EPlayer *>(ECSManager::Get()->GetEntity<EPlayer>(eh_player1._entity_id))->_id == 2u);
    REQUIRE(static_cast<EPlayer *>(ECSManager::Get()->GetEntity<EPlayer>(eh_player2._entity_id))->_id == 3u);

    ComponentHandle ch_transform_1 = ECSManager::Get()->CreateComponent<CTransform, EPlayer>(eh_player1._entity_id);
    ComponentHandle ch_transform_2 = ECSManager::Get()->CreateComponent<CLifeState, EPlayer>(eh_player1._entity_id);

    REQUIRE(static_cast<CTransform *>(ECSManager::Get()->GetComponent<CTransform>(eh_player1._entity_id))->_id == 2u);
    REQUIRE(static_cast<CLifeState *>(ECSManager::Get()->GetComponent<CLifeState>(eh_player1._entity_id))->_id == 2u);

    SystemHandle sh_movement_1 = ECSManager::Get()->CreateSystem<SMovement>();
    SystemHandle sh_movement_2 = ECSManager::Get()->CreateSystem<SMovement>();

    REQUIRE(static_cast<SMovement *>(ECSManager::Get()->GetSystem<SMovement>(sh_movement_1._system_id))->_id == 1u);
    REQUIRE(static_cast<SMovement *>(ECSManager::Get()->GetSystem<SMovement>(sh_movement_2._system_id))->_id == 2u);
}

TEST_CASE("ENTITY REGISTRATION TEST", "[standard]")
{
    SystemHandle sh_movement_1 = ECSManager::Get()->CreateSystem<SMovement>();
    EntityHandle eh_player_1 = ECSManager::Get()->CreateEntity<EPlayer>("player1",100);
    EntityHandle eh_player_2 = ECSManager::Get()->CreateEntity<EPlayer>("player2",500);
    ECSManager::Get()->CreateComponent<CTransform,EPlayer>(eh_player_1._entity_id);
    ECSManager::Get()->CreateComponent<CTransform,EPlayer>(eh_player_2._entity_id);
    ECSManager::Get()->CreateComponent<CDynamic,EPlayer>(eh_player_1._entity_id);
    ECSManager::Get()->CreateComponent<CDynamic,EPlayer>(eh_player_2._entity_id);
    ECSManager::Get()->RegisterEntity<SMovement,EPlayer>(sh_movement_1._system_id, eh_player_1._entity_id);
    ECSManager::Get()->RegisterEntity<SMovement,EPlayer>(sh_movement_1._system_id, eh_player_2._entity_id);
    SMovement* ptrSMovement = static_cast<SMovement*>(ECSManager::Get()->GetSystem<SMovement>(sh_movement_1._system_id));

    REQUIRE(ptrSMovement->_entities.at(0) == 4);
    REQUIRE(ptrSMovement->_entities.at(1) == 5);
}