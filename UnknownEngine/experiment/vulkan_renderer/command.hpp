#pragma once
#include "utils/container.hpp"
#include "vulkan_renderer/common.hpp"
#include <array>
#include <queue>
#include <vector>

namespace unknown::renderer::vulkan {

struct CommandBuffer {
  VkCommandBuffer buffer;
  QueueType queueType;
};

class CommandBufferPool {
public:
  CommandBufferPool() {}
  ~CommandBufferPool() {}

  void Init(const VkDevice &device, const u32 &queueIndex, const QueueType & type);
  void Reset();
  void Destroy();

  CommandBuffer GetCommandBuffer();

private:
  VkDevice mDevice;
  void create();
  bool bInit = false;
  u32 mQueueFamilyIndex;
  QueueType mQueueType;
  VkCommandPool mPool;
  // std::vector<CommandBuffer> mBuffers;

  std::queue<CommandBuffer> mFreeCommandBuffers;
  std::vector<CommandBuffer> mUsedCommandBuffers;
};

class CommandPoolSet {
public:
  void AddQueueCommandPool(const QueueType &queue, const VkDevice &device,
                           const u32 &familyIndex) {
    auto it = mPools.find(queue);
    if (it != mPools.end())
      return;

    std::shared_ptr<CommandBufferPool> pool =
        std::make_shared<CommandBufferPool>();
    pool->Init(device, familyIndex, queue);
    mPools.insert({queue, pool});
  }

  std::shared_ptr<CommandBufferPool> GetQueuePool(const QueueType &queueType) {
    auto it = mPools.find(queueType);
    if (it == mPools.end())
      return nullptr;

    return it->second;
  }

  void Reset(QueueType queue) {
    auto it = mPools.find(queue);
    if (it == mPools.end())
      return;

    it->second->Reset();
  }

  void ResetAll()
  {
    for(auto & p : mPools)
    {
      p.second->Reset();
    }
  }

  void Destroy(const VkDevice &device) {
    for (auto p : mPools) {
      p.second->Destroy();
    }
    mPools.clear();
  }

private:
  std::unordered_map<QueueType, std::shared_ptr<CommandBufferPool>> mPools;
};

template <u32 FRAMES_COUNT> class CommandBufferManager {
public:
  void Init(const VulkanCoreData &vkData) {
    mVkData = vkData;
    for (auto &s : mPoolSets) {
      s.AddQueueCommandPool(QueueType::Graphic, vkData.device,
                            vkData.graphicsQueueFamily);

      if (vkData.useComputeQueue && vkData.hasComputeQueue)
        s.AddQueueCommandPool(QueueType::Compute, vkData.device,
                              vkData.computeQueueFamily);
      if (vkData.useTransferQueue && vkData.hasTransferQueue)
        s.AddQueueCommandPool(QueueType::Transfer, vkData.device,
                              vkData.transferQueueFamily);
    }
  }

  void Destroy() {
    for (auto &s : mPoolSets) {
      s.Destroy(mVkData.device);
    }
  }

  void Advance() { mCurrentIndex = (mCurrentIndex + 1u) % mFramesCount; }

  void ResetCurrentPoolSetByQueue(QueueType queue) {
    auto &poolset = mPoolSets[mCurrentIndex];
    poolset.Reset(queue);
  }

  void ResetCurrentPoolSets()
  {
    auto &poolset = mPoolSets[mCurrentIndex];
    poolset.ResetAll();
  }

  CommandBuffer BeginCommandBuffer(QueueType queue) {
    // Todo
    auto &poolSet = mPoolSets[mCurrentIndex];
    auto queuePool = poolSet.GetQueuePool(queue);
    assert(queuePool != nullptr);
    if (queuePool == nullptr)
      return CommandBuffer();

    auto cb = queuePool->GetCommandBuffer();

    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cb.buffer, &info));

    return cb;
  }

  void EndCommandBuffer(const CommandBuffer & cmd)
  {
    VK_CHECK(vkEndCommandBuffer(cmd.buffer));
  }

private:
  CommandPoolSet &getCurrentPoolSet() { return mPoolSets[mCurrentIndex]; }

  VulkanCoreData mVkData;
  const u32 mFramesCount = FRAMES_COUNT;
  u32 mCurrentIndex = 0u;
  std::array<CommandPoolSet, FRAMES_COUNT> mPoolSets;
};
} // namespace unknown::renderer::vulkan