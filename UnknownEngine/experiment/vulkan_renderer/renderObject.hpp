#pragma once

#include "vulkan_renderer/pipeline.hpp"
//#include "vulkan_renderer/descriptor.hpp"
#include "vulkan_renderer/sstring.hpp"
#include <vector>

namespace unknown::renderer::vulkan
{
    struct BindingDesc {
        u32 set = 0;
        u32 bindingPoint = 0;
        SString name = "";
    };

    struct RenderObject
    {
        PipelineDesc pipelineDesc;
        std::vector<BindingDesc> bindings;
    };
}