#pragma once
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/descriptor.hpp"
#include "vulkan_renderer/device.hpp"
#include "vulkan_renderer/resource.hpp"
#include "vulkan_renderer/swapchain.hpp"
#include "vulkan_renderer/sync.hpp"
#include "vulkan_renderer/vulkanContext.hpp"


#include <array>

namespace unknown::renderer::vulkan {
struct FrameInitDesc {
  uint32_t queueFamilyIndex;
  VkQueue graphicsQueue;
  VkDevice device;

  Semaphore resourceSemaphore;
  SemaphoreHandle resourceSemaphoreHandle;
  Semaphore swapchainSemaphore;
  SemaphoreHandle swapchainSemaphoreHandle;
  Semaphore renderSemaphore;
  SemaphoreHandle renderSemaphoreHandle;
  Fence renderFence;
  FenceHandle renderFenceHandle;

  ImageHandle colorTarget;
  ImageHandle depthTarget;

  DescriptorSetAllocatorInitDesc descriptorAllocatorDesc;
};

class Swapchain;

struct Frame {
  friend class Swapchain;
  void Init(FrameInitDesc desc);
  void ResetDescriptorSet();
  void Destroy();

  void WaitFence();
  void ResetFence();
  void ResetCommand();
  void ReleaseFrameResource(VulkanContext ctx);

  void Begin(VulkanContext ctx);
  void Render(VulkanContext ctx, VkImage scImage);
  void End(VulkanContext ctx, VkImage scImage);

private:
  bool bInit = false;

  VkDevice mDevice;
  VkQueue mGraphicsQueue;

  Semaphore mResourceSemaphore;
  SemaphoreHandle mResourceSemaphoreHandle;

  Semaphore mSwapchainSemaphore;
  SemaphoreHandle mSwapchainSemaphoreHandle;
  Semaphore mRenderSemaphore;
  SemaphoreHandle mRenderSemaphoreHandle;
  Fence mRenderFence;
  FenceHandle mRenderFenceHandle;

  //CommandManager mCmdPool;
  //CommandBuffer mCmdBuffer;

  DescriptorSetAllocator mDescriptorSetAllocator;

  ImageHandle mColorTarget;
  ImageHandle mDepthTarget;

  //std::vector<ResourceHandle>  mFrameReleaseResources;
};

template <uint32_t FRAME_COUNT> class Frames {
public:
  void Init(VulkanContext ctx);
  std::array<Frame, FRAME_COUNT> &GetFrames() { return mFrames; }
  Frame &GetCurrentFrame() { return mFrames[mFrameIndex]; }
  uint32_t GetCurrentFrameIndex() { return mFrameIndex; }
  void Advance() {
    ++mFrameIndex;
    mFrameIndex %= mFrameCount;
  }
  void Destroy() {
    for (auto &f : mFrames) {
      f.Destroy();
    }
    mSwapchain.Destroy();
  }

  void Wait(VulkanContext ctx) {
    auto &frame = GetCurrentFrame();
    frame.WaitFence();
    frame.ResetDescriptorSet();
    ctx.commandBufferManager->ResetCurrentPoolSets();
    mSwapchain.UpdateNextIndex(frame);
  }

  bool NeedResize() { return mSwapchain.NeedResize(); }

  void Reset(VulkanContext ctx) {
    auto &frame = GetCurrentFrame();
    frame.ResetFence();
    frame.ResetCommand();
    frame.ReleaseFrameResource(ctx);
  }

  void Begin(VulkanContext ctx) {
    //auto &frame = GetCurrentFrame();
    //frame.Begin(ctx);
  }

  void Render(VulkanContext ctx) {
    auto &frame = GetCurrentFrame();
    frame.Render(ctx,mSwapchain.GetCurrentImage());
  }

  void End(VulkanContext ctx) {
    //auto &frame = GetCurrentFrame();
    //frame.End(ctx, mSwapchain.GetCurrentImage());
  }

  void Present(VulkanContext ctx) {
    auto &frame = GetCurrentFrame();
    mSwapchain.Present(frame);
  }

private:
  std::array<Frame, FRAME_COUNT> mFrames;
  uint32_t mFrameIndex = 0;
  const uint32_t mFrameCount = FRAME_COUNT;
  Swapchain mSwapchain;
};

template <uint32_t FRAME_COUNT>
inline void Frames<FRAME_COUNT>::Init(VulkanContext ctx) {
  SwapchainInitInfo sInfo;
  sInfo.device = ctx.device->GetDevice();
  sInfo.graphicsQueue = ctx.device->GetGraphicsQueue();
  sInfo.physicalDevice = ctx.device->GetGPU();
  sInfo.width = ctx.viewport.width;
  sInfo.height = ctx.viewport.height;
  sInfo.surface = ctx.device->GetSurface();
  mSwapchain.Init(sInfo);

  FrameInitDesc fDesc;
  fDesc.device = ctx.device->GetDevice();
  fDesc.graphicsQueue = ctx.device->GetGraphicsQueue();
  fDesc.queueFamilyIndex = ctx.device->GetGraphicsQueueIndex();

  DescriptorSetAllocatorInitDesc frameDescAllocatorDesc;
  frameDescAllocatorDesc.device = ctx.device->GetDevice();
  frameDescAllocatorDesc.storage_image(3u).storage_buffer(3u).uniform_buffer(
      3u);
  fDesc.descriptorAllocatorDesc = frameDescAllocatorDesc;

  ImageDesc colorTargetDesc;
  ImageDesc depthTargetDesc;

  colorTargetDesc.imageFormat.rgba_16_sf().color_aspect().size(
      ctx.viewport.width, ctx.viewport.height);
  colorTargetDesc.imageUsage.transfer_src().shader_storage().color_attachment();
  colorTargetDesc.memoryUsage.prefer_device();
  //colorTargetDesc.memoryProperty.

  depthTargetDesc.imageFormat.d_32_sf().depth_aspect().size(
      ctx.viewport.width, ctx.viewport.height);
  depthTargetDesc.imageUsage.depth_attachment();
  depthTargetDesc.memoryUsage.prefer_device();

  for (auto &f : mFrames) {
    fDesc.renderFenceHandle = ctx.synchronizationManager->CreateFence();
    fDesc.renderFence =
        ctx.synchronizationManager->GetFence(fDesc.renderFenceHandle);

    fDesc.resourceSemaphoreHandle = ctx.synchronizationManager->CreateSemaphore();
    fDesc.resourceSemaphore = ctx.synchronizationManager->GetSemaphore(fDesc.resourceSemaphoreHandle);    

    fDesc.swapchainSemaphoreHandle =
        ctx.synchronizationManager->CreateSemaphore();
    fDesc.swapchainSemaphore = ctx.synchronizationManager->GetSemaphore(
        fDesc.swapchainSemaphoreHandle);

    fDesc.renderSemaphoreHandle = ctx.synchronizationManager->CreateSemaphore();
    fDesc.renderSemaphore =
        ctx.synchronizationManager->GetSemaphore(fDesc.renderSemaphoreHandle);

    fDesc.colorTarget = ctx.resourceManager->CreateImage(colorTargetDesc);
    fDesc.depthTarget = ctx.resourceManager->CreateImage(depthTargetDesc);

    f.Init(fDesc);
  }
}
} // namespace unknown::renderer::vulkan