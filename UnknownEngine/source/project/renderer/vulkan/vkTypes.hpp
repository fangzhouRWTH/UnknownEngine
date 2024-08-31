#pragma once

#include "debug/log.hpp"
// #include "vulkan/"

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <vulkan/vulkan.h>
#include "renderer/vulkan/sdk/vk_enum_string_helper.h"
#include "renderer/vulkan/sdk/vk_mem_alloc.h"

#include "core/math.hpp"

namespace unknown
{
    struct AllocatedImage
    {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;
    };

    struct AllocatedBuffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    struct GPUGLTFMaterial
    {
        Vec4f colorFactors;
        Vec4f metal_rough_factors;
        Vec4f extra[14];
    };

    static_assert(sizeof(GPUGLTFMaterial) == 256);

    struct GPUSceneData
    {
        Mat4f view;
        Mat4f proj;
        Mat4f viewproj;
        Vec4f ambientColor;
        Vec4f sunlightDirection; // w for sun power
        Vec4f sunlightColor;
    };

    //> mat_types
    enum class MaterialPass : uint8_t
    {
        MainColor,
        Transparent,
        Other
    };
    struct MaterialPipeline
    {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };

    struct MaterialInstance
    {
        MaterialPipeline *pipeline;
        VkDescriptorSet materialSet;
        MaterialPass passType;
    };
    //< mat_types

    struct GPUMeshBuffers
    {
        AllocatedBuffer indexBuffer;
        AllocatedBuffer vertexBuffer;
        VkDeviceAddress vertexBufferAddress;
    };

    struct GPUDrawPushConstants
    {
        Mat4f worldMatrix;
        VkDeviceAddress vertexBuffer;
    };
}

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
