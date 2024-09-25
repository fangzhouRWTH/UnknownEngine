#pragma once
#include "renderer/api_interface.hpp"

namespace unknown::renderer::vulkan
{
    class API_Vulkan:public GraphicAPI
    {
        public:
            virtual void initialize() override {}
            virtual void shutdown() override {}
    };
}