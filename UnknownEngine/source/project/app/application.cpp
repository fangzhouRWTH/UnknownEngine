#include "application.hpp"
#include "core/math.hpp"
#include "renderer/renderer.hpp"
#include "asset/assetManager.hpp"
#include "configuration/globalValues.hpp"

#include "ecs/entities/actorEntity.hpp"
#include "ecs/components/transformComponent.hpp"
#include "renderer/renderSystem/meshComponent.hpp"
#include "renderer/renderSystem/renderSystem.hpp"

#include "stringTable.hpp"

namespace unknown
{
    void Application_Default::AddSystems(ecs::Initializer &Initializer)
    {
    }

    void Application_Default::Initialize(EngineContext & context)
    {
        // asset::RenderElementAssetInfo assetInfo;
        // assetInfo.path = config::model_folder_path + "backpack/backpack.obj";
        // asset::LoadedRenderObject reObject = asset::AssetManager::RequestRenderObject(assetInfo);
        // renderer::RenderElementHandle rehandle1 = asset::AssetManager::RequestRenderElement(assetInfo);

        // asset::TextureAssetInfo txaInfo1;
        // txaInfo1.path = config::texture_folder_path + "container.jpg";
        // asset::TextureAssetInfo txaInfo2;
        // txaInfo2.path = config::texture_folder_path + "container2.png";
        // renderer::TextureHandle txhandle1 = asset::AssetManager::RequestTexture(txaInfo1);
        // renderer::TextureHandle txhandle2 = asset::AssetManager::RequestTexture(txaInfo2);
        // // unknown::renderer::Shader shader("test_vs.glsl", "test_fs.glsl");
        // asset::ProgramAssetInfo pgInfo;
        // pgInfo.vsPath = config::shader_folder_path + "phong_vs.glsl";
        // pgInfo.fsPath = config::shader_folder_path + "phong_fs.glsl";
        // renderer::ProgramHandle pgHandle = asset::AssetManager::RequestProgram(pgInfo);

        // auto object1_h = context.ecsManager->CreateEntity<ecs::EOpponent>("backpack");
        // reObject.programHandle = pgHandle;
        // Mat4f obj_trans = Mat4f::Identity();
        // auto obj_trans_h = context.ecsManager->CreateComponent<ecs::EOpponent, ecs::CTransform>(object1_h._entity_id, obj_trans);
        // auto obj_mesh_h = context.ecsManager->CreateComponent<ecs::EOpponent, ecs::CMesh>(object1_h._entity_id, reObject);

        // ecs::SystemHandle rSys = context.ecsManager->GetSystemHandle<ecs::SRender>(kEcsDefaultRenderSystem);
        // context.ecsManager->RegisterEntity<ecs::SRender, ecs::EOpponent>(rSys._system_id, object1_h._entity_id);
    }

    void Application_Default::Update(const EngineContext &context)
    {
    }

    void Application_Default::Shutdown()
    {
    }
}