#pragma once

#include "utils/container.hpp"
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/defines.hpp"

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

template <u32 FRAME_COUNT> class SynchronizationManager {
private:
  struct FramePool {
    std::queue<SemaphoreHandle> freeSemaphores;
    std::vector<SemaphoreHandle> usedSemaphores;
    std::queue<FenceHandle> freeFences;
    std::vector<FenceHandle> usedFences;

    SemaphoreHandle acquireSemaphore() {
      auto h = freeSemaphores.front();
      freeSemaphores.pop();
      usedSemaphores.push_back(h);
      return h;
    }

    FenceHandle acquireFence() {
      auto h = freeFences.front();
      freeFences.pop();
      usedFences.push_back(h);
      return h;
    }

    void reset() {
      for (auto s : usedSemaphores) {
        freeSemaphores.push(s);
      }
      usedSemaphores.clear();

      for (auto s : usedFences) {
        freeFences.push(s);
      }
      usedFences.clear();
    }
  };

  friend class FramePool;

public:
  static VkBufferMemoryBarrier
  CreateBufferMemoryBarrier(VkAccessFlags sFlags, VkAccessFlags dFlags,
                            VkBuffer buffer,
                            u32 queueSrc = VK_QUEUE_FAMILY_IGNORED,
                            u32 queueDst = VK_QUEUE_FAMILY_IGNORED) {
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = sFlags;
    barrier.dstAccessMask = dFlags;
    barrier.srcQueueFamilyIndex = queueSrc;
    barrier.dstQueueFamilyIndex = queueDst;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    return barrier;
  }

  static VkImageMemoryBarrier2
  CreateImageMemoryBarrier(VkAccessFlags sFlags, VkAccessFlags dFlags,
                           VkImage img,
                           u32 queueSrc = VK_QUEUE_FAMILY_IGNORED,
                           u32 queueDst = VK_QUEUE_FAMILY_IGNORED) {
    VkImageMemoryBarrier2 barrier{};
    barrier.srcQueueFamilyIndex = queueSrc;
    barrier.dstQueueFamilyIndex = queueDst;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcAccessMask = sFlags;
    barrier.dstAccessMask = dFlags;
    barrier.srcQueueFamilyIndex = queueSrc;
    barrier.dstQueueFamilyIndex = queueDst;

    return barrier;
  }

public:
  void Init(const SyncInitDesc &desc) { mDevice = desc.device; };

  SemaphoreHandle AcquireFrameSemaphore() {
    if (mFramePool[mCurrentFrameIndex].freeSemaphores.empty()) {
      auto h = createSemaphore();
      mFramePool[mCurrentFrameIndex].usedSemaphores.push_back(h);
      return h;
    }

    return mFramePool[mCurrentFrameIndex].acquireSemaphore();
  }

  FenceHandle AcquireFrameFence() {
    if (mFramePool[mCurrentFrameIndex].freeFences.empty()) {
      auto h = createFence();
      mFramePool[mCurrentFrameIndex].usedFences.push_back(h);
      return h;
    }

    return mFramePool[mCurrentFrameIndex].acquireFence();
  }

  SemaphoreHandle AcquireSemaphore() {
    if (mFreeSemaphores.empty()) {
      auto h = createSemaphore();
      mFreeSemaphores.push(h);
      return h;
    }

    auto h = mFreeSemaphores.front();
    mFreeSemaphores.pop();
    mUsedSemaphores.push_back(h);
    return h;
  }
  FenceHandle AcquireFence() {
    if (mFreeFences.empty()) {
      auto h = createFence();
      mFreeFences.push(h);
      return h;
    }

    auto h = mFreeFences.front();
    mFreeFences.pop();
    mUsedFences.push_back(h);
    return h;
  }

  Semaphore GetSemaphore(const SemaphoreHandle &h) {
    return mSemaphores.get(h);
  }
  Fence GetFence(const FenceHandle &h) { return mFences.get(h); }

  void ResetFrameResource() { mFramePool[mCurrentFrameIndex].reset(); }

  void Destroy() {
    auto &semas = mSemaphores.getContainer();
    for (auto &s : semas) {
      if (s.first)
        ;
      vkDestroySemaphore(mDevice, s.second.data, nullptr);
    }

    auto &fens = mFences.getContainer();
    for (auto &f : fens) {
      if (f.first)
        ;
      vkDestroyFence(mDevice, f.second.data, nullptr);
    }

    mSemaphores.reset();
    mFences.reset();
  }

  void Advance() {
    mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameCount;
  }

private:
  SemaphoreHandle createSemaphore() {
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    Semaphore sema;
    VK_CHECK(vkCreateSemaphore(mDevice, &info, nullptr, &sema.data));

    auto h = mSemaphores.push(sema);
    assert(h.isValid());

    return h;
  }

  FenceHandle createFence() {
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    Fence fence;
    VK_CHECK(vkCreateFence(mDevice, &info, nullptr, &fence.data));

    auto h = mFences.push(fence);
    assert(h.isValid());
    return h;
  }

  VkDevice mDevice;
  VectorContainer<SemaphoreHandle, Semaphore> mSemaphores;
  VectorContainer<FenceHandle, Fence> mFences;

  std::queue<SemaphoreHandle> mFreeSemaphores;
  std::vector<SemaphoreHandle> mUsedSemaphores;
  std::queue<FenceHandle> mFreeFences;
  std::vector<FenceHandle> mUsedFences;

  const u32 mFrameCount = FRAME_COUNT;
  u32 mCurrentFrameIndex = 0u;
  std::array<FramePool, FRAME_COUNT> mFramePool;
};
} // namespace unknown::renderer::vulkan