#pragma once
#include <memory>
#include <vector>

#include "vulkan_renderer/types.hpp"
#include "vulkan_renderer/defines.hpp"
#include "vulkan_renderer/renderObject.hpp"

namespace unknown::renderer::vulkan
{

class Device;
class ResourceManager;
class PipelineManager;
class DescriptorSetAllocator;
class DescriptorSetLayoutManager;

template<u32>
class CommandBufferManager;

template<u32>
class SynchronizationManager;

struct VulkanContext
{
  std::shared_ptr<Device> device = nullptr;
  std::shared_ptr<ResourceManager> resourceManager = nullptr;
  std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> commandBufferManager = nullptr;
  std::shared_ptr<SynchronizationManager<FRAME_OVERLAP>> synchronizationManager = nullptr;
  std::shared_ptr<PipelineManager> pipelineManager = nullptr;
  std::shared_ptr<DescriptorSetAllocator> globalDescriptorSetAllocator = nullptr;

  Viewport viewport;
  
  std::vector<RenderObject> renderObjects;
};

}