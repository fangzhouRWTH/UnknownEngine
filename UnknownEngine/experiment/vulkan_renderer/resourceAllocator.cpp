#include "vulkan_renderer/resourceAllocator.hpp"
#include "vulkan_renderer/sync.hpp"
#include "resourceAllocator.hpp"

namespace unknown::renderer::vulkan {
GPUResourceAllocator::~GPUResourceAllocator() {}

void GPUResourceAllocator::Init(const VulkanCoreData &vkData) {
  // mPhysicalDevice = vkData.physicalDevice;
  // mDevice = vkData.device;
  // mInstance = vkData.instance;

  // if (mUseTransferQueue = vkData.useTransferQueue, mUseTransferQueue) {
  //   mQueueFamilyIndex = vkData.transferQueueFamily;
  //   mQueue = vkData.transferQueue;
  // } else {
  //   mQueueFamilyIndex = vkData.graphicsQueueFamily;
  //   mQueue = vkData.graphicsQueue;
  // }
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

  if(buffer.state.isMapped)
    vmaMapMemory(mAllocator, buffer.allocation, &buffer.state.mapPtr);

  BufferHandle handle = mBuffers.addResource(buffer);

  assert(handle.isValid());

  return handle;
}

Buffer GPUResourceAllocator::GetBuffer(const BufferHandle &handle) const {
  return mBuffers.getResource(handle);
}

Buffer & GPUResourceAllocator::GetBuffer(const BufferHandle &handle) {
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

void GPUResourceAllocator::Destroy() {
  ReleaseAll();
  vmaDestroyAllocator(mAllocator);
}

void GPUResourceAllocator::unmapMappedBuffer(Buffer buffer) {
  if(buffer.state.isMapped)
    vmaUnmapMemory(mAllocator,buffer.allocation);
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
                                         u32 dstOffset, u32 size, ResourceStage stage) {
  auto & src = mBuffers.getResource(srcHandle);
  auto & dst = mBuffers.getResource(dstHandle);

  if (mVkData.useTransferQueue) {
    if (dst.state.queueFamilyIndex != mVkData.transferQueueFamily) {
      VkBufferMemoryBarrier srcBarrier =
          SynchronizationManager::CreateBufferMemoryBarrier(
              0, VK_ACCESS_TRANSFER_WRITE_BIT, dst.buffer,
              dst.state.queueFamilyIndex, mVkData.transferQueueFamily);

      //src.state.queueFamilyIndex = mVkData.transferQueueFamily;
      dst.state.queueFamilyIndex = mVkData.transferQueueFamily;
      dst.state.ownerQueueType = QueueType::Transfer;
      vkCmdPipelineBarrier(cmd.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &srcBarrier, 0,
                       nullptr);
    }
  }

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  vkCmdCopyBuffer(cmd.buffer, src.buffer, dst.buffer, 1, &copyRegion);

  VkBufferMemoryBarrier barrier =
      SynchronizationManager::CreateBufferMemoryBarrier(
          VK_ACCESS_TRANSFER_WRITE_BIT, 0, dst.buffer, mVkData.transferQueueFamily, mVkData.graphicsQueueFamily);

  vkCmdPipelineBarrier(cmd.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 1, &barrier, 0,
                       nullptr);
}
} // namespace unknown::renderer::vulkan