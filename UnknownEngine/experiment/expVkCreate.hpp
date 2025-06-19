#pragma once
#include <stdint.h>

#include "expVkDefines.hpp"
#include "renderer/vulkan/sdk/vk_mem_alloc.h"
#include "volk.h"

namespace unknown::exp {
VkPipelineLayoutCreateInfo _default_pipeline_layout_create_info() {
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

VkCommandPoolCreateInfo
_default_cmd_pool_create_info(uint32_t queueFamilyIndex,
                              VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.queueFamilyIndex = queueFamilyIndex;
  info.flags = flags;
  return info;
}

VkCommandBufferAllocateInfo
_default_cmd_buffer_allocate_info(VkCommandPool pool, u32 count) {
  VkCommandBufferAllocateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.commandPool = pool;
  info.commandBufferCount = count;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  return info;
}

VkFenceCreateInfo _default_fence_create_info(VkFenceCreateFlags flags) {
  VkFenceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  return info;
}

VkSemaphoreCreateInfo
_default_semaphore_create_info(VkSemaphoreCreateFlags flags = 0) {
  VkSemaphoreCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  return info;
}

VkImageCreateInfo _default_image_create_info(VkFormat format,
                                             VkImageUsageFlags usageFlags,
                                             VkExtent3D extent) {
  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.format = format;
  info.extent = extent;
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usageFlags;
  return info;
}

VkImageViewCreateInfo
_default_imageview_create_info(VkFormat format, VkImage image,
                               VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo info{};
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

void _destroy_image(VkDevice device, VmaAllocator allocator,
                    VmaAllocation allocation, VkImage image, VkImageView view) {
  vkDestroyImageView(device, view, nullptr);
  vmaDestroyImage(allocator, image, allocation);
}

VkCommandBufferBeginInfo
_default_cmd_buffer_begin_info(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;
  info.pInheritanceInfo = nullptr;
  info.flags = flags;
  return info;
}

VkImageSubresourceRange
_default_image_subresource_range(VkImageAspectFlags aspectMask) {
  VkImageSubresourceRange range{};
  range.aspectMask = aspectMask;
  range.baseMipLevel = 0u;
  range.levelCount = VK_REMAINING_MIP_LEVELS;
  range.baseArrayLayer = 0u;
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;
  return range;
}

VkCommandBufferSubmitInfo _default_cmd_buffer_submit_info(VkCommandBuffer cmd) {
  VkCommandBufferSubmitInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  info.pNext = nullptr;
  info.commandBuffer = cmd;
  info.deviceMask = 0;

  return info;
}

VkSemaphoreSubmitInfo
_default_semaphore_submit_info(VkPipelineStageFlags2 stageMask,
                               VkSemaphore semaphore) {
  VkSemaphoreSubmitInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  info.pNext = nullptr;
  info.semaphore = semaphore;
  info.stageMask = stageMask;
  info.deviceIndex = 0;
  info.value = 1;

  return info;
}

VkSubmitInfo2 _default_submit_info(VkCommandBufferSubmitInfo *cmd,
                                   VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                                   VkSemaphoreSubmitInfo *waitSemaphoreInfo) {
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

VkPresentInfoKHR _default_present_info() {
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

VkPipelineShaderStageCreateInfo
_default_pipeline_shader_stage_create_info(VkShaderStageFlagBits stage,
                                           VkShaderModule shaderModule,
                                           const char *entry = "main") {
  VkPipelineShaderStageCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.stage = stage;
  info.module = shaderModule;
  info.pName = entry;
  return info;
}

VkRenderingAttachmentInfo _default_attachment_info(VkImageView view,
                                                   VkClearValue *clear,
                                                   VkImageLayout layout) {
  VkRenderingAttachmentInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info.pNext = nullptr;

  info.imageView = view;
  info.imageLayout = layout;
  info.loadOp =
      clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  if (clear) {
    info.clearValue = *clear;
  }

  return info;
}

VkRenderingAttachmentInfo _default_depth_attachment_info(VkImageView view,
                                                         VkImageLayout layout) {
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

VkRenderingInfo
_default_rendering_info(VkExtent2D renderExtent,
                        VkRenderingAttachmentInfo *colorAttachment,
                        VkRenderingAttachmentInfo *depthAttachment) {
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

namespace utils {
bool _load_shader_module(const char *filePath, VkDevice device,
                         VkShaderModule &out) {
  // open the file. With cursor at the end
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return false;
  }

  // find what the size of the file is by looking up the location of the cursor
  // because the cursor is at the end, it gives the size directly in bytes
  size_t fileSize = (size_t)file.tellg();

  // spirv expects the buffer to be on uint32, so make sure to reserve a int
  // vector big enough for the entire file
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  // put file cursor at beginning
  file.seekg(0);

  // load the entire file into the buffer
  file.read((char *)buffer.data(), fileSize);

  // now that the file is loaded into the buffer, we can close it
  file.close();

  // create a new shader module, using the buffer we loaded
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = nullptr;

  // codeSize has to be in bytes, so multply the ints in the buffer by size of
  // int to know the real size of the buffer
  createInfo.codeSize = buffer.size() * sizeof(uint32_t);
  createInfo.pCode = buffer.data();

  // check that the creation goes well.
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    return false;
  }
  out = shaderModule;
  return true;
}

void _transition_image(VkCommandBuffer cmd, VkImage image,
                       VkImageLayout currentLayout, VkImageLayout newLayout) {
  VkImageMemoryBarrier2 imageBarrier;
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageBarrier.pNext = nullptr;
  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  imageBarrier.oldLayout = currentLayout;
  imageBarrier.newLayout = newLayout;

  VkImageAspectFlags aspectMask =
      (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;
  imageBarrier.subresourceRange = _default_image_subresource_range(aspectMask);
  imageBarrier.image = image;

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.pNext = nullptr;
  dependencyInfo.imageMemoryBarrierCount = 1u;
  dependencyInfo.pImageMemoryBarriers = &imageBarrier;
  vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

void _copy_image_to_image(VkCommandBuffer cmd, VkImage source,
                          VkImage destination, VkExtent2D srcSize,
                          VkExtent2D dstSize) {
  VkImageBlit2 blitRegion{};

  blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
  blitRegion.pNext = nullptr;
  blitRegion.srcOffsets[1].x = srcSize.width;
  blitRegion.srcOffsets[1].y = srcSize.height;
  blitRegion.srcOffsets[1].z = 1;

  blitRegion.dstOffsets[1].x = dstSize.width;
  blitRegion.dstOffsets[1].y = dstSize.height;
  blitRegion.dstOffsets[1].z = 1;

  blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.srcSubresource.baseArrayLayer = 0;
  blitRegion.srcSubresource.layerCount = 1;
  blitRegion.srcSubresource.mipLevel = 0;

  blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.dstSubresource.baseArrayLayer = 0;
  blitRegion.dstSubresource.layerCount = 1;
  blitRegion.dstSubresource.mipLevel = 0;

  VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                            .pNext = nullptr};
  blitInfo.dstImage = destination;
  blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  blitInfo.srcImage = source;
  blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  blitInfo.filter = VK_FILTER_LINEAR;
  blitInfo.regionCount = 1;
  blitInfo.pRegions = &blitRegion;

  vkCmdBlitImage2(cmd, &blitInfo);
}
} // namespace utils
} // namespace unknown::exp