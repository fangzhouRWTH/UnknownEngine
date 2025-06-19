#pragma once

#include "common.hpp"

namespace unknown::renderer::vulkan {
struct ImageTransitionDesc {
  VkCommandBuffer cmd;
  VkImage image;
  VkImageLayout oldLayout;
  VkImageLayout newLayout;
};

inline void TransitionImageResource(const ImageTransitionDesc &desc) {
  VkImageMemoryBarrier2 imageBarrier {};
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageBarrier.pNext = nullptr;
  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  imageBarrier.oldLayout = desc.oldLayout;
  imageBarrier.newLayout = desc.newLayout;

  VkImageAspectFlags aspectMask =
      (desc.newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageSubresourceRange range{};
  range.aspectMask = aspectMask;
  range.baseMipLevel = 0u;
  range.levelCount = VK_REMAINING_MIP_LEVELS;
  range.baseArrayLayer = 0u;
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;

  imageBarrier.subresourceRange = range;
  imageBarrier.image = desc.image;

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.pNext = nullptr;
  dependencyInfo.imageMemoryBarrierCount = 1u;
  dependencyInfo.pImageMemoryBarriers = &imageBarrier;
  vkCmdPipelineBarrier2(desc.cmd, &dependencyInfo);
}

struct CopyImageToImageDesc {
  VkCommandBuffer cmd;
  VkImage source;
  VkImage destination;
  VkExtent2D srcSize;
  VkExtent2D dstSize;
};

inline void CopyImageToImage(const CopyImageToImageDesc &desc) {
  VkImageBlit2 blitRegion{};

  blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
  blitRegion.pNext = nullptr;
  blitRegion.srcOffsets[1].x = desc.srcSize.width;
  blitRegion.srcOffsets[1].y = desc.srcSize.height;
  blitRegion.srcOffsets[1].z = 1;

  blitRegion.dstOffsets[1].x = desc.dstSize.width;
  blitRegion.dstOffsets[1].y = desc.dstSize.height;
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
  blitInfo.dstImage = desc.destination;
  blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  blitInfo.srcImage = desc.source;
  blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  blitInfo.filter = VK_FILTER_LINEAR;
  blitInfo.regionCount = 1;
  blitInfo.pRegions = &blitRegion;

  vkCmdBlitImage2(desc.cmd, &blitInfo);
}
} // namespace unknown::renderer::vulkan