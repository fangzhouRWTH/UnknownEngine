#pragma once
#include <memory>
#include "core/clock.hpp"

#include "renderer/vulkan/vkCore.hpp"

namespace unknown
{
    class IApplication;

    namespace renderer
    {
        class IRenderer;
        class IMaterialManager;
    }

    namespace asset
    {
        class IAssetManager;
    }

    namespace ecs
    {
        class ECSManager;
    }

    class Engine
    {
        public:
            void Initialize();
            void Run();
            void Shutdown();

        private:
            std::shared_ptr<ecs::ECSManager> mpEcsManager;
            std::shared_ptr<IApplication> mpApp;
            std::shared_ptr<renderer::IRenderer> mpRenderer;
            std::shared_ptr<asset::IAssetManager> mpAssetManager;
            std::shared_ptr<renderer::IMaterialManager> mpMaterialManager;


            core::Clock mEngineClock;
            
        private:
            bool loadScene();
    };
}