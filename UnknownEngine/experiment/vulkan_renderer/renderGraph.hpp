#pragma once

#include "platform/type.hpp"
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/resource.hpp"
#include "vulkan_renderer/types.hpp"
#include "vulkan_renderer/vulkanContext.hpp"
#include <memory>
#include <vector>

namespace unknown::renderer::vulkan {
enum class Access { ReadOnly, WriteOnly, ReadWrite };

struct Attachment {
  ImageHandle handle;
  // test
  Access access = Access::ReadWrite;
};

class PassDependency
{

};

class IRenderPass {
public:
  virtual void Prepare(VulkanContext ctx) {};
  virtual void Execute(VulkanContext ctx) = 0;
  virtual void End(VulkanContext ctx) {};
private:
};

class ResourcePass : public IRenderPass {
public:
  virtual void Execute(VulkanContext ctx) override
  {
    //ctx.resourceManager->FlushFrameResource()
  }
private:
};

class RenderGraph
{
public:
  void Execute()
  {
    
  }

  std::vector<std::shared_ptr<IRenderPass>> passes;

private:

};
} // namespace unknown::renderer::vulkan