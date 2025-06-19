#include "vulkan_renderer/sync.hpp"
#include "sync.hpp"

namespace unknown::renderer::vulkan {
VkBufferMemoryBarrier SynchronizationManager::CreateBufferMemoryBarrier(
    VkAccessFlags sFlags, VkAccessFlags dFlags, VkBuffer buffer, u32 queueSrc,
    u32 queueDst) {
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

void SynchronizationManager::Init(const SyncInitDesc &desc) {
  mDevice = desc.device;
};
// Semaphore Semaphore::Create(const SemaphoreInitDesc & desc)
SemaphoreHandle SynchronizationManager::CreateSemaphore() {
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

FenceHandle SynchronizationManager::CreateFence() {
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

Semaphore SynchronizationManager::GetSemaphore(const SemaphoreHandle &h) {
  return mSemaphores.get(h);
}

Fence SynchronizationManager::GetFence(const FenceHandle &h) {
  return mFences.get(h);
}

void SynchronizationManager::Destroy() {
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

} // namespace unknown::renderer::vulkan