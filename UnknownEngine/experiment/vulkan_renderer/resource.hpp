#pragma once

#include "vulkan_renderer/common.hpp"

//#include "buffer.hpp"
//#include "image.hpp"
#include <vector>

#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/defines.hpp"
#include "vulkan_renderer/deviceResourceAllocator.hpp"
#include "vulkan_renderer/memAllocation.hpp"
#include "vulkan_renderer/sstring.hpp"
#include "vulkan_renderer/sync.hpp"
#include "vulkan_renderer/image.hpp"

#include <memory>

namespace unknown::renderer::vulkan {

template <u32> class CommandBufferManager;

template <u32> class SynchronizationManager;

struct ResourceManagerDesc {

  VulkanCoreData vkData;

  std::shared_ptr<SynchronizationManager<FRAME_OVERLAP>> syncManager;
  std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> cmdManager;

  u32 uniformAlignment = 0;
  u32 storageAlignment = 0;
};

class StagingMemoryPool {};

class ResourceManager {
private:
public:
  ~ResourceManager();
  void Init(const ResourceManagerDesc &desc);

  BufferHandle CreateBuffer(const BufferDesc &desc);
  Buffer GetBuffer(const BufferHandle &handle) const;
  Buffer &GetBuffer(const BufferHandle &handle);
  void DestroyBuffer(const BufferHandle &handle);

  ImageHandle CreateImage(const ImageDesc &desc);
  ImageHandle CreateImage(ImageDesc &desc, const std::string &filePath);
  Image GetImage(const ImageHandle &handle);
  void TransitionImage(const ImageHandle &handle, const ImageTransitionDesc & desc);

  void DestroyImage(const ImageHandle &handle);
  void DestroyResource(const ImageHandle &handle);
  //
  void RegisterUniform(const UniformDesc &desc);

  void StageUniform(const std::string_view name, byte *data, u32 size);

  UniformRegistrationInfo GetBindingInfo(const SString &name);

  void FlushFrameResource(Semaphore start, Semaphore finish);

  void Reset();

  void Destroy();

private:
  void destroy();

  void acquireFrameResourceOwnership(const CommandBuffer &gcmd,
                                     const CommandBuffer &tcmd);
  void releaseFrameResourceOwnership(const CommandBuffer &gcmd,
                                     const CommandBuffer &tcmd);
  void stagingFrameResource(const CommandBuffer &cmd);

  MemoryAllocation addUniform(u32 size);
  MemoryAllocation addStorage(u32 size);

  MemoryAllocation addFrameUnifrom(u32 size);
  MemoryAllocation getFrameUniformsHostMemory() const;

  MemoryAllocation addFrameStorage(u32 size);
  MemoryAllocation getFrameStoragesHostMemory() const;

  u32 getFrameUniformLocalOffset(const void *ptr) const;
  u32 getFrameUniformOffset() const;

  u32 getFrameStorageLocalOffset(const void *ptr) const;
  u32 getFrameStorageOffset() const;

  u32 getGlobalOffset(UniformRegistrationInfo &info) const;
  u32 getLocalOffset(UniformRegistrationInfo &info) const;

  void frame();

private:
  // std::unique_ptr<DeviceResourceAllocator> mDeviceResourcePtr;
  VulkanCoreData mVkData;

  std::shared_ptr<SynchronizationManager<FRAME_OVERLAP>> mSyncManagerPtr;
  std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> mCmdManagerPtr;
  std::unique_ptr<GPUResourceAllocator> mGPUResourcePtr;
  std::unique_ptr<UniformRegistrationManager> mUniformRegistrationPtr;

  bool mUseTransferQueue = false;

  std::unordered_map<SString, UniformRegistrationInfo> mMemAllocationMap;

  u32 mUniformAlignment = 0u;
  u32 mStorageAlignment = 0u;
  // pre allocated buffers

  std::shared_ptr<LinearMemory> mFrameUniformMemory_CPU = nullptr;
  u32 mFrameUniformMemorySize_CPU = 0u;

  std::shared_ptr<LinearMemory> mFrameStorageMemory_CPU = nullptr;
  u32 mFrameStorageMemorySize_CPU = 0u;

  Buffer mFrameUniformBuffer;
  Buffer mFrameUniformStagingBuffer;
  BufferHandle mFrameUniformBufferHandle;
  BufferHandle mFrameUniformStagingBufferHandle;
  u32 mFrameUniformBufferSize = 0u;
  u32 mFrameUniformOffset = 0u;

  Buffer mFrameStorageBuffer;
  Buffer mFrameStorageStagingBuffer;
  BufferHandle mFrameStorageBufferHandle;
  BufferHandle mFrameStorageStagingBufferHandle;
  u32 mFrameStorageBufferSize = 0u;
  u32 mFrameStorageOffset = 0u;
};
} // namespace unknown::renderer::vulkan