#include "resource.hpp"
#include "deviceResourceAllocator.hpp"
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/sync.hpp"

namespace unknown::renderer::vulkan {
#define UNK_GLOBAL_FRAME_UNIFORM_MAX (4 * MB)
#define UNK_GLOBAL_FRAME_STORAGE_MAX (16 * MB)

ResourceManager::~ResourceManager() {}

void ResourceManager::Init(const ResourceManagerDesc &desc) {
  mVkData = desc.vkData;
  mGPUResourcePtr = std::make_unique<GPUResourceAllocator>();
  mGPUResourcePtr->Init(desc.vkData);

  assert(desc.syncManager != nullptr);
  mSyncManagerPtr = desc.syncManager;
  mCmdManagerPtr = desc.cmdManager;

  mUniformRegistrationPtr = std::make_unique<UniformRegistrationManager>();

  mUniformAlignment = desc.uniformAlignment;
  mStorageAlignment = desc.storageAlignment;

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

Buffer &ResourceManager::GetBuffer(const BufferHandle &handle) {
  return mGPUResourcePtr->GetBuffer(handle);
}

void ResourceManager::DestroyBuffer(const BufferHandle &handle) {
  mGPUResourcePtr->Release(handle);
  // mTransferCommandManagerPtr->Destroy();
}

ImageHandle ResourceManager::CreateImage(const ImageDesc &desc) {
  return mGPUResourcePtr->CreateImage(desc);
}

ImageHandle ResourceManager::CreateImage(ImageDesc &desc,
                                         const std::string &filePath) {
  u32 width, height, channels;
  return mGPUResourcePtr->LoadCreateImage(mCmdManagerPtr, true, desc, filePath, width, height,
                                          channels);
}

Image ResourceManager::GetImage(const ImageHandle &handle) {
  return mGPUResourcePtr->GetImage(handle);
}

void ResourceManager::TransitionImage(const ImageHandle &handle,
                                      const ImageTransitionDesc &desc) {
                                        mGPUResourcePtr->TransitionImageResource(desc.cmd,handle,desc.newLayout);
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

void ResourceManager::FlushFrameResource(Semaphore start, Semaphore finish) {

  bool needOwnershipTransfer =
      mVkData.useTransferQueue && mVkData.hasTransferQueue;
  Semaphore flushWaitSemaphore;

  if (needOwnershipTransfer) {
    auto gcmd = mCmdManagerPtr->BeginCommandBuffer(QueueType::Graphic);
    auto tcmd = mCmdManagerPtr->BeginCommandBuffer(QueueType::Transfer);
    acquireFrameResourceOwnership(gcmd, tcmd);
    mCmdManagerPtr->EndCommandBuffer(gcmd);
    mCmdManagerPtr->EndCommandBuffer(tcmd);

    VkCommandBufferSubmitInfo gcmdinfo = {};
    gcmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    gcmdinfo.pNext = nullptr;
    gcmdinfo.commandBuffer = gcmd.buffer;
    gcmdinfo.deviceMask = 0;

    VkCommandBufferSubmitInfo tcmdinfo = {};
    tcmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    tcmdinfo.pNext = nullptr;
    tcmdinfo.commandBuffer = tcmd.buffer;
    tcmdinfo.deviceMask = 0;

    auto gh = mSyncManagerPtr->AcquireFrameSemaphore();
    auto gReleaseSema = mSyncManagerPtr->GetSemaphore(gh);

    auto th = mSyncManagerPtr->AcquireFrameSemaphore();
    auto tAcquireSema = mSyncManagerPtr->GetSemaphore(th);

    flushWaitSemaphore = tAcquireSema;

    // release
    VkSemaphoreSubmitInfo releaseWaitinfo = {};
    releaseWaitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    releaseWaitinfo.pNext = nullptr;
    releaseWaitinfo.semaphore = start.data;
    releaseWaitinfo.stageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    releaseWaitinfo.deviceIndex = 0;
    releaseWaitinfo.value = 1;

    VkSemaphoreSubmitInfo releaseSignalinfo = {};
    releaseSignalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    releaseSignalinfo.pNext = nullptr;
    releaseSignalinfo.semaphore = gReleaseSema.data;
    releaseSignalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    releaseSignalinfo.deviceIndex = 0;
    releaseSignalinfo.value = 1;

    VkSubmitInfo2 releaseSubmitinfo = {};
    releaseSubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    releaseSubmitinfo.pNext = nullptr;

    releaseSubmitinfo.waitSemaphoreInfoCount = 1;
    releaseSubmitinfo.pWaitSemaphoreInfos = &releaseWaitinfo;

    releaseSubmitinfo.signalSemaphoreInfoCount = 1;
    releaseSubmitinfo.pSignalSemaphoreInfos = &releaseSignalinfo;

    releaseSubmitinfo.commandBufferInfoCount = 1;
    releaseSubmitinfo.pCommandBufferInfos = &gcmdinfo;
    VK_CHECK(vkQueueSubmit2(mVkData.graphicsQueue, 1, &releaseSubmitinfo,
                            VK_NULL_HANDLE));

    // acquire
    VkSemaphoreSubmitInfo acquireWaitinfo = {};
    acquireWaitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    acquireWaitinfo.pNext = nullptr;
    acquireWaitinfo.semaphore = gReleaseSema.data;
    acquireWaitinfo.stageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    acquireWaitinfo.deviceIndex = 0;
    acquireWaitinfo.value = 1;

    VkSemaphoreSubmitInfo acquireSignalinfo = {};
    acquireSignalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    acquireSignalinfo.pNext = nullptr;
    acquireSignalinfo.semaphore = tAcquireSema.data;
    acquireSignalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    acquireSignalinfo.deviceIndex = 0;
    acquireSignalinfo.value = 1;

    VkSubmitInfo2 acquireSubmitinfo = {};
    acquireSubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    acquireSubmitinfo.pNext = nullptr;

    acquireSubmitinfo.waitSemaphoreInfoCount = 1;
    acquireSubmitinfo.pWaitSemaphoreInfos = &acquireWaitinfo;

    acquireSubmitinfo.signalSemaphoreInfoCount = 1;
    acquireSubmitinfo.pSignalSemaphoreInfos = &acquireSignalinfo;

    acquireSubmitinfo.commandBufferInfoCount = 1;
    acquireSubmitinfo.pCommandBufferInfos = &tcmdinfo;
    VK_CHECK(vkQueueSubmit2(mVkData.transferQueue, 1, &acquireSubmitinfo,
                            VK_NULL_HANDLE));
  } else {
    flushWaitSemaphore = start;
  }

  auto cmd = mCmdManagerPtr->BeginCommandBuffer(QueueType::Transfer);
  stagingFrameResource(cmd);
  mCmdManagerPtr->EndCommandBuffer(cmd);

  Semaphore flushSignalSemaphore;
  if (needOwnershipTransfer) {
    auto flsh = mSyncManagerPtr->AcquireFrameSemaphore();
    flushSignalSemaphore = mSyncManagerPtr->GetSemaphore(flsh);
  } else {
    flushSignalSemaphore = finish;
  }

  {
    VkCommandBufferSubmitInfo cmdinfo = {};
    cmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdinfo.pNext = nullptr;
    cmdinfo.commandBuffer = cmd.buffer;
    cmdinfo.deviceMask = 0;

    VkSemaphoreSubmitInfo waitinfo = {};
    waitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitinfo.pNext = nullptr;
    waitinfo.semaphore = flushWaitSemaphore.data;
    waitinfo.stageMask =
        VK_PIPELINE_STAGE_TRANSFER_BIT; // VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
    waitinfo.deviceIndex = 0;
    waitinfo.value = 1;

    VkSemaphoreSubmitInfo signalinfo = {};
    signalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalinfo.pNext = nullptr;
    signalinfo.semaphore = flushSignalSemaphore.data;
    signalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    ;
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

    auto queue =
        needOwnershipTransfer ? mVkData.transferQueue : mVkData.graphicsQueue;

    VK_CHECK(vkQueueSubmit2(queue, 1, &submitinfo, VK_NULL_HANDLE));
  }

  if (needOwnershipTransfer) {
    auto gcmd = mCmdManagerPtr->BeginCommandBuffer(QueueType::Graphic);
    auto tcmd = mCmdManagerPtr->BeginCommandBuffer(QueueType::Transfer);
    releaseFrameResourceOwnership(gcmd, tcmd);
    mCmdManagerPtr->EndCommandBuffer(gcmd);
    mCmdManagerPtr->EndCommandBuffer(tcmd);

    VkCommandBufferSubmitInfo gcmdinfo = {};
    gcmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    gcmdinfo.pNext = nullptr;
    gcmdinfo.commandBuffer = gcmd.buffer;
    gcmdinfo.deviceMask = 0;

    VkCommandBufferSubmitInfo tcmdinfo = {};
    tcmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    tcmdinfo.pNext = nullptr;
    tcmdinfo.commandBuffer = tcmd.buffer;
    tcmdinfo.deviceMask = 0;

    auto th = mSyncManagerPtr->AcquireFrameSemaphore();
    auto tReleaseSema = mSyncManagerPtr->GetSemaphore(th);

    // release
    VkSemaphoreSubmitInfo releaseWaitinfo = {};
    releaseWaitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    releaseWaitinfo.pNext = nullptr;
    releaseWaitinfo.semaphore = flushSignalSemaphore.data;
    releaseWaitinfo.stageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    releaseWaitinfo.deviceIndex = 0;
    releaseWaitinfo.value = 1;

    VkSemaphoreSubmitInfo releaseSignalinfo = {};
    releaseSignalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    releaseSignalinfo.pNext = nullptr;
    releaseSignalinfo.semaphore = tReleaseSema.data;
    releaseSignalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    releaseSignalinfo.deviceIndex = 0;
    releaseSignalinfo.value = 1;

    VkSubmitInfo2 releaseSubmitinfo = {};
    releaseSubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    releaseSubmitinfo.pNext = nullptr;

    releaseSubmitinfo.waitSemaphoreInfoCount = 1;
    releaseSubmitinfo.pWaitSemaphoreInfos = &releaseWaitinfo;

    releaseSubmitinfo.signalSemaphoreInfoCount = 1;
    releaseSubmitinfo.pSignalSemaphoreInfos = &releaseSignalinfo;

    releaseSubmitinfo.commandBufferInfoCount = 1;
    releaseSubmitinfo.pCommandBufferInfos = &tcmdinfo;
    VK_CHECK(vkQueueSubmit2(mVkData.transferQueue, 1, &releaseSubmitinfo,
                            VK_NULL_HANDLE));

    // acquire
    VkSemaphoreSubmitInfo acquireWaitinfo = {};
    acquireWaitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    acquireWaitinfo.pNext = nullptr;
    acquireWaitinfo.semaphore = tReleaseSema.data;
    acquireWaitinfo.stageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    acquireWaitinfo.deviceIndex = 0;
    acquireWaitinfo.value = 1;

    VkSemaphoreSubmitInfo acquireSignalinfo = {};
    acquireSignalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    acquireSignalinfo.pNext = nullptr;
    acquireSignalinfo.semaphore = finish.data;
    acquireSignalinfo.stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    acquireSignalinfo.deviceIndex = 0;
    acquireSignalinfo.value = 1;

    VkSubmitInfo2 acquireSubmitinfo = {};
    acquireSubmitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    acquireSubmitinfo.pNext = nullptr;

    acquireSubmitinfo.waitSemaphoreInfoCount = 1;
    acquireSubmitinfo.pWaitSemaphoreInfos = &acquireWaitinfo;

    acquireSubmitinfo.signalSemaphoreInfoCount = 1;
    acquireSubmitinfo.pSignalSemaphoreInfos = &acquireSignalinfo;

    acquireSubmitinfo.commandBufferInfoCount = 1;
    acquireSubmitinfo.pCommandBufferInfos = &gcmdinfo;
    VK_CHECK(vkQueueSubmit2(mVkData.graphicsQueue, 1, &acquireSubmitinfo,
                            VK_NULL_HANDLE));
  }

  frame();
}

void ResourceManager::Reset() { mGPUResourcePtr->ReleaseAll(); }

void ResourceManager::Destroy() {
  // mTransferCommandManagerPtr->Destroy();
  mGPUResourcePtr->Destroy();
}

void ResourceManager::destroy() {} // vmaDestroyAllocator(mAllocator); }

void ResourceManager::acquireFrameResourceOwnership(const CommandBuffer &gcmd,
                                                    const CommandBuffer &tcmd) {
  mGPUResourcePtr->ReleaseOwnership(gcmd, mFrameUniformBufferHandle,
                                    QueueType::Graphic, QueueType::Transfer);
  mGPUResourcePtr->ReleaseOwnership(gcmd, mFrameStorageBufferHandle,
                                    QueueType::Graphic, QueueType::Transfer);

  mGPUResourcePtr->AcquireOwnership(tcmd, mFrameUniformBufferHandle,
                                    QueueType::Graphic, QueueType::Transfer);
  mGPUResourcePtr->AcquireOwnership(tcmd, mFrameStorageBufferHandle,
                                    QueueType::Graphic, QueueType::Transfer);
}

void ResourceManager::releaseFrameResourceOwnership(const CommandBuffer &gcmd,
                                                    const CommandBuffer &tcmd) {
  mGPUResourcePtr->ReleaseOwnership(tcmd, mFrameUniformBufferHandle,
                                    QueueType::Transfer, QueueType::Graphic);
  mGPUResourcePtr->ReleaseOwnership(tcmd, mFrameStorageBufferHandle,
                                    QueueType::Transfer, QueueType::Graphic);

  mGPUResourcePtr->AcquireOwnership(gcmd, mFrameUniformBufferHandle,
                                    QueueType::Transfer, QueueType::Graphic);
  mGPUResourcePtr->AcquireOwnership(gcmd, mFrameStorageBufferHandle,
                                    QueueType::Transfer, QueueType::Graphic);
}

void ResourceManager::stagingFrameResource(const CommandBuffer &cmd) {
  {
    mGPUResourcePtr->MapCopyBuffer(
        mFrameUniformMemory_CPU->getBasePtr(), mFrameUniformStagingBufferHandle,
        mFrameUniformOffset, mFrameUniformMemorySize_CPU);

    mGPUResourcePtr->GPUCopyBuffer(
        cmd, mFrameUniformStagingBufferHandle, mFrameUniformBufferHandle,
        mFrameUniformOffset, mFrameUniformOffset, mFrameUniformMemorySize_CPU,
        ResourceStage::UniformTaskMesh);
  }

  {
    mGPUResourcePtr->MapCopyBuffer(
        mFrameStorageMemory_CPU->getBasePtr(), mFrameStorageStagingBufferHandle,
        mFrameStorageOffset, mFrameStorageMemorySize_CPU);
    mGPUResourcePtr->GPUCopyBuffer(
        cmd, mFrameStorageStagingBufferHandle, mFrameStorageBufferHandle,
        mFrameStorageOffset, mFrameStorageOffset, mFrameStorageMemorySize_CPU,
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
  // mTransferCommandIndex =  (mTransferCommandIndex + 1u) % 2u;

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