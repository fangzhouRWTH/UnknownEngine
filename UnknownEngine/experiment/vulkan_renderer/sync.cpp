//#include "vulkan_renderer/sync.hpp"

namespace unknown::renderer::vulkan {
// VkBufferMemoryBarrier SynchronizationPool::CreateBufferMemoryBarrier(
//     VkAccessFlags sFlags, VkAccessFlags dFlags, VkBuffer buffer, u32 queueSrc,
//     u32 queueDst) {
//   VkBufferMemoryBarrier barrier{};
//   barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
//   barrier.srcAccessMask = sFlags;
//   barrier.dstAccessMask = dFlags;
//   barrier.srcQueueFamilyIndex = queueSrc;
//   barrier.dstQueueFamilyIndex = queueDst;
//   barrier.buffer = buffer;
//   barrier.offset = 0;
//   barrier.size = VK_WHOLE_SIZE;
//   return barrier;
// }

// void SynchronizationPool::Init(const SyncInitDesc &desc) {
//   mDevice = desc.device;
// };
// // Semaphore Semaphore::Create(const SemaphoreInitDesc & desc)
// SemaphoreHandle SynchronizationPool::createSemaphore() {
//   VkSemaphoreCreateInfo info = {};
//   info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//   info.pNext = nullptr;
//   info.flags = 0;

//   Semaphore sema;
//   VK_CHECK(vkCreateSemaphore(mDevice, &info, nullptr, &sema.data));

//   auto h = mSemaphores.push(sema);
//   mFreeSemaphores.push(h);
//   assert(h.isValid());
//   return h;
// }

// FenceHandle SynchronizationPool::createFence() {
//   VkFenceCreateInfo info = {};
//   info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//   info.pNext = nullptr;
//   info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

//   Fence fence;
//   VK_CHECK(vkCreateFence(mDevice, &info, nullptr, &fence.data));

//   auto h = mFences.push(fence);
//   mFreeFences.push(h);
//   assert(h.isValid());
//   return h;
// }

// SemaphoreHandle SynchronizationPool::AcquireSemaphore() {
//   if (mFreeSemaphores.empty())
//     return createSemaphore();

//   auto h = mFreeSemaphores.front();
//   mFreeSemaphores.pop();
//   mUsedSemaphores.push_back(h);
//   return h;
// }

// Semaphore SynchronizationPool::GetSemaphore(const SemaphoreHandle &h) {
//   return mSemaphores.get(h);
// }

// FenceHandle SynchronizationPool::AcquireFence() {
//   if (mFreeFences.empty())
//     return createFence();

//   auto h = mFreeFences.front();
//   mFreeFences.pop();
//   mUsedFences.push_back(h);
//   return h;
// }

// Fence SynchronizationPool::GetFence(const FenceHandle &h) {
//   return mFences.get(h);
// }

// void SynchronizationPool::Reset() {
//   for (auto s : mUsedSemaphores) {
//     mFreeSemaphores.push(s);
//   }
//   mUsedSemaphores.clear();

//   for (auto s : mUsedFences) {
//     mFreeFences.push(s);
//   }
//   mUsedFences.clear();
// }

// void SynchronizationPool::Destroy() {
//   auto &semas = mSemaphores.getContainer();
//   for (auto &s : semas) {
//     if (s.first)
//       ;
//     vkDestroySemaphore(mDevice, s.second.data, nullptr);
//   }

//   auto &fens = mFences.getContainer();
//   for (auto &f : fens) {
//     if (f.first)
//       ;
//     vkDestroyFence(mDevice, f.second.data, nullptr);
//   }

//   mSemaphores.reset();
//   mFences.reset();
// }

} // namespace unknown::renderer::vulkan