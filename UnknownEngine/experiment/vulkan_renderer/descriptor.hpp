#pragma once

#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/resource.hpp"

#include "utils/container.hpp"
#include "utils/hash.hpp"
#include "vulkan_renderer/sstring.hpp"

#include <queue>
#include <vector>

namespace unknown::renderer::vulkan {
// enum class DescriptorType
// {
//   Uniform,
//   Image,
//   ShaderStorage,
// };

struct DescriptorSetAllocDesc {
  std::vector<VkDescriptorSetLayout> layouts;
  void *pNext = nullptr;
};

struct DescriptorPoolSize {
  VkDescriptorType type;
  u32 averageCount;
};

struct DescriptorSetAllocatorInitDesc {
#define ADD_DS_TYPE(FUNC_NAME, TYPE_NAME)                                      \
  DescriptorSetAllocatorInitDesc &FUNC_NAME(u32 count) {                       \
    sizes.push_back(                                                           \
        DescriptorPoolSize{.type = TYPE_NAME, .averageCount = count});         \
    return *this;                                                              \
  }

  VkDevice device;
  ADD_DS_TYPE(storage_image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
  ADD_DS_TYPE(storage_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
  ADD_DS_TYPE(uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)

  std::vector<DescriptorPoolSize> &get() { return sizes; }

private:
  std::vector<DescriptorPoolSize> sizes;
};

struct DescriptorSlot {
  u32 binding;
  VkDescriptorType type;
};

struct DescriptorSetLayoutKey {
  std::vector<DescriptorSlot> slots;
  u32 descriptorCount = 0u;
  VkShaderStageFlags stageFlags;
  u64 hashCache = 0u;

  bool operator==(const DescriptorSetLayoutKey &rhs) const {
    if (descriptorCount != rhs.descriptorCount)
      return false;

    bool equal = true;

    for (u64 i = 0; i < descriptorCount; i++) {
      equal &= slots[i].binding == rhs.slots[i].binding;
      equal &= slots[i].type == rhs.slots[i].type;
    }
    equal &= stageFlags == rhs.stageFlags;
    return equal;
  }

  void updateKey() { hashCache = hash(); }

  u64 hash() const {
    if (descriptorCount == 0)
      return 0;
    u64 h = unknown::make_combined_hash(descriptorCount, stageFlags);
    for (u64 i = 0; i < slots.size(); i++) {
      unknown::hash_combine(h, slots[i].binding);
      unknown::hash_combine(h, slots[i].type);
    }

    return h;
  }
};

struct DescriptorSetLayout {
  VkDescriptorSetLayout data;
};

struct DescriptorSetLayouts {
  std::vector<DescriptorSetLayoutKey> layouts;
  u64 hashCache = 0;

  bool operator==(const DescriptorSetLayouts &rhs) const {
    if (layouts.size() != rhs.layouts.size())
      return false;

    bool equal = true;

    for (u64 i = 0; i < layouts.size(); i++) {
      equal &= layouts[i].hash() == rhs.layouts[i].hash();
    }

    return equal;
  }

  void updateKey() { hashCache = hash(); }

  u64 hash() const {
    u64 h = 0;
    u64 size = layouts.size();
    if (size == 0)
      return h;

    unknown::hash_combine(h, size);
    for (auto &l : layouts) {
      unknown::hash_combine(h, l.hash());
    }
    return h;
  }
};

struct PushConstants {
  std::vector<VkPushConstantRange> ranges;
  u64 hashCache = 0;

  bool operator==(const PushConstants &rhs) const {
    if (ranges.size() != rhs.ranges.size())
      return false;

    bool equal = true;

    for (u64 i = 0; i < ranges.size(); i++) {
      equal &= ranges[i].offset == rhs.ranges[i].offset;
      equal &= ranges[i].size == rhs.ranges[i].size;
      equal &= ranges[i].stageFlags == rhs.ranges[i].stageFlags;
    }
    return equal;
  }

  void updateKey() { hashCache = hash(); }

  u64 hash() const {
    u64 h = 0;
    u64 size = ranges.size();
    if (size == 0)
      return h;
    unknown::hash_combine(h, size);
    for (u64 i = 0; i < size; i++) {
      unknown::hash_combine(h, ranges[i].offset);
      unknown::hash_combine(h, ranges[i].size);
      unknown::hash_combine(h, ranges[i].stageFlags);
    }

    return h;
  }
};

struct PushConstantsBuilder {
private:
#define ADD_FLAG(FUNC_NAME, FLAG)                                              \
  PushConstantsBuilder &FUNC_NAME() {                                          \
    stageFlags |= FLAG;                                                        \
    return *this;                                                              \
  }

  ADD_FLAG(stage_compute, VK_SHADER_STAGE_COMPUTE_BIT)

  PushConstantsBuilder &add(u32 size) {
    VkPushConstantRange pushRange{};
    pushRange.offset = offset;
    pushRange.size = size;
    pushRange.stageFlags = stageFlags;

    offset += size;
    stageFlags = {};
    ranges.push_back(pushRange);
    return *this;
  }

  PushConstantsBuilder &clear() {
    ranges.clear();
    stageFlags = {};
    offset = 0u;
    return *this;
  }

private:
  PushConstants build() { return PushConstants{.ranges = ranges}; }

  std::vector<VkPushConstantRange> ranges;
  VkShaderStageFlags stageFlags = {};
  u32 offset = 0u;
};

} // namespace unknown::renderer::vulkan

namespace std {
template <> struct hash<unknown::renderer::vulkan::DescriptorSetLayoutKey> {
  std::size_t operator()(
      const unknown::renderer::vulkan::DescriptorSetLayoutKey &key) const {
    return std::size_t(key.hash());
  }
};

template <> struct hash<unknown::renderer::vulkan::PushConstants> {
  std::size_t
  operator()(const unknown::renderer::vulkan::PushConstants &key) const {
    return std::size_t(key.hash());
  }
};

template <> struct hash<unknown::renderer::vulkan::DescriptorSetLayouts> {
  std::size_t
  operator()(const unknown::renderer::vulkan::DescriptorSetLayouts &key) const {
    return std::size_t(key.hash());
  }
};

} // namespace std

namespace unknown::renderer::vulkan {

struct DescriptorSetLayoutBuilder {
  friend class DescriptorSetLayoutManager;
#define ADD_BINDING(FUNC_NAME, TYPE_NAME)                                      \
  DescriptorSetLayoutBuilder &FUNC_NAME(u32 bindingPoint, u32 count = 1u) {    \
    VkDescriptorSetLayoutBinding binding{};                                    \
    binding.binding = bindingPoint;                                            \
    binding.descriptorCount = count;                                           \
    binding.descriptorType = TYPE_NAME;                                        \
    bindings.push_back(binding);                                               \
    return *this;                                                              \
  }

#define ADD_STAGE(FUNC_NAME, TYPE_NAME)                                        \
  DescriptorSetLayoutBuilder &FUNC_NAME() {                                    \
    stages |= TYPE_NAME;                                                       \
    return *this;                                                              \
  }

  ADD_BINDING(storage_image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
  ADD_BINDING(combined_image_sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
  ADD_BINDING(unifrom_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
  ADD_BINDING(storage_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
  ADD_BINDING(unifrom_buffer_dynamic, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
  ADD_BINDING(storage_buffer_dynamic, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)

  ADD_STAGE(stage_fragment, VK_SHADER_STAGE_FRAGMENT_BIT)
  ADD_STAGE(stage_vertex, VK_SHADER_STAGE_VERTEX_BIT)
  ADD_STAGE(stage_compute, VK_SHADER_STAGE_COMPUTE_BIT)
  ADD_STAGE(stage_task, VK_SHADER_STAGE_TASK_BIT_EXT)
  ADD_STAGE(stage_mesh, VK_SHADER_STAGE_MESH_BIT_EXT)

  void clear_bindings() { bindings.clear(); }
  void clear_stages() { stages = 0; }

private:
  DescriptorSetLayoutKey preBuild() {
    u32 size = bindings.size();
    descInfo.stageFlags = stages;
    descInfo.descriptorCount = size;
    descInfo.slots.resize(size);

    for (u32 i = 0; i < size; i++) {
      descInfo.slots[i].binding = bindings[i].binding;
      descInfo.slots[i].type = bindings[i].descriptorType;
    }

    return descInfo;
  }

  DescriptorSetLayout build(VkDevice device, void *pNext = nullptr,
                            VkDescriptorSetLayoutCreateFlags flags = 0) {
    assert(!bindings.empty());

    DescriptorSetLayout layout;

    for (auto &b : bindings) {
      b.stageFlags = stages;
    }

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext = pNext;
    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    auto res = vkCreateDescriptorSetLayout(device, &info, nullptr, &set);
    //INFO_LOG("create descriptor {}",res==VK_SUCCESS?"TRUE":"FALSE");
    VK_CHECK(res);

    layout.data = set;
    // layout.info = descInfo;

    return layout;
  }

private:
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  VkShaderStageFlags stages = {};
  DescriptorSetLayoutKey descInfo;
};

// DECL_CONTAINER_HANDLE(DescriptorSetLayoutHandle, NoRecycle, VectorContainer,
//                       DescriptorSetLayout)

struct DescriptorSetLayoutManagerInitDesc {
  VkDevice device;
};

class DescriptorSetLayoutManager {
public:
  void Init(DescriptorSetLayoutManagerInitDesc &desc) { mDevice = desc.device; }

  DescriptorSetLayoutKey Create(DescriptorSetLayoutBuilder &builder) {
    DescriptorSetLayout layout;

    DescriptorSetLayoutKey k = builder.preBuild();

    if (auto it = _mLayouts.find(k); it != _mLayouts.end()) {
      return it->first;
    }

    layout = builder.build(mDevice);
    auto bRes = _mLayouts.insert({k, layout}).second;
    assert(bRes);
    return k;
  }

  DescriptorSetLayout Get(DescriptorSetLayoutKey key) {
    DescriptorSetLayout layout;
    if (auto it = _mLayouts.find(key); it != _mLayouts.end())
      layout = it->second;
    return layout;
  }

  void Destroy() {
    for (auto &l : _mLayouts) {
      vkDestroyDescriptorSetLayout(mDevice, l.second.data, nullptr);
    }

    _mLayouts.clear();
  }

private:
  VkDevice mDevice;
  std::unordered_map<DescriptorSetLayoutKey, DescriptorSetLayout> _mLayouts;
  // VectorContainer<DescriptorSetLayoutHandle, DescriptorSetLayout> mLayouts;
};

class DescriptorSetBinder {
public:
  explicit DescriptorSetBinder(VkDevice device, VkDescriptorSet set)
      : _device(device), _set(set) 
  {
    imageInfos.reserve(8);
    bufferInfos.reserve(8);
    writes.reserve(8);
  }
  void clear() {
    imageInfos.clear();
    bufferInfos.clear();
    writes.clear();
  }

  void buffer(u32 binding, Buffer buffer, u32 size, u32 offset,
              VkDescriptorType type) {

    //VkDescriptorBufferInfo &info =
    bufferInfos.emplace_back(VkDescriptorBufferInfo{.buffer = buffer.buffer, .offset = offset, .range = size});

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = binding;
    write.dstSet = _set;
    write.descriptorType = type;
    write.descriptorCount = 1;
    //write.pBufferInfo = &info;

    writes.push_back(write);
  }

  void write() {
    u32 writeCount = writes.size();

    for(u32 i = 0u; i < writeCount; i++)
      writes[i].pBufferInfo = &bufferInfos[i];

    vkUpdateDescriptorSets(_device, writes.size(), writes.data(), 0, nullptr);
  }

private:
  std::vector<VkDescriptorImageInfo> imageInfos;
  std::vector<VkDescriptorBufferInfo> bufferInfos;
  std::vector<VkWriteDescriptorSet> writes;

  VkDevice _device;
  VkDescriptorSet _set;
};

class DescriptorSetAllocator {
public:
  void Init(const DescriptorSetAllocatorInitDesc &desc);
  // void Clear();
  void Reset();
  std::vector<VkDescriptorSet> Allocate(const DescriptorSetAllocDesc &desc);
  void Destroy();

private:
  VkDescriptorPool createPool();
  VkDescriptorPool getPool();

private:
  DescriptorSetAllocatorInitDesc mDesc;
  u32 mMaxCount = 256u;
  const u32 mLimit = 2048u;
  u32 mGrowthRate = 2u;

  std::vector<VkDescriptorPool> mFullPools;
  std::vector<VkDescriptorPool> mReadyPools;
};
} // namespace unknown::renderer::vulkan