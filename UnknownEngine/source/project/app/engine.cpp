#include "engine.hpp"
#include "getApp.hpp"
#include "api_application.hpp"

#include "world/scene.hpp"
#include "ecs/ecs.hpp"
#include "framework/framework.hpp"
#include "renderer/renderer.hpp"
#include "core/hash.hpp"

#include "ecs/components/cameraComponent.hpp"
#include "ecs/components/inputControllerComponent.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/entities/actorEntity.hpp"
#include "ecs/systems/cameraSystem.hpp"
#include "ecs/systems/inputControllerSystem.hpp"

#include "renderer/renderSystem/renderSystem.hpp"
#include "renderer/renderSystem/meshComponent.hpp"

#include "configuration/globalValues.hpp"
#include "world/scene.hpp"

#include "asset/assetManager.hpp"

#include "debug/log.hpp"

#include "app/stringTable.hpp"

#include "renderer/gui/simpleGUI.hpp"

#include "renderer/material/materials.hpp"
#include "renderer/material/materialManager.hpp"

#include <memory>


namespace unknown
{
    std::vector<renderer::RenderObject> rObjects;

    void DefaultPlayerController(unknown::ecs::CTransform *transformComponent, const unknown::input::KeyEvents &keyEvent,
                                 const unknown::input::CursorPosition &cursor, float dt)
    {
        float speed = 10.f;

        auto dx = cursor._current.x() - cursor._previous.x();
        auto dy = cursor._current.y() - cursor._previous.y();
        float yaw = dx * dt * 0.01f * 3.14159f;
        float pitch = dy * dt * 0.01f * 3.14159f;
        transformComponent->yaw += yaw;
        float npitch = unknown::math::clamp(transformComponent->pitch + pitch, -M_HPI, M_HPI);
        pitch = npitch - transformComponent->pitch;
        transformComponent->pitch = npitch;
        unknown::Mat4f linear = unknown::Mat4f::Identity();

        linear.block<3, 3>(0, 0) = transformComponent->transform.block<3, 3>(0, 0);

        linear = unknown::math::Rotate(linear, unknown::Vec3f(0.f, 0.f, -1.f), yaw);
        unknown::Vec3f right = unknown::Vec3f(linear.col(0).x(), linear.col(0).y(), linear.col(0).z());
        linear = unknown::math::Rotate(linear, -right, pitch);

        if (keyEvent.down.IsSet(unknown::input::Key::W))
        {
            unknown::Vec4f move = linear.col(1);
            transformComponent->transform.col(3) += move * speed * dt;
        }
        if (keyEvent.down.IsSet(unknown::input::Key::S))
        {
            unknown::Vec4f move = linear.col(1);
            transformComponent->transform.col(3) -= move * speed * dt;
        }
        if (keyEvent.down.IsSet(unknown::input::Key::D))
        {
            unknown::Vec4f move = linear.col(0);
            transformComponent->transform.col(3) += move * speed * dt;
        }
        if (keyEvent.down.IsSet(unknown::input::Key::A))
        {
            unknown::Vec4f move = linear.col(0);
            transformComponent->transform.col(3) -= move * speed * dt;
        }

        transformComponent->transform.block<3, 3>(0, 0) = linear.block<3, 3>(0, 0);
        auto t = transformComponent->transform;
        // INFO_LOG("TRANS: X: {}   Y: {}   Z: {}", t.col(3).x(), t.col(3).y(), t.col(3).z());
        //  INFO_LOG("FORWARD: X: {}   Y: {}   Z: {}",linear.col(1).x(),linear.col(1).y(),linear.col(1).z());
        //  INFO_LOG("dX: {}   dY: {} ", dx, dy);
        //  INFO_LOG("yaw: {}   pitch: {} ", yaw, pitch);
    };

    void Engine::Initialize()
    {
        // Engine Initialize
        // loadConfig();
        // loadScene();

        // Framework initialization
        {
            FrameworkInfo fInfo;
            fInfo.cursorMode = input::CursorMode::Default;
            fInfo.name = "UnknownEngine V 0 0 0";
            fInfo.width = 1920u;
            fInfo.height = 1080u;
            FrameworkManager::Initialize(fInfo);
        }

        // Renderer
        {
            renderer::RendererInitInfo rInfo;
            rInfo.windowPtr = FrameworkManager::GetWindowRawPointer();
            // Temp Vulkan Test
            mpRenderer = std::make_shared<renderer::RenderEngine>(renderer::EGraphicAPI::Vulkan);
            mpRenderer->Initialize(rInfo);

            renderer::ui::IMGUI_VULKAN_GLFW::InitializeVulkan(
                mpRenderer->GetCore(), 
                FrameworkManager::GetWindowRawPointer(), 
                true);
        }

        // AssetManager
        {
            mpAssetManager = std::make_shared<asset::AssetManager>(mpRenderer);
            mpAssetManager->Initialize();
        }

        //MaterialManager
        {
            mpMaterialManager = std::make_shared<renderer::MaterialManager>(mpRenderer);
            mpMaterialManager->Initialize();
        }

        // Scene
        {
            SceneLoaderContext sceneLoaderContext;
            sceneLoaderContext.managerPtr = mpAssetManager;
            sceneLoaderContext.sceneFilePath = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/scene/scene_default.json";
            
            SceneLoader::LoadScene(sceneLoaderContext);
        }

        // ECS
        {
            mpEcsManager = std::shared_ptr<ecs::ECSManager>(new ecs::ECSManager);
            ecs::Initializer initializer(mpEcsManager);

            auto controllersystem_h = initializer.AddSystem<ecs::SInputController>(kEcsDefaultInputControllerSystem);
            auto camerasystem_h = initializer.AddSystem<ecs::SCamera>(kEcsDefaultCameraSystem);
            auto rendersystem_h = initializer.AddSystem<ecs::SRender>(kEcsDefaultRenderSystem);

            initializer.AddDependency(kEcsDefaultRenderSystem, kEcsDefaultCameraSystem);
            // Build Systems Depandency
            mpApp = Application::GetApplication();
            mpApp->AddSystems(initializer);
            bool buildRes = initializer.BuildGraph();
            assert(buildRes);

            mpEcsManager->Initialize();

            // Default Systems
            auto player_default_h = mpEcsManager->CreateEntity<ecs::EPlayer>("player_default");
            mpEcsManager->CreateComponent<ecs::EPlayer, ecs::CTransform>(player_default_h._entity_id);

            // temp
            auto pTrans = mpEcsManager->GetComponent<ecs::CTransform>(player_default_h._entity_id);
            static_cast<ecs::CTransform*>(pTrans)->transform.col(3) = Vec4f(0.f,-2.8f,0.f,1.f);
            // temp

            mpEcsManager->CreateComponent<ecs::EPlayer, ecs::CCamera>(player_default_h._entity_id);
            mpEcsManager->CreateComponent<ecs::EPlayer, ecs::CInputController>(player_default_h._entity_id);

            auto controller = static_cast<ecs::CInputController *>(mpEcsManager->GetComponent<ecs::CInputController>(player_default_h._entity_id));
            controller->controllTransformComponent = DefaultPlayerController;
            controller->target = ecs::CInputController::ControllTarget::TransformComponent;

            mpEcsManager->RegisterEntity<ecs::SInputController, ecs::EPlayer>(controllersystem_h._system_id, player_default_h._entity_id);
            mpEcsManager->RegisterEntity<ecs::SCamera, ecs::EPlayer>(camerasystem_h._system_id, player_default_h._entity_id);
        }

        // Asset
        {
            //std::string modelPath = config::asset_folder_path + "models/structure/structure_.glb";
            std::string modelPath = config::asset_folder_path + "models/test/ball.glb";
            h64 h = math::HashString(modelPath);

            auto sceneData = mpAssetManager->GetSceneTree(h);
            
            rObjects.clear();

            if(sceneData)
            {
                // temp
                std::function<void(std::shared_ptr<SceneTree>, SceneNodeIndex, Mat4f)> stRecursive;
                stRecursive = [&](std::shared_ptr<SceneTree> tree, SceneNodeIndex parent, Mat4f transform)
                {
                    std::vector<SceneNodeIndex> childs;
                    tree->GetChilds(parent, childs);
                    for (auto ci : childs)
                    {
                        auto nPtr = tree->GetNode(ci);
                        assert(nPtr);

                        if (std::shared_ptr<SceneEmptyNode> eNode = std::dynamic_pointer_cast<SceneEmptyNode>(nPtr); eNode)
                        {
                            Mat4f newTransform = eNode->transform * transform;
                            //Mat4f newTransform = Mat4f::Identity();
                            stRecursive(tree, ci, newTransform);
                        }
                        else if (std::shared_ptr<SceneMeshNode> mNode = std::dynamic_pointer_cast<SceneMeshNode>(nPtr); mNode)
                        {
                            // draw
                            h64 meshHash = mNode->data.AssetHash;
                            std::shared_ptr<asset::MeshData> meshData = mpAssetManager->GetMeshData(meshHash);
                            
                            if (!meshData || !meshData->uploaded)
                                continue;

                            renderer::RenderObject rObject;
                            rObject.meshBufferHandle = meshData->meshBufferHandle;
                            rObject.transform = transform;
                            rObject.indicesCount = meshData->indices.size();

                            //temp
                            rObject.materialKey = renderer::ToyMaterial::GetMaterialInfo().GetKey();

                            rObjects.push_back(rObject);
                        }
                    }
                };

                auto rNode = sceneData->GetNode(sceneData->RootIndex());
                auto rEmpty = std::dynamic_pointer_cast<SceneEmptyNode>(rNode);
                assert(rEmpty);
                Mat4f rTransform = rEmpty->transform;

                float distX = 5.0;
                float distY = 5.0;
                float distZ = 5.0;

                Vec4f xv = Vec4f(distX,0.0,0.0,0.0);
                Vec4f zv = Vec4f(0.0,0.0,distZ,0.0);
                Vec4f yv = Vec4f(0.0,distY,0.0,0.0);

                u32 countX = 20u;
                u32 countZ = 10u;
                u32 countY = 20u;

                Vec4f startV = Vec4f(-distX * countX/2.0f, -distY * countY/2.0f, -distZ * countZ/2.0f,1.0);

                for(u32 i = 0u; i < countX; i++)
                {
                    for(u32 n = 0u; n < countZ; n++)
                    {
                        for(u32 j = 0u; j < countY; j++)
                        {
                            Vec4f dv = xv * i + yv * j + zv * n + startV;
                            Mat4f matGrid = Mat4f::Identity();
                            matGrid.col(3) = dv;
                            matGrid = rTransform * matGrid;

                            stRecursive(sceneData,sceneData->RootIndex(),matGrid);
                        }
                    }
                }

                // test indirect draw
                if(!rObjects.empty())
                {
                    //rObjects[0].indicesCount
                    mpRenderer->TestInitIndirectDraw(rObjects[0].indicesCount);
                }

                //stRecursive(sceneData,sceneData->RootIndex(),rTransform);
            }
        }

        // App Initialize
        {
            EngineContext context;
            context.ecsManager = mpEcsManager;

            // TODO temp comment out for faster launching
            // mpApp->Initialize(context);
        }
    }

    void Engine::Run()
    {
        // renderer::RenderStates states;
        // states.set(renderer::RenderStates::State::DepthTest);
        // states.set(renderer::RenderStates::State::BackCulling);
        // renderer::GraphicBackend::SetRenderStates(states);
        mEngineClock.Activate();

        float f1, f2, f3, f4;

        while (FrameworkManager::RunningMain())
        {
            renderer::ui::IMGUI_VULKAN_GLFW::NewFrame();

            float dt = float(mEngineClock.CheckDeltaTime().count());

            FrameworkManager::Update();
            u32 windowHeight = 0u;
            u32 windowWidth = 0u;
            FrameworkManager::GetWindowSize(windowWidth, windowHeight);

            // Vec4f color = Vec4f(0.0f, 0.0f, 0.0f, 1.f);
            // renderer::GraphicBackend::SetClearColor(color);
            // renderer::GraphicBackend::Clear(renderer::ClearFrameBuffer::Color_Depth);

            EngineContext context;
            context.deltaTime = dt;
            context.ecsManager = mpEcsManager;
            // Application Update
            mpApp->Update(context);

            mpEcsManager->Update(context);

            mpRenderer->TryResize(windowHeight,windowWidth);

            if (ImGui::Begin("background"))
            {
                //auto &backgroundEffects = mVkCore->test_get_backgroud_effects();
                //auto &currentBackgroundEffect = mVkCore->test_get_backgroud_effect_index();
                //auto &selected = backgroundEffects[currentBackgroundEffect];

                //ImGui::Text("Selected effect: ", selected.name);

                //ImGui::SliderInt("Effect Index", &currentBackgroundEffect, 0, backgroundEffects.size() - 1);

                //ImGui::InputFloat4("data1", (float *)&selected.data.data1);
                //ImGui::InputFloat4("data2", (float *)&selected.data.data2);
                //ImGui::InputFloat4("data3", (float *)&selected.data.data3);
                //ImGui::InputFloat4("data4", (float *)&selected.data.data4);

                ImGui::End();
            }

            renderer::ui::IMGUI_VULKAN_GLFW::Render();
            // Temp Vulkan Test
            
            mpRenderer->PushRenderObjects(rObjects);
            mpRenderer->Frame({windowWidth,windowHeight,context});

            FrameworkManager::PostUpdate();
        }
    }

    void Engine::Shutdown()
    {
        mpApp->Shutdown();
        //  Temp Vulkan Test
        //mVkCore->cleanup();
        mpRenderer->ShutDown();

        FrameworkManager::TerminateMain();
    }

    bool Engine::loadScene()
    {
        // SceneLoader::LoadScene(config::scene_folder_path + "scene_default.json");
        return false;
    }
}