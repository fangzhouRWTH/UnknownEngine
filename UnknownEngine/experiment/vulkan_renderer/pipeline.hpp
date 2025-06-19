#pragma once
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/descriptor.hpp"
#include "vulkan_renderer/shader.hpp"

#include "utils/hash.hpp"

#include <unordered_map>
#include <vector>

namespace unknown::renderer::vulkan {
struct PipelineLayoutDesc {
  DescriptorSetLayouts setLayouts;
  PushConstants pushConstants;

  u64 hashCache = 0;

  void updateKey() { hashCache = hash(); }

  bool operator==(const PipelineLayoutDesc &rhs) const {
    return (setLayouts == rhs.setLayouts && pushConstants == rhs.pushConstants);
  }

  u64 hash() const {
    u64 hash = 0;

    auto hset = setLayouts.hash();
    auto hpush = pushConstants.hash();

    if (hset != 0)
      hash_combine(hash, setLayouts.hash());

    if (hpush != 0)
      hash_combine(hash, pushConstants.hash());

    return hash;
  }
};

struct PipelineManagerInitDesc {
  VkDevice device;
};

enum class PipelineType {
  Normal,
  Mesh,
  Compute,
};

struct PipelineDesc {
  PipelineLayoutDesc layoutDesc;
  PipelineType type;

  std::unordered_map<ShaderType, ShaderDesc> shaders;

  bool operator==(const PipelineDesc & rhs) const
  {
    bool equal = layoutDesc == rhs.layoutDesc && type == rhs.type;
    equal &= shaders ==rhs.shaders;
    return equal;
  }

  u64 hash() const {
    u64 h = layoutDesc.hash();
    hash_combine(h, u32(type));

    {
      for (u64 i = 0; i < static_cast<u64>(ShaderType::EnumMax); i++) {
        auto it = shaders.find(static_cast<ShaderType>(i));
        if (it != shaders.end()) {
          h = make_combined_hash(h, it->second.hash());
        }
      }
    }
    return h;
  }
};

struct PipelineLayout {
  VkPipelineLayout data;
};

struct Pipeline {
  VkPipeline data;
};
} // namespace unknown::renderer::vulkan

namespace std {
template <> struct hash<unknown::renderer::vulkan::PipelineLayoutDesc> {
  std::size_t
  operator()(const unknown::renderer::vulkan::PipelineLayoutDesc &key) const {
    return std::size_t(key.hash());
  }
};

template <> struct hash<unknown::renderer::vulkan::PipelineDesc> {
  std::size_t
  operator()(const unknown::renderer::vulkan::PipelineDesc &key) const {
    return std::size_t(key.hash());
  }
};
} // namespace std

namespace unknown::renderer::vulkan {

class PipelineManager {
public:
  void Init(const PipelineManagerInitDesc &desc);
  void Destroy();

  PipelineLayout GetPipelineLayout(const PipelineLayoutDesc &desc);
  Pipeline GetPipeline(const PipelineDesc &desc);

  bool CreateShader(const ShaderDesc & desc);
  void ClearShaderCache();

  DescriptorSetLayoutKey CreateDescriptorSetLayout(DescriptorSetLayoutBuilder builder);
  DescriptorSetLayout GetDescriptorSetLayout(DescriptorSetLayoutKey key);

private:
  VkDevice mDevice;
  ShaderManager mShaderManager;
  DescriptorSetLayoutManager mDescriptorSetLayoutManager;

  PipelineLayout createPipelineLayout(const PipelineLayoutDesc &desc);
  Pipeline createPipeline(const PipelineDesc &desc);

  VkPipeline createGraphicPipeline(const PipelineDesc &desc);
  VkPipeline createComputePipeline(const PipelineDesc &desc);
  VkPipeline createMeshPipeline(const PipelineDesc &desc);

  std::unordered_map<PipelineLayoutDesc, PipelineLayout> mLayouts;
  std::unordered_map<PipelineDesc, Pipeline> mPipelines;
};
} // namespace unknown::renderer::vulkan