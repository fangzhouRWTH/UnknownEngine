#pragma once

#include "utils/container.hpp"
#include "vulkan_renderer/common.hpp"


namespace unknown::renderer::vulkan {
struct SemaphoreInitDesc {};

struct FenceInitDesc {};

struct Semaphore {
  VkSemaphore data;
};

struct Fence {
  VkFence data;
};

DECL_CONTAINER_HANDLE(SemaphoreHandle, NoRecycle, VectorContainer, Semaphore);
DECL_CONTAINER_HANDLE(FenceHandle, NoRecycle, VectorContainer, Fence);

struct SyncInitDesc {
  VkDevice device;
};

class SynchronizationManager {
public:
  static VkBufferMemoryBarrier CreateBufferMemoryBarrier(VkAccessFlags sFlags, VkAccessFlags dFlags,VkBuffer buffer, u32 queueSrc = VK_QUEUE_FAMILY_IGNORED,u32 queueDst = VK_QUEUE_FAMILY_IGNORED);

public:
  void Init(const SyncInitDesc &desc);
  SemaphoreHandle CreateSemaphore();
  FenceHandle CreateFence();

  Semaphore GetSemaphore(const SemaphoreHandle &h);
  Fence GetFence(const FenceHandle &h);

  void Destroy();

private:
  VkDevice mDevice;
  VectorContainer<SemaphoreHandle, Semaphore> mSemaphores;
  VectorContainer<FenceHandle, Fence> mFences;
};
} // namespace unknown::renderer::vulkan