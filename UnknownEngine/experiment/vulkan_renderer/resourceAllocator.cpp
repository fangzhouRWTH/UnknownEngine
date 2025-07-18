#include "vulkan_renderer/resourceAllocator.hpp"
#include "resourceAllocator.hpp"
#include "utils/imageLoader.hpp"
#include "vulkan_renderer/defines.hpp"
#include "vulkan_renderer/sync.hpp"

namespace unknown::renderer::vulkan {
GPUResourceAllocator::~GPUResourceAllocator() {}

ResourceUsageInfo getResourceUsageInfo(const ResourceUsage &usage) {
  ResourceUsageInfo info;

  switch (usage) {
  case ResourceUsage::TransferSrc:
    break;
  case ResourceUsage::TransferDst:
    break;
  default:
    assert(false);
  }
  return info;
}

ResourceOwnershipInfo GPUResourceAllocator::getResourceTransferInfo(
    const ResourceOwnershipTransition &transition) {
  ResourceOwnershipInfo info;
  switch (transition) {
  case ResourceOwnershipTransition::Acquire_Graphic_Transfer:
    info.src.access = VK_ACCESS_NONE;
    info.src.stage = VK_PIPELINE_STAGE_NONE;
    info.src.queue = QueueType::Graphic;
    info.src.index = mVkData.graphicsQueueFamily;

    info.dst.access = VK_ACCESS_TRANSFER_WRITE_BIT;
    info.dst.stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    info.dst.queue = QueueType::Transfer;
    info.dst.index = mVkData.transferQueueFamily;
    break;
  case ResourceOwnershipTransition::Release_Transfer_Graphic:
    info.src.access = VK_ACCESS_TRANSFER_WRITE_BIT;
    info.src.stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    info.src.queue = QueueType::Transfer;
    info.src.index = mVkData.transferQueueFamily;

    info.dst.access = VK_ACCESS_NONE;
    info.dst.stage = VK_PIPELINE_STAGE_NONE;
    info.dst.queue = QueueType::Graphic;
    info.dst.index = mVkData.graphicsQueueFamily;
    break;
  default:
    assert(false);
  }

  return info;
}

void GPUResourceAllocator::getQueueInfo(QueueType type, VkQueue &queue,
                                        u32 &index) {
  switch (type) {
  case QueueType::Graphic:
    queue = mVkData.graphicsQueue;
    index = mVkData.graphicsQueueFamily;
    break;
  case QueueType::Transfer:
    queue = mVkData.transferQueue;
    index = mVkData.transferQueueFamily;
    break;
  case QueueType::Compute:
    queue = mVkData.computeQueue;
    index = mVkData.computeQueueFamily;
    break;
  default:
    break;
  }
}

void GPUResourceAllocator::Init(const VulkanCoreData &vkData) {
  mVkData = vkData;

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = vkData.physicalDevice;
  allocatorInfo.device = vkData.device;
  allocatorInfo.instance = vkData.instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  VmaVulkanFunctions vmaFunctions = {};
  vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
  allocatorInfo.pVulkanFunctions = &vmaFunctions;

  vmaCreateAllocator(&allocatorInfo, &mAllocator);
}

BufferHandle GPUResourceAllocator::CreateBuffer(const BufferDesc &desc) {
  VkBufferCreateInfo bInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bInfo.pNext = nullptr;
  bInfo.size = desc.size;
  bInfo.usage = desc.bufferUsage.get();
  bInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bInfo.queueFamilyIndexCount = 1;
  bInfo.pQueueFamilyIndices = &mVkData.graphicsQueueFamily;

  if (mVkData.useTransferQueue && desc.memoryProperty.isOnlyStaging()) {
    bInfo.queueFamilyIndexCount = 1;
    bInfo.pQueueFamilyIndices = &mVkData.transferQueueFamily;
  }

  VmaAllocationCreateInfo vmaAllocation = {};
  vmaAllocation.usage = desc.memoryUsage.get();
  vmaAllocation.flags = desc.memoryProperty.get();

  Buffer buffer;
  if (vmaAllocation.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
    buffer.state.isMapped = true;
  else
    buffer.state.isMapped = false;

  buffer.state.onlyStaging = desc.memoryProperty.isOnlyStaging();

  if (mVkData.useTransferQueue && desc.memoryProperty.isOnlyStaging()) {
    buffer.state.queueFamilyIndex = mVkData.transferQueueFamily;
  } else {
    buffer.state.queueFamilyIndex = mVkData.graphicsQueueFamily;
  }

  VK_CHECK(vmaCreateBuffer(mAllocator, &bInfo, &vmaAllocation, &buffer.buffer,
                           &buffer.allocation, &buffer.info));

  if (buffer.state.isMapped)
    vmaMapMemory(mAllocator, buffer.allocation, &buffer.state.mapPtr);

  BufferHandle handle = mBuffers.addResource(buffer);

  assert(handle.isValid());

  return handle;
}

Buffer GPUResourceAllocator::GetBuffer(const BufferHandle &handle) const {
  return mBuffers.getResource(handle);
}

Buffer &GPUResourceAllocator::GetBuffer(const BufferHandle &handle) {
  return mBuffers.getResource(handle);
}

void GPUResourceAllocator::Release(const BufferHandle &handle) {
  auto res = mBuffers.getResource(handle);
  mBuffers.releaseResource(handle);
  unmapMappedBuffer(res);
  vmaDestroyBuffer(mAllocator, res.buffer, res.allocation);
}

ImageHandle GPUResourceAllocator::CreateImage(const ImageDesc &desc) {
  VkImageCreateInfo info{};
  // default
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;

  // setting
  info.format = desc.imageFormat.getFormat();
  info.extent = desc.imageFormat.getExtent();
  info.usage = desc.imageUsage.get();

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = desc.memoryUsage.get();
  imageAllocInfo.requiredFlags = desc.memoryProperty.get();

  Image image;
  image.format = info.format;
  image.extent = info.extent;
  // auto r = vmaCreateImage(mAllocator, &info, &imageAllocInfo, &image.image,
  //                         &image.allocation, nullptr);
  //                         VK_CHECK(r);
  VK_CHECK(vmaCreateImage(mAllocator, &info, &imageAllocInfo, &image.image,
                          &image.allocation, nullptr));

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = nullptr;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.image = image.image;
  viewInfo.format = image.format;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;
  viewInfo.subresourceRange.aspectMask = desc.imageFormat.getAspect();

  VK_CHECK(vkCreateImageView(mVkData.device, &viewInfo, nullptr, &image.view));

  ImageHandle handle = mImages.addResource(image);

  assert(handle.isValid());

  return handle;
}

Image GPUResourceAllocator::GetImage(const ImageHandle &handle) {
  return mImages.getResource(handle);
}

void GPUResourceAllocator::TransitionImageResource(const VkCommandBuffer &cmd,
                                                   const ImageHandle &handle,
                                                   VkImageLayout dst) {
  auto & img = mImages.getResource(handle);

  VkImageMemoryBarrier2 imageBarrier{};
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageBarrier.pNext = nullptr;
  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  imageBarrier.oldLayout = img.layout;
  imageBarrier.newLayout = dst;

  VkImageAspectFlags aspectMask =
      (dst == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageSubresourceRange range{};
  range.aspectMask = aspectMask;
  range.baseMipLevel = 0u;
  range.levelCount = VK_REMAINING_MIP_LEVELS;
  range.baseArrayLayer = 0u;
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;

  imageBarrier.subresourceRange = range;
  imageBarrier.image = img.image;

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.pNext = nullptr;
  dependencyInfo.imageMemoryBarrierCount = 1u;
  dependencyInfo.pImageMemoryBarriers = &imageBarrier;
  vkCmdPipelineBarrier2(cmd, &dependencyInfo);

  img.layout = dst;
}

struct ImageTransitionInfo {
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;
  VkImageMemoryBarrier barrier;
};

ImageTransitionInfo transition_image_layout(bool transferQueue,
                                            VkQueue graphicsQueue,
                                            VkImage image, VkFormat format,
                                            VkImageLayout oldLayout,
                                            VkImageLayout newLayout) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;

  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (transferQueue) {
      destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      barrier.dstAccessMask = 0;
    } else {
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }

  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  return {sourceStage, destinationStage, barrier};
}

ImageHandle GPUResourceAllocator::LoadCreateImage(
    std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> &cmdManager,
    bool useTransferQueue, ImageDesc &desc, const std::string &filePath,
    u32 &width, u32 &height, u32 &channels) {
  // TODO

  bool transferQueue = (useTransferQueue && mVkData.useTransferQueue &&
                        mVkData.hasTransferQueue);

  auto ptr = ImageLoader::Load(filePath, width, height, channels);
  assert(ptr != nullptr);
  u32 size = width * height * channels;
  BufferDesc bDesc;
  bDesc.bufferUsage.transfer_src();
  bDesc.memoryUsage.prefer_host();
  bDesc.memoryProperty.staging();
  bDesc.size = size;
  auto stagingHandle = CreateBuffer(bDesc);
  auto stagingBuffer = GetBuffer(stagingHandle);

  MapCopyBuffer((void *)ptr, stagingHandle, 0u, size);

  ImageLoader::Free(ptr);

  // TODO
  ImageDesc &d = desc;
  d.imageFormat.size(width, height);
  d.imageFormat.rgba_8_srgb().color_aspect();
  d.imageUsage.sample().transfer_dst();
  d.memoryUsage.prefer_device();
  auto imageHandle = CreateImage(d);
  auto image = GetImage(imageHandle);

  VkQueue queue;
  if (transferQueue)
    queue = mVkData.transferQueue;
  else
    queue = mVkData.graphicsQueue;

  CommandBuffer cmd;
  if (transferQueue)
    cmd = cmdManager->BeginCommandBuffer(QueueType::Transfer);
  else
    cmd = cmdManager->BeginCommandBuffer(QueueType::Graphic);

  auto preTrans = transition_image_layout(
      transferQueue, queue, image.image, VK_FORMAT_R8G8B8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  vkCmdPipelineBarrier(cmd.buffer, preTrans.sourceStage,
                       preTrans.destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                       &preTrans.barrier);

  {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0; // tightly packed
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmd.buffer, stagingBuffer.buffer, image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  }

  auto postTrans = transition_image_layout(
      transferQueue, queue, image.image, VK_FORMAT_R8G8B8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkCmdPipelineBarrier(cmd.buffer, postTrans.sourceStage,
                       postTrans.destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                       &postTrans.barrier);

  if (transferQueue) {
    ReleaseOwnership(cmd, imageHandle, QueueType::Transfer, QueueType::Graphic);
    AcquireOwnership(cmd, imageHandle, QueueType::Transfer, QueueType::Graphic);
  }

  cmdManager->EndCommandBuffer(cmd);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd.buffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  Release(stagingHandle);
  // ReleaseBuffer(stagingHandle);

  return imageHandle;
}

void GPUResourceAllocator::Release(const ImageHandle &handle) {
  auto res = mImages.getResource(handle);
  mImages.releaseResource(handle);
  vmaDestroyImage(mAllocator, res.image, res.allocation);
  vkDestroyImageView(mVkData.device, res.view, nullptr);
}

void GPUResourceAllocator::ReleaseAllBuffers() {
  std::vector<Buffer> buffers;
  mBuffers.getResources(buffers);
  for (auto res : buffers) {
    unmapMappedBuffer(res);
    vmaDestroyBuffer(mAllocator, res.buffer, res.allocation);
  }
  mBuffers.resetContainer();
}

void GPUResourceAllocator::ReleaseAllImages() {
  std::vector<Image> images;
  mImages.getResources(images);
  for (auto res : images) {
    vmaDestroyImage(mAllocator, res.image, res.allocation);
    vkDestroyImageView(mVkData.device, res.view, nullptr);
  }
  mImages.resetContainer();
}

void GPUResourceAllocator::ReleaseOwnership(const CommandBuffer &cmd,
                                            BufferHandle handle, QueueType from,
                                            QueueType to) {
  // todo
  VkQueue fromQ, toQ;
  u32 fromI, toI;
  getQueueInfo(from, fromQ, fromI);
  getQueueInfo(to, toQ, toI);

  if (fromI == toI)
    return;

  VkAccessFlags srcAccess;
  VkAccessFlags dstAccess;
  VkPipelineStageFlags srcStage;
  VkPipelineStageFlags dstStage;

  if (from == QueueType::Graphic && to == QueueType::Transfer) {
    srcAccess = VK_ACCESS_SHADER_READ_BIT;
    dstAccess = 0;
    srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else if (from == QueueType::Transfer && to == QueueType::Graphic) {
    srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
    dstAccess = 0;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else {
    // todo
    return;
  }

  auto buffer = GetBuffer(handle);
  VkBufferMemoryBarrier srcBarrier =
      SynchronizationManager<FRAME_OVERLAP>::CreateBufferMemoryBarrier(
          srcAccess, dstAccess, buffer.buffer, fromI, toI);

  vkCmdPipelineBarrier(cmd.buffer, srcStage, dstStage, 0, 0, nullptr, 1,
                       &srcBarrier, 0, nullptr);
}

void GPUResourceAllocator::AcquireOwnership(const CommandBuffer &cmd,
                                            BufferHandle handle, QueueType from,
                                            QueueType to) {
  // todo
  VkQueue fromQ, toQ;
  u32 fromI, toI;
  getQueueInfo(from, fromQ, fromI);
  getQueueInfo(to, toQ, toI);

  if (fromI == toI)
    return;

  VkAccessFlags srcAccess;
  VkAccessFlags dstAccess;
  VkPipelineStageFlags srcStage;
  VkPipelineStageFlags dstStage;

  if (from == QueueType::Graphic && to == QueueType::Transfer) {
    srcAccess = 0;
    dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (from == QueueType::Transfer && to == QueueType::Graphic) {
    srcAccess = 0;
    dstAccess = VK_ACCESS_SHADER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  } else {
    // todo
    return;
  }

  auto buffer = GetBuffer(handle);
  VkBufferMemoryBarrier srcBarrier =
      SynchronizationManager<FRAME_OVERLAP>::CreateBufferMemoryBarrier(
          srcAccess, dstAccess, buffer.buffer, fromI, toI);

  vkCmdPipelineBarrier(cmd.buffer, srcStage, dstStage, 0, 0, nullptr, 1,
                       &srcBarrier, 0, nullptr);
}

void GPUResourceAllocator::ReleaseOwnership(const CommandBuffer &cmd,
                                            ImageHandle handle, QueueType from,
                                            QueueType to) {
  // VkQueue fromQ, toQ;
  // u32 fromI, toI;
  // getQueueInfo(from, fromQ, fromI);
  // getQueueInfo(to, toQ, toI);

  // if (fromI == toI)
  //   return;

  // VkAccessFlags srcAccess;
  // VkAccessFlags dstAccess;
  // VkPipelineStageFlags srcStage;
  // VkPipelineStageFlags dstStage;

  // if (from == QueueType::Graphic && to == QueueType::Transfer) {
  //   srcAccess = VK_ACCESS_SHADER_READ_BIT;
  //   dstAccess = 0;
  //   srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  //   dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  // } else if (from == QueueType::Transfer && to == QueueType::Graphic) {
  //   srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
  //   dstAccess = 0;
  //   srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  //   dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  // } else {
  //   // todo
  //   return;
  // }

  // //TransitionImageResource(cmd.buffer,handle,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // auto & img = mImages.getResource(handle);

  // VkImageMemoryBarrier2 imageBarrier{};
  // imageBarrier.srcQueueFamilyIndex = fromI;
  // imageBarrier.dstQueueFamilyIndex = toI;
  // imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  // imageBarrier.pNext = nullptr;
  // imageBarrier.srcStageMask = srcStage;
  // imageBarrier.srcAccessMask = srcAccess;
  // imageBarrier.dstStageMask = dstStage;
  // imageBarrier.dstAccessMask = dstAccess;

  // imageBarrier.oldLayout = img.layout;
  // imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

  // VkImageAspectFlags aspectMask =
  //     (dst == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
  //         ? VK_IMAGE_ASPECT_DEPTH_BIT
  //         : VK_IMAGE_ASPECT_COLOR_BIT;

  // VkImageSubresourceRange range{};
  // range.aspectMask = aspectMask;
  // range.baseMipLevel = 0u;
  // range.levelCount = VK_REMAINING_MIP_LEVELS;
  // range.baseArrayLayer = 0u;
  // range.layerCount = VK_REMAINING_ARRAY_LAYERS;

  // imageBarrier.subresourceRange = range;
  // imageBarrier.image = img.image;

  // VkDependencyInfo dependencyInfo{};
  // dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  // dependencyInfo.pNext = nullptr;
  // dependencyInfo.imageMemoryBarrierCount = 1u;
  // dependencyInfo.pImageMemoryBarriers = &imageBarrier;
  // vkCmdPipelineBarrier2(cmd, &dependencyInfo);

  // img.layout = dst;
}

void GPUResourceAllocator::AcquireOwnership(const CommandBuffer &cmd,
                                            ImageHandle handle, QueueType from,
                                            QueueType to) {
  // // todo
  // VkQueue fromQ, toQ;
  // u32 fromI, toI;
  // getQueueInfo(from, fromQ, fromI);
  // getQueueInfo(to, toQ, toI);

  // if (fromI == toI)
  //   return;

  // VkAccessFlags srcAccess;
  // VkAccessFlags dstAccess;
  // VkPipelineStageFlags srcStage;
  // VkPipelineStageFlags dstStage;

  // if (from == QueueType::Graphic && to == QueueType::Transfer) {
  //   srcAccess = 0;
  //   dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
  //   srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  //   dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  // } else if (from == QueueType::Transfer && to == QueueType::Graphic) {
  //   srcAccess = 0;
  //   dstAccess = VK_ACCESS_SHADER_READ_BIT;
  //   srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  //   dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  // } else {
  //   // todo
  //   return;
  // }

  // auto img = GetImage(handle);
  // VkBufferMemoryBarrier srcBarrier =
  //     SynchronizationManager<FRAME_OVERLAP>::CreateImageMemoryBarrier(
  //         srcAccess, dstAccess, img.image, fromI, toI);

  // vkCmdPipelineBarrier(cmd.buffer, srcStage, dstStage, 0, 0, nullptr, 1,
  //                      &srcBarrier, 0, nullptr);
}

void GPUResourceAllocator::Destroy() {
  ReleaseAll();
  vmaDestroyAllocator(mAllocator);
}

void GPUResourceAllocator::unmapMappedBuffer(Buffer buffer) {
  if (buffer.state.isMapped)
    vmaUnmapMemory(mAllocator, buffer.allocation);
}

void GPUResourceAllocator::ReleaseAll() {
  ReleaseAllBuffers();
  ReleaseAllImages();
}

void GPUResourceAllocator::MapCopyBuffer(void *src, BufferHandle dstHandle,
                                         u32 dstOffset, u32 size) {
  auto res = mBuffers.getResource(dstHandle);
  assert(res.state.isMapped);
  assert(res.state.mapPtr != nullptr);
  if (!res.state.isMapped || res.state.mapPtr == nullptr)
    return;

  void *dst = reinterpret_cast<void *>(
      reinterpret_cast<byte *>(res.state.mapPtr) + dstOffset);
  memcpy(dst, src, size);
}

VkAccessFlags getAccessFlag(ResourceStage stage) {
  switch (stage) {
  case ResourceStage::UniformTaskMesh:
  case ResourceStage::UniformCompute:
  case ResourceStage::UniformVertexFragment:
    return VK_ACCESS_UNIFORM_READ_BIT;
  case ResourceStage::StorageVertexFragment:
  case ResourceStage::StorageTaskMesh:
  case ResourceStage::StorageCompute:
    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
  case ResourceStage::TransferWrite:
    return VK_ACCESS_TRANSFER_WRITE_BIT;
  case ResourceStage::TransferRead:
    return VK_ACCESS_TRANSFER_READ_BIT;
  default:
    assert(false);
    return VK_ACCESS_NONE;
  }
}

VkPipelineStageFlags getStageFlags(ResourceStage stage) {
  switch (stage) {
  case ResourceStage::UniformTaskMesh:
  case ResourceStage::StorageTaskMesh:
    return VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT |
           VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;

  case ResourceStage::UniformCompute:
  case ResourceStage::StorageCompute:
    return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

  case ResourceStage::StorageVertexFragment:
  case ResourceStage::UniformVertexFragment:
    return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  case ResourceStage::TransferRead:
  case ResourceStage::TransferWrite:
    return VK_PIPELINE_STAGE_TRANSFER_BIT;

  default:
    assert(false);
    return VK_PIPELINE_STAGE_NONE;
  }
}

void GPUResourceAllocator::GPUCopyBuffer(const CommandBuffer &cmd,
                                         BufferHandle srcHandle,
                                         BufferHandle dstHandle, u32 srcOffset,
                                         u32 dstOffset, u32 size,
                                         ResourceStage stage) {
  auto &src = mBuffers.getResource(srcHandle);
  auto &dst = mBuffers.getResource(dstHandle);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  vkCmdCopyBuffer(cmd.buffer, src.buffer, dst.buffer, 1, &copyRegion);

  VkBufferMemoryBarrier barrier =
      SynchronizationManager<FRAME_OVERLAP>::CreateBufferMemoryBarrier(
          VK_ACCESS_TRANSFER_WRITE_BIT, 0, dst.buffer,
          mVkData.transferQueueFamily, mVkData.graphicsQueueFamily);

  vkCmdPipelineBarrier(cmd.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 1,
                       &barrier, 0, nullptr);
}
} // namespace unknown::renderer::vulkan