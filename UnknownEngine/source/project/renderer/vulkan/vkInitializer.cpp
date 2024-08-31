#include "vkInitializer.hpp"

namespace unknown::renderer::vulkan::create
{
    VkCommandPoolCreateInfo command_pool_create_info(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;
        return info;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, u32 count)
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        return info;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.pInheritanceInfo = nullptr;
        info.flags = flags;
        return info;
    }

    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
    {
        VkSemaphoreSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        info.pNext = nullptr;
        info.semaphore = semaphore;
        info.stageMask = stageMask;
        info.deviceIndex = 0;
        info.value = 1;

        return info;
    }

    VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd)
    {
        VkCommandBufferSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        info.pNext = nullptr;
        info.commandBuffer = cmd;
        info.deviceMask = 0;

        return info;
    }

    VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signalSemaphoreInfo, VkSemaphoreSubmitInfo *waitSemaphoreInfo)
    {
        VkSubmitInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;

        info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
        info.pWaitSemaphoreInfos = waitSemaphoreInfo;

        info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
        info.pSignalSemaphoreInfos = signalSemaphoreInfo;

        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = cmd;

        return info;
    }

    VkPresentInfoKHR present_info()
    {
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.pNext = 0;
        info.swapchainCount = 0;
        info.pSwapchains = nullptr;
        info.pWaitSemaphores = nullptr;
        info.waitSemaphoreCount = 0;
        info.pImageIndices = nullptr;

        return info;
    }

    VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue *clear, VkImageLayout layout)
    {
        VkRenderingAttachmentInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        info.pNext = nullptr;

        info.imageView = view;
        info.imageLayout = layout;
        info.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (clear)
        {
            info.clearValue = *clear;
        }

        return info;
    }

    VkRenderingAttachmentInfo depth_attachment_info(VkImageView view, VkImageLayout layout)
    {
        VkRenderingAttachmentInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        info.pNext = nullptr;

        info.imageView = view;
        info.imageLayout = layout;
        info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.clearValue.depthStencil.depth = 1.f;

        return info;
    }

    VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment, VkRenderingAttachmentInfo *depthAttachment)
    {
        VkRenderingInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.pNext = nullptr;

        info.renderArea = VkRect2D{VkOffset2D(0, 0), renderExtent};
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = colorAttachment;
        info.pDepthAttachment = depthAttachment;
        info.pStencilAttachment = nullptr;
        return info;
    }

    VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask)
    {
        VkImageSubresourceRange range = {};
        range.aspectMask = aspectMask;
        range.baseMipLevel = 0;
        range.levelCount = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer = 0;
        range.layerCount = VK_REMAINING_ARRAY_LAYERS;
        return range;
    }

    VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags,u32 binding)
    {
        VkDescriptorSetLayoutBinding setBind = {};
        setBind.binding = binding;
        setBind.descriptorCount = 1;
        setBind.descriptorType = type;
        setBind.pImmutableSamplers = nullptr;
        setBind.stageFlags = stageFlags;

        return setBind;
    }

    VkDescriptorSetLayoutCreateInfo descriptorset_layout_create_info(VkDescriptorSetLayoutBinding* bindings, u32 bindingCount)
    {
        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;
        info.pBindings = bindings;
        info.bindingCount = bindingCount;
        info.flags = 0;
        return info;
    }

    VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, u32 binding)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstBinding = binding;
        write.dstSet = dstSet;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = imageInfo;

        return write;
    }

    VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, u32 binding)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstBinding = binding;
        write.dstSet = dstSet;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = bufferInfo;
        return write;
    }

    VkDescriptorBufferInfo buffer_info(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
    {
        VkDescriptorBufferInfo info = {};
        info.buffer= buffer;
        info.offset = offset;
        info.range = range;
        return info;
    }

    VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = format;
        info.extent = extent;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;

        return info;
    }

    VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;
        return info;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info()
    {
        VkPipelineLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = 0;
        info.setLayoutCount = 0;
        info.pSetLayouts = nullptr;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges = nullptr;
        return info;
    }

    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage,VkShaderModule shaderModule, const char* entry)
    {
        VkPipelineShaderStageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.stage = stage;
        info.module = shaderModule;
        info.pName = entry;
        return info;
    }


}