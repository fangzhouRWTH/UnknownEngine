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
    }

    namespace asset
    {
        class ResourceManager;
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
            core::Clock mEngineClock;
            
        private:
            bool loadScene();
            std::shared_ptr<IApplication> mpApp;
            std::shared_ptr<renderer::IRenderer> mpRenderer;
            std::shared_ptr<asset::ResourceManager> mpResourceManager;
    };
}