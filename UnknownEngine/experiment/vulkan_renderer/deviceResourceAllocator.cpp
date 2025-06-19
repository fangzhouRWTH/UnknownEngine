#include "vulkan_renderer/deviceResourceAllocator.hpp"
#include "vulkan_renderer/sync.hpp"

namespace unknown::renderer::vulkan {
//DeviceResourceAllocator::~DeviceResourceAllocator() {}

// void DeviceResourceAllocator::Init(const VulkanCoreData &vkData) {
//   mPhysicalDevice = vkData.physicalDevice;
//   mDevice = vkData.device;
//   mInstance = vkData.instance;

//   VmaAllocatorCreateInfo allocatorInfo = {};
//   allocatorInfo.physicalDevice = vkData.physicalDevice;
//   allocatorInfo.device = vkData.device;
//   allocatorInfo.instance = vkData.instance;
//   allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
//   VmaVulkanFunctions vmaFunctions = {};
//   vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
//   vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
//   allocatorInfo.pVulkanFunctions = &vmaFunctions;

//   vmaCreateAllocator(&allocatorInfo, &mAllocator);
// }

// ResourceHandle DeviceResourceAllocator::CreateBuffer(const BufferDesc &desc) {
//   VkBufferCreateInfo bInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//   bInfo.pNext = nullptr;
//   bInfo.size = desc.size;
//   bInfo.usage = desc.bufferUsage.get();

//   VmaAllocationCreateInfo vmaAllocation = {};
//   vmaAllocation.usage = desc.memoryUsage.getUsage();
//   //?
//   vmaAllocation.flags = desc.memoryUsage.getProperty();

//   Buffer buffer;

//   VK_CHECK(vmaCreateBuffer(mAllocator, &bInfo, &vmaAllocation, &buffer.buffer,
//                            &buffer.allocation, &buffer.info));

//   Resource res;
//   res.data.buffer = buffer;
//   res.type = ResourceType::Buffer;

//   ResourceHandle handle = mResources.addResource(res);

//   assert(handle.isValid());

//   return handle;
// }

// Buffer DeviceResourceAllocator::GetBuffer(const ResourceHandle &handle) {
//   auto res = mResources.getResource(handle);
//   assert(res.type == ResourceType::Buffer);
//   return res.data.buffer;
// }

// ResourceHandle DeviceResourceAllocator::CreateImage(const ImageDesc &desc) {
//   VkImageCreateInfo info{};
//   // default
//   info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//   info.pNext = nullptr;
//   info.imageType = VK_IMAGE_TYPE_2D;
//   info.mipLevels = 1;
//   info.arrayLayers = 1;
//   info.samples = VK_SAMPLE_COUNT_1_BIT;
//   info.tiling = VK_IMAGE_TILING_OPTIMAL;

//   // setting
//   info.format = desc.imageFormat.getFormat();
//   info.extent = desc.imageFormat.getExtent();
//   info.usage = desc.imageUsage.get();

//   VmaAllocationCreateInfo imageAllocInfo{};
//   imageAllocInfo.usage = desc.memoryUsage.getUsage();
//   imageAllocInfo.requiredFlags = desc.memoryUsage.getProperty();

//   Image image;
//   image.format = info.format;
//   image.extent = info.extent;
//   // auto r = vmaCreateImage(mAllocator, &info, &imageAllocInfo, &image.image,
//   //                         &image.allocation, nullptr);
//   //                         VK_CHECK(r);
//   VK_CHECK(vmaCreateImage(mAllocator, &info, &imageAllocInfo, &image.image,
//                           &image.allocation, nullptr));

//   VkImageViewCreateInfo viewInfo = {};
//   viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//   viewInfo.pNext = nullptr;
//   viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//   viewInfo.image = image.image;
//   viewInfo.format = image.format;
//   viewInfo.subresourceRange.baseMipLevel = 0;
//   viewInfo.subresourceRange.levelCount = 1;
//   viewInfo.subresourceRange.baseArrayLayer = 0;
//   viewInfo.subresourceRange.layerCount = 1;
//   viewInfo.subresourceRange.aspectMask = desc.imageFormat.getAspect();

//   VK_CHECK(vkCreateImageView(mDevice, &viewInfo, nullptr, &image.view));

//   Resource res;
//   res.data.image = image;
//   res.type = ResourceType::Image;

//   ResourceHandle handle = mResources.addResource(res);

//   assert(handle.isValid());

//   return handle;
// }

// Image DeviceResourceAllocator::GetImage(const ResourceHandle &handle) {
//   auto res = mResources.getResource(handle);
//   assert(res.type == ResourceType::Image);
//   return res.data.image;
// }

// void DeviceResourceAllocator::releaseBuffer(const ResourceHandle &handle) {
//   Buffer buffer = GetBuffer(handle);
//   vmaDestroyBuffer(mAllocator, buffer.buffer, buffer.allocation);
//   mResources.releaseResource(handle);
// }

// void DeviceResourceAllocator::releasImage(const ResourceHandle &handle) {
//   Image image = GetImage(handle);
//   vmaDestroyImage(mAllocator, image.image, image.allocation);
//   vkDestroyImageView(mDevice, image.view, nullptr);
//   mResources.releaseResource(handle);
// }

// void DeviceResourceAllocator::ReleaseResource(const ResourceHandle &handle) {
//   auto res = mResources.getResource(handle);
//   switch (res.type) {
//   case ResourceType::Buffer: {
//     vmaDestroyBuffer(mAllocator, res.data.buffer.buffer,
//                      res.data.buffer.allocation);
//     break;
//   }
//   case ResourceType::Image: {
//     vmaDestroyImage(mAllocator, res.data.image.image,
//                     res.data.image.allocation);
//     break;
//   }
//   default:
//     assert(false);
//   }
//   mResources.releaseResource(handle);
// }

// void DeviceResourceAllocator::MapBuffer(u32 size, void *src, ResourceHandle dst,
//                                         u32 dstOffset) {
//   auto res = mResources.getResource(dst);
//   assert(res.type == ResourceType::Buffer);
//   MapBuffer(size, src, res.data.buffer, dstOffset);
// }

// void DeviceResourceAllocator::MapBuffer(u32 size, void *src, Buffer dst,
//                                         u32 dstOffset) {
//   void *dstPtr;
//   vmaMapMemory(mAllocator, dst.allocation, &dstPtr);
//   void *dOffset =
//       reinterpret_cast<void *>(reinterpret_cast<byte *>(dstPtr) + dstOffset);
//   void *srcPtr = src;
//   memcpy(dOffset, srcPtr, size);
//   vmaUnmapMemory(mAllocator, dst.allocation);
// }

// VkAccessFlags getAccessFlag(ResourceStage stage) {
//   switch (stage) {
//   case ResourceStage::UniformTaskMesh:
//   case ResourceStage::UniformCompute:
//   case ResourceStage::UniformVertexFragment:
//     return VK_ACCESS_UNIFORM_READ_BIT;
//   case ResourceStage::StorageVertexFragment:
//   case ResourceStage::StorageTaskMesh:
//   case ResourceStage::StorageCompute:
//     return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
//   default:
//     assert(false);
//     return VK_ACCESS_NONE;
//   }
// }

// VkPipelineStageFlags getStageFlags(ResourceStage stage) {
//   switch (stage) {
//   case ResourceStage::UniformTaskMesh:
//   case ResourceStage::StorageTaskMesh:
//     return VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT |
//            VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;

//   case ResourceStage::UniformCompute:
//   case ResourceStage::StorageCompute:
//     return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

//   case ResourceStage::StorageVertexFragment:
//   case ResourceStage::UniformVertexFragment:
//     return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
//            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

//   default:
//     assert(false);
//     return VK_PIPELINE_STAGE_NONE;
//   }
// }

// void DeviceResourceAllocator::StagingBuffer(const CommandBuffer &cmd, u32 size,
//                                             ResourceHandle src,
//                                             ResourceHandle dst, u32 srcOffset,
//                                             u32 dstOffset,
//                                             ResourceStage stage) {
//   auto sRes = mResources.getResource(src);
//   assert(sRes.type == ResourceType::Buffer);
//   auto dRes = mResources.getResource(src);
//   assert(dRes.type == ResourceType::Buffer);
//   StagingBuffer(cmd, size, sRes.data.buffer, dRes.data.buffer, srcOffset,
//                 dstOffset, stage);
// }

// void DeviceResourceAllocator::StagingBuffer(const CommandBuffer &cmd, u32 size,
//                                             Buffer src, Buffer dst,
//                                             u32 srcOffset, u32 dstOffset,
//                                             ResourceStage stage) {
//   VkBufferCopy copyRegion{};
//   copyRegion.srcOffset = srcOffset;
//   copyRegion.dstOffset = dstOffset;
//   copyRegion.size = size;
//   vkCmdCopyBuffer(cmd.buffer, src.buffer, dst.buffer, 1, &copyRegion);

//   VkBufferMemoryBarrier barrier =
//       SynchronizationManager::CreateBufferMemoryBarrier(
//           VK_ACCESS_TRANSFER_WRITE_BIT, getAccessFlag(stage), dst.buffer);

//   vkCmdPipelineBarrier(cmd.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
//                        getStageFlags(stage), 0, 0, nullptr, 1, &barrier, 0,
//                        nullptr);
// }

// void DeviceResourceAllocator::Reset() {
//   std::vector<Resource> resources;
//   mResources.getResources(resources);
//   for (auto res : resources) {
//     switch (res.type) {
//     case ResourceType::Buffer: {
//       vmaDestroyBuffer(mAllocator, res.data.buffer.buffer,
//                        res.data.buffer.allocation);
//       break;
//     }
//     case ResourceType::Image: {
//       vmaDestroyImage(mAllocator, res.data.image.image,
//                       res.data.image.allocation);
//       vkDestroyImageView(mDevice, res.data.image.view, nullptr);
//       break;
//     }
//     default:
//       assert(false);
//     }
//   }
//   mResources.resetContainer();
// }

// void DeviceResourceAllocator::ReleaseAll() {
//   Reset();
//   vmaDestroyAllocator(mAllocator);
// }

} // namespace unknown::renderer::vulkan
