#pragma once

#include <vulkan/vulkan.h>

namespace unknown::renderer::vulkan::utils
{
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

    void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

    //void generate_mipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize);

    bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
}