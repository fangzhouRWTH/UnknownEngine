#include "vulkan_renderer/descriptor.hpp"
#include "descriptor.hpp"

namespace unknown::renderer::vulkan {
void DescriptorSetAllocator::Init(const DescriptorSetAllocatorInitDesc &desc) {
  mDesc = desc;
  auto newPool = createPool();
  mReadyPools.push_back(newPool);
}

VkDescriptorPool DescriptorSetAllocator::createPool() {

  std::vector<VkDescriptorPoolSize> sizes;
  assert(mDesc.get().size() != 0);
  for (auto s : mDesc.get()) {
    auto pSize = VkDescriptorPoolSize{
        .type = s.type, .descriptorCount = s.averageCount * mMaxCount};
    sizes.push_back(pSize);
  }

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = 0;
  poolInfo.maxSets = mMaxCount;
  poolInfo.poolSizeCount = sizes.size();
  poolInfo.pPoolSizes = sizes.data();

  VkDescriptorPool newPool;
  VK_CHECK(vkCreateDescriptorPool(mDesc.device, &poolInfo, nullptr, &newPool));
  u32 nextMax = mMaxCount * mGrowthRate;
  mMaxCount = nextMax > mLimit ? mLimit : nextMax;
  return newPool;
}

VkDescriptorPool DescriptorSetAllocator::getPool() {
  VkDescriptorPool _pool;
  if (mReadyPools.size() != 0) {
    _pool = mReadyPools.back();
    mReadyPools.pop_back();
  } else {
    _pool = createPool();
  }
  return _pool;
}

void DescriptorSetAllocator::Reset() {
  for (auto p : mReadyPools) {
    vkResetDescriptorPool(mDesc.device, p, 0);
  }
  for (auto p : mFullPools) {
    vkResetDescriptorPool(mDesc.device, p, 0);
    mReadyPools.push_back(p);
  }
  mFullPools.clear();
}

void DescriptorSetAllocator::Destroy() {
  for (auto p : mReadyPools) {
    vkDestroyDescriptorPool(mDesc.device, p, nullptr);
  }
  mReadyPools.clear();
  for (auto p : mFullPools) {
    vkDestroyDescriptorPool(mDesc.device, p, nullptr);
  }
  mFullPools.clear();
}

std::vector<VkDescriptorSet>
DescriptorSetAllocator::Allocate(const DescriptorSetAllocDesc &desc) {
  VkDescriptorPool _pool = getPool();

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.pNext = desc.pNext;
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _pool;
  allocInfo.descriptorSetCount = desc.layouts.size();
  allocInfo.pSetLayouts = desc.layouts.data();

  std::vector<VkDescriptorSet> sets;
  sets.resize(desc.layouts.size());
  assert(sets.size() == desc.layouts.size());
  VkResult result =
      vkAllocateDescriptorSets(mDesc.device, &allocInfo, sets.data());

  if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
      result == VK_ERROR_FRAGMENTED_POOL) {
    mFullPools.push_back(_pool);
    _pool = getPool();
    allocInfo.descriptorPool = _pool;
    result = vkAllocateDescriptorSets(mDesc.device, &allocInfo, sets.data());

    if (result != VK_SUCCESS)
      assert(false);
  }

  mReadyPools.push_back(_pool);

  return sets;
}

} // namespace unknown::renderer::vulkan