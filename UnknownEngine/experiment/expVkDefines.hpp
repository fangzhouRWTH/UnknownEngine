#pragma once
#include "renderer/vulkan/sdk/vk_enum_string_helper.h"

#define VK_CHECK(x)                                                        \
    do                                                                     \
    {                                                                      \
        VkResult err = x;                                                  \
        if (err)                                                           \
        {                                                                  \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
            abort();                                                       \
        }                                                                  \
    } while (0)