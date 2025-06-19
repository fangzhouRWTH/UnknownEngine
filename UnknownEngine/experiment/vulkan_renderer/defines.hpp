#pragma once

#define FRAME_OVERLAP (2u)

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