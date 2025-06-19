#include "resource.hpp"
#include "deviceResourceAllocator.hpp"
#include "vulkan_renderer/command.hpp"

namespace unknown::renderer::vulkan {
#define UNK_GLOBAL_FRAME_UNIFORM_MAX (4 * MB)
#define UNK_GLOBAL_FRAME_STORAGE_MAX (16 * MB)

ResourceManager::~ResourceManager() {}

void ResourceManager::Init(const ResourceManagerDesc &desc) {
  // mDeviceResourcePtr = std::make_unique<DeviceResourceAllocator>();
  // mDeviceResourcePtr->Init(desc.vkData);
  mVkData = desc.vkData;
  mGPUResourcePtr = std::make_unique<GPUResourceAllocator>();
  mGPUResourcePtr->Init(desc.vkData);

  mUniformRegistrationPtr = std::make_unique<UniformRegistrationManager>();

  mUniformAlignment = desc.uniformAlignment;
  mStorageAlignment = desc.storageAlignment;

  CommandManagerInitDesc cmddesc;
  cmddesc.device = desc.vkData.device;
  //mTransferCommandManagerPtr = std::make_unique<CommandManager>();

  if (desc.vkData.useTransferQueue &&
      desc.vkData.graphicsQueueFamily != desc.vkData.transferQueueFamily) {
    mUseTransferQueue = true;
    cmddesc.queueFamilyIndex = desc.vkData.transferQueueFamily;
  } else {
    mUseTransferQueue = false;
    cmddesc.queueFamilyIndex = desc.vkData.graphicsQueueFamily;
  }

  //temp
  mUseTransferQueue = true;
  cmddesc.queueFamilyIndex = desc.vkData.graphicsQueueFamily;

  // mTransferCommandManagerPtr->Init(cmddesc);
  // for(auto & c : mTransferCommand)
  // {
  //   c = mTransferCommandManagerPtr->Create();
  // }

  {
    auto uMemSize =
        alignUp(UNK_GLOBAL_FRAME_UNIFORM_MAX, desc.uniformAlignment);

    BufferDesc frameUniformDesc;
    frameUniformDesc.size = uMemSize * 4u;
    frameUniformDesc.bufferUsage.uniform().transfer_dst();
    frameUniformDesc.memoryUsage.prefer_device();
    // frameUniformDesc.memoryProperty.;

    BufferDesc frameUniformStagingDesc;
    frameUniformStagingDesc.size = uMemSize * 4u;
    frameUniformStagingDesc.bufferUsage.transfer_src().transfer_dst();
    frameUniformStagingDesc.memoryUsage.prefer_host();
    frameUniformStagingDesc.memoryProperty.staging();

    mFrameUniformBufferHandle = CreateBuffer(frameUniformDesc);
    mFrameUniformStagingBufferHandle = CreateBuffer(frameUniformStagingDesc);

    mFrameUniformMemory_CPU =
        std::make_shared<LinearMemory>(uMemSize, desc.uniformAlignment);

    mFrameUniformBuffer = GetBuffer(mFrameUniformBufferHandle);
    mFrameUniformStagingBuffer = GetBuffer(mFrameUniformStagingBufferHandle);
    mFrameUniformBufferSize = uMemSize * 4u;
    mFrameUniformMemorySize_CPU = uMemSize;

    // UniformRegistrationInfo frameBufferReg;
    //   frameBufferReg.alloc.ptr = mFrameUniformMemory_CPU->getBasePtr();
    //   frameBufferReg.alloc.reserve = uMemSize;
    //   frameBufferReg.alloc.size = uMemSize;
    //   frameBufferReg.desc.name = "frame_uniform_ring_buffer";
    //   frameBufferReg.desc.size = uMemSize * 4u;
    //   frameBufferReg.desc.type =
  }

  {
    auto uMemSize =
        alignUp(UNK_GLOBAL_FRAME_STORAGE_MAX, desc.storageAlignment);

    BufferDesc frameStorageDesc;
    frameStorageDesc.size = uMemSize * 4u;
    frameStorageDesc.bufferUsage.shader_storage().transfer_dst();
    frameStorageDesc.memoryUsage.prefer_device();
    // frameStorageDesc.memoryUsage.property_local();

    BufferDesc frameStorageStagingDesc;
    frameStorageStagingDesc.size = uMemSize * 4u;
    frameStorageStagingDesc.bufferUsage.transfer_src().transfer_dst();
    frameStorageStagingDesc.memoryUsage.prefer_host();
    frameStorageStagingDesc.memoryProperty.staging();

    mFrameStorageBufferHandle = CreateBuffer(frameStorageDesc);
    mFrameStorageStagingBufferHandle = CreateBuffer(frameStorageStagingDesc);

    mFrameStorageMemory_CPU =
        std::make_shared<LinearMemory>(uMemSize, desc.storageAlignment);

    mFrameStorageBuffer = GetBuffer(mFrameStorageBufferHandle);
    mFrameStorageStagingBuffer = GetBuffer(mFrameStorageStagingBufferHandle);
    mFrameStorageBufferSize = uMemSize * 4u;
    mFrameStorageMemorySize_CPU = uMemSize;
  }
}

BufferHandle ResourceManager::CreateBuffer(const BufferDesc &desc) {
  return mGPUResourcePtr->CreateBuffer(desc);
}

Buffer ResourceManager::GetBuffer(const BufferHandle &handle) const {
  return mGPUResourcePtr->GetBuffer(handle);
}

Buffer & ResourceManager::GetBuffer(const BufferHandle &handle) {
  return mGPUResourcePtr->GetBuffer(handle);
}

void ResourceManager::DestroyBuffer(const BufferHandle &handle) {
  mGPUResourcePtr->Release(handle);
  //mTransferCommandManagerPtr->Destroy();
}

ImageHandle ResourceManager::CreateImage(const ImageDesc &desc) {
  return mGPUResourcePtr->CreateImage(desc);
}

Image ResourceManager::GetImage(const ImageHandle &handle) {
  return mGPUResourcePtr->GetImage(handle);
}

void ResourceManager::DestroyImage(const ImageHandle &handle) {
  mGPUResourcePtr->Release(handle);
}

void ResourceManager::DestroyResource(const ImageHandle &handle) {
  mGPUResourcePtr->Release(handle);
}

void ResourceManager::RegisterUniform(const UniformDesc &desc) {
  auto it = mMemAllocationMap.find(desc.name);
  assert(it == mMemAllocationMap.end());
  if (it != mMemAllocationMap.end())
    return;

  UniformRegistrationInfo info;
  info.desc = desc;
  switch (desc.type) {
  case UniformType::FrameUniform: {
    info.alloc = addFrameUnifrom(desc.size);
    info.local_offset = getFrameUniformLocalOffset(info.alloc.ptr);
    info.handle = mFrameUniformBufferHandle;
    break;
  }
  case UniformType::FrameStorage: {
    info.alloc = addFrameStorage(desc.size);
    info.local_offset = getFrameStorageLocalOffset(info.alloc.ptr);
    info.handle = mFrameStorageBufferHandle;
    break;
  }
  default:
    assert(false);
  }
  mMemAllocationMap.insert({desc.name, info});
}

void ResourceManager::StageUniform(const std::string_view name, byte *data,
                                   u32 size) {
  auto it = mMemAllocationMap.find(name.data());
  assert(it != mMemAllocationMap.end());
  const auto &alloc = it->second.alloc;
  assert(alloc.size >= size);

  memcpy(alloc.ptr, data, size);
}

UniformRegistrationInfo ResourceManager::GetBindingInfo(const SString &name) {
  auto it = mMemAllocationMap.find(name);
  if (it == mMemAllocationMap.end())
    return UniformRegistrationInfo();

  auto info = it->second;
  info.global_offset = getGlobalOffset(info);

  return it->second;
}

void ResourceManager::FlushFrameResource(std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> cmdManager, Semaphore start, Semaphore finish) {
  // temp
  // if (mUseTransferCommand) {
  // VkCommandBufferBeginInfo info{};
  // info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // info.pNext = nullptr;
  // info.pInheritanceInfo = nullptr;
  // info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  //auto cmd = mTransferCommand[mTransferCommandIndex];
  auto cmd = cmdManager->BeginCommandBuffer(QueueType::Transfer);

  //VK_CHECK(vkBeginCommandBuffer(cmd.buffer, &info));

  stagingFrameResource(cmd);

  cmdManager->EndCommandBuffer(cmd);
  //VK_CHECK(vkEndCommandBuffer(cmd.buffer));

  VkCommandBufferSubmitInfo cmdinfo = {};
  cmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdinfo.pNext = nullptr;
  cmdinfo.commandBuffer = cmd.buffer;
  cmdinfo.deviceMask = 0;

  VkSemaphoreSubmitInfo waitinfo = {};
  waitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  waitinfo.pNext = nullptr;
  waitinfo.semaphore = start.data;
  waitinfo.stageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;//VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
  waitinfo.deviceIndex = 0;
  waitinfo.value = 1;

  VkSemaphoreSubmitInfo signalinfo = {};
  signalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signalinfo.pNext = nullptr;
  signalinfo.semaphore = finish.data;
  signalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;//VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  signalinfo.deviceIndex = 0;
  signalinfo.value = 1;

  VkSubmitInfo2 submitinfo = {};
  submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submitinfo.pNext = nullptr;

  submitinfo.waitSemaphoreInfoCount = 1;
  submitinfo.pWaitSemaphoreInfos = &waitinfo;

  submitinfo.signalSemaphoreInfoCount = 1;
  submitinfo.pSignalSemaphoreInfos = &signalinfo;

  submitinfo.commandBufferInfoCount = 1;
  submitinfo.pCommandBufferInfos = &cmdinfo;

  auto queue = mVkData.useTransferQueue && mVkData.hasTransferQueue
                   ? mVkData.transferQueue
                   : mVkData.graphicsQueue;

  //temp
  //queue = mVkData.graphicsQueue;

  VK_CHECK(vkQueueSubmit2(queue, 1, &submitinfo, VK_NULL_HANDLE));

  frame();
}

void ResourceManager::Reset() { mGPUResourcePtr->ReleaseAll(); }

void ResourceManager::Destroy() { 
  //mTransferCommandManagerPtr->Destroy();
  mGPUResourcePtr->Destroy(); 
}

void ResourceManager::destroy() {} // vmaDestroyAllocator(mAllocator); }

void ResourceManager::stagingFrameResource(const CommandBuffer &cmd) {
  {
    // mDeviceResourcePtr->MapBuffer(
    //     mFrameUniformMemorySize_CPU, mFrameUniformMemory_CPU->getBasePtr(),
    //     mFrameUniformStagingBuffer, mFrameUniformOffset);
    // mDeviceResourcePtr->StagingBuffer(
    //     cmd, mFrameUniformMemorySize_CPU, mFrameUniformStagingBuffer,
    //     mFrameUniformBuffer, mFrameUniformOffset, mFrameUniformOffset,
    //     ResourceStage::UniformTaskMesh);

    mGPUResourcePtr->MapCopyBuffer(
        mFrameUniformMemory_CPU->getBasePtr(), mFrameUniformStagingBufferHandle,
        mFrameUniformOffset, mFrameUniformMemorySize_CPU);
    mGPUResourcePtr->GPUCopyBuffer(
        cmd, mFrameUniformStagingBufferHandle,
        mFrameUniformBufferHandle, mFrameUniformOffset, mFrameUniformOffset, mFrameUniformMemorySize_CPU,
        ResourceStage::UniformTaskMesh);
  }

  {
    // mGPUResourcePtr->MapBuffer(
    //     mFrameStorageMemorySize_CPU, mFrameStorageMemory_CPU->getBasePtr(),
    //     mFrameStorageStagingBuffer, mFrameStorageOffset);
    // mGPUResourcePtr->StagingBuffer(
    //     cmd, mFrameStorageMemorySize_CPU, mFrameStorageStagingBuffer,
    //     mFrameStorageBuffer, mFrameStorageOffset, mFrameStorageOffset,
    //     ResourceStage::UniformTaskMesh);
    mGPUResourcePtr->MapCopyBuffer(
        mFrameStorageMemory_CPU->getBasePtr(), mFrameStorageStagingBufferHandle,
        mFrameStorageOffset, mFrameStorageMemorySize_CPU);
    mGPUResourcePtr->GPUCopyBuffer(
        cmd, mFrameStorageStagingBufferHandle,
        mFrameStorageBufferHandle, mFrameStorageOffset, mFrameStorageOffset, mFrameStorageMemorySize_CPU,
        ResourceStage::UniformTaskMesh);
  }
}

MemoryAllocation ResourceManager::addFrameUnifrom(u32 size) {
  return mFrameUniformMemory_CPU->allocate(size);
}

u32 ResourceManager::getFrameUniformOffset() const {
  return mFrameUniformOffset;
}
u32 ResourceManager::getFrameStorageOffset() const {
  return mFrameStorageOffset;
}
u32 ResourceManager::getFrameUniformLocalOffset(const void *ptr) const {
  auto bPtr = reinterpret_cast<u8 *>(mFrameUniformMemory_CPU->getBasePtr());
  assert(ptr >= bPtr && ptr < bPtr + mFrameUniformMemorySize_CPU);
  return u32((byte *)ptr - bPtr);
}
u32 ResourceManager::getFrameStorageLocalOffset(const void *ptr) const {
  auto bPtr = reinterpret_cast<u8 *>(mFrameStorageMemory_CPU->getBasePtr());
  assert(ptr >= bPtr && ptr < bPtr + mFrameStorageMemorySize_CPU);
  return u32((byte *)ptr - bPtr);
}

u32 ResourceManager::getGlobalOffset(UniformRegistrationInfo &info) const {
  switch (info.desc.type) {
  case UniformType::FrameUniform: {
    return getFrameUniformOffset();
    break;
  }
  case UniformType::FrameStorage: {
    return getFrameStorageOffset();
    break;
  }
  default:
    return 0u;
    // assert(false);
  }
}

u32 ResourceManager::getLocalOffset(UniformRegistrationInfo &info) const {
  switch (info.desc.type) {
  case UniformType::FrameUniform: {
    return getFrameUniformLocalOffset(info.alloc.ptr);
    break;
  }
  case UniformType::FrameStorage: {
    return getFrameStorageLocalOffset(info.alloc.ptr);
    break;
  }
  default:
    return 0u;
    // assert(false);
  }
}

MemoryAllocation ResourceManager::getFrameUniformsHostMemory() const {
  MemoryAllocation alloc;

  alloc.ptr = reinterpret_cast<u8 *>(mFrameUniformMemory_CPU->getBasePtr());
  alloc.size = mFrameUniformMemory_CPU->getOffset();
  alloc.reserve = mFrameUniformMemory_CPU->getOffset();

  return alloc;
}

MemoryAllocation ResourceManager::addFrameStorage(u32 size) {
  return mFrameStorageMemory_CPU->allocate(size);
}

MemoryAllocation ResourceManager::getFrameStoragesHostMemory() const {
  MemoryAllocation alloc;

  alloc.ptr = reinterpret_cast<u8 *>(mFrameStorageMemory_CPU->getBasePtr());
  alloc.size = mFrameStorageMemory_CPU->getOffset();
  alloc.reserve = mFrameStorageMemory_CPU->getOffset();

  return alloc;
}

void ResourceManager::frame() {
  //mTransferCommandIndex =  (mTransferCommandIndex + 1u) % 2u;

  mFrameUniformOffset += mFrameUniformMemorySize_CPU;
  mFrameUniformOffset = mFrameUniformOffset + mFrameUniformMemorySize_CPU >=
                                mFrameUniformBufferSize
                            ? 0
                            : mFrameUniformOffset;

  mFrameStorageOffset += mFrameStorageMemorySize_CPU;
  mFrameStorageOffset = mFrameStorageOffset + mFrameStorageMemorySize_CPU >=
                                mFrameStorageBufferSize
                            ? 0
                            : mFrameStorageOffset;
}
} // namespace unknown::renderer::vulkan