#include "vulkan_renderer/frame.hpp"
#include "frame.hpp"
#include "vulkan_renderer/defines.hpp"
#include "vulkan_renderer/image.hpp"
#include "vulkan_renderer/resource.hpp"

namespace unknown::renderer::vulkan {
void Frame::Init(FrameInitDesc desc) {
  if (bInit)
    return;

  mDevice = desc.device;
  mGraphicsQueue = desc.graphicsQueue;
  // CommandManagerInitDesc cmddesc;
  // cmddesc.device = desc.device;
  // cmddesc.queueFamilyIndex = desc.queueFamilyIndex;
  // mCmdPool.Init(cmddesc);

  // mCmdBuffer = mCmdPool.Create();

  mResourceSemaphore = desc.resourceSemaphore;
  mResourceSemaphoreHandle = desc.resourceSemaphoreHandle;
  mSwapchainSemaphore = desc.swapchainSemaphore;
  mSwapchainSemaphoreHandle = desc.swapchainSemaphoreHandle;
  mRenderSemaphore = desc.renderSemaphore;
  mRenderSemaphoreHandle = desc.renderSemaphoreHandle;
  mRenderFence = desc.renderFence;
  mRenderFenceHandle = desc.renderFenceHandle;

  mDescriptorSetAllocator.Init(desc.descriptorAllocatorDesc);

  mColorTarget = desc.colorTarget;
  mDepthTarget = desc.depthTarget;
}

void Frame::Destroy() {
  mDescriptorSetAllocator.Destroy();
  // mCmdPool.Destroy();
}

void Frame::WaitFence() {
  VK_CHECK(vkWaitForFences(mDevice, 1, &mRenderFence.data, true, 1000000000));
}

void Frame::ResetFence() {
  VK_CHECK(vkResetFences(mDevice, 1, &mRenderFence.data));
}

void Frame::ResetCommand() {
  // VK_CHECK(vkResetCommandBuffer(mCmdBuffer.buffer, 0));
}

void Frame::ReleaseFrameResource(VulkanContext ctx) {}

void Frame::Begin(VulkanContext ctx) {}

void Frame::Render(VulkanContext ctx, VkImage scImage) {
  VkCommandBufferBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;
  info.pInheritanceInfo = nullptr;
  info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  ctx.resourceManager->FlushFrameResource(
      ctx.commandBufferManager, mSwapchainSemaphore, mResourceSemaphore);

  // VK_CHECK(vkBeginCommandBuffer(mCmdBuffer.buffer, &info));

  auto cb = ctx.commandBufferManager->BeginCommandBuffer(QueueType::Graphic);

  {
    auto targetImage = ctx.resourceManager->GetImage(mColorTarget);
    ImageTransitionDesc colordesc;
    colordesc.cmd = cb.buffer; // mCmdBuffer.buffer;
    colordesc.image = targetImage.image;
    colordesc.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colordesc.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    TransitionImageResource(colordesc);

    auto targetDepth = ctx.resourceManager->GetImage(mDepthTarget);
    ImageTransitionDesc depthdesc;
    depthdesc.cmd = cb.buffer; // mCmdBuffer.buffer;
    depthdesc.image = targetDepth.image;
    depthdesc.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthdesc.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    TransitionImageResource(depthdesc);
  }

  {
    auto image = ctx.resourceManager->GetImage(mColorTarget);
    auto depth = ctx.resourceManager->GetImage(mDepthTarget);

    VkRenderingAttachmentInfo colorAttachment = {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = image.view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingAttachmentInfo depthAttachment = {};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.pNext = nullptr;

    depthAttachment.imageView = depth.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil.depth = 1.f;

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D{
        VkOffset2D(0, 0), VkExtent2D(image.extent.width, image.extent.height)};
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachment;
    renderInfo.pDepthAttachment = &depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    // vkCmdBeginRendering(mCmdBuffer.buffer, &renderInfo);
    {
      auto bindingInfo = ctx.resourceManager->GetBindingInfo("scene_data");
      auto &buffer = ctx.resourceManager->GetBuffer(bindingInfo.handle);

      auto tQueue = ctx.device->GetCoreData().transferQueueFamily;
      auto gQueue = ctx.device->GetCoreData().graphicsQueueFamily;

      if (buffer.state.ownerQueueType != QueueType::Graphic) {
        //auto qindex = ctx.device->GetCoreData().graphicsQueueFamily;
        VkBufferMemoryBarrier srcBarrier =
            SynchronizationManager::CreateBufferMemoryBarrier(
                0, VK_ACCESS_SHADER_READ_BIT, buffer.buffer,
                tQueue, gQueue);
        buffer.state.queueFamilyIndex = gQueue;
        buffer.state.ownerQueueType = QueueType::Graphic;

        vkCmdPipelineBarrier(cb.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT, 0, 0,
                             nullptr, 1, &srcBarrier, 0, nullptr);
      }

      u32 sceneDataOffset =
          bindingInfo.global_offset + bindingInfo.local_offset;
      auto instanceBindingInfo =
          ctx.resourceManager->GetBindingInfo("instance_data");
      auto &instanceBuffer =
          ctx.resourceManager->GetBuffer(instanceBindingInfo.handle);

      if (instanceBuffer.state.ownerQueueType != QueueType::Graphic) {
        //auto qindex = ctx.device->GetCoreData().graphicsQueueFamily;
        VkBufferMemoryBarrier srcBarrier =
            SynchronizationManager::CreateBufferMemoryBarrier(
                0, VK_ACCESS_SHADER_READ_BIT, instanceBuffer.buffer,
                tQueue, gQueue);
        instanceBuffer.state.queueFamilyIndex = gQueue;
        instanceBuffer.state.ownerQueueType = QueueType::Graphic;

        vkCmdPipelineBarrier(cb.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT, 0, 0,
                             nullptr, 1, &srcBarrier, 0, nullptr);
      }
    }
    vkCmdBeginRendering(cb.buffer, &renderInfo);

    {
      // set dynamic viewport and scissor
      VkViewport viewport = {};
      viewport.x = 0;
      viewport.y = 0;
      viewport.width = ctx.viewport.width;
      viewport.height = ctx.viewport.height;
      viewport.minDepth = 0.f;
      viewport.maxDepth = 1.f;

      // vkCmdSetViewport(mCmdBuffer.buffer, 0, 1, &viewport);
      vkCmdSetViewport(cb.buffer, 0, 1, &viewport);

      VkRect2D scissor = {};
      scissor.offset.x = 0;
      scissor.offset.y = 0;
      scissor.extent.width = ctx.viewport.width;
      scissor.extent.height = ctx.viewport.height;

      // vkCmdSetScissor(mCmdBuffer.buffer, 0, 1, &scissor);
      vkCmdSetScissor(cb.buffer, 0, 1, &scissor);
    }
  }

  if (ctx.renderObjects.size() >= 1) {

    auto &renderObj = ctx.renderObjects[0];

    DescriptorSetAllocDesc allocDesc;
    for (auto k : renderObj.pipelineDesc.layoutDesc.setLayouts.layouts) {
      auto dlayout = ctx.pipelineManager->GetDescriptorSetLayout(k);
      allocDesc.layouts.push_back(dlayout.data);
    }
    auto sets = mDescriptorSetAllocator.Allocate(allocDesc);

    {
      DescriptorSetBinder binder(mDevice, sets[0]);

      auto bindingInfo = ctx.resourceManager->GetBindingInfo("scene_data");
      auto &buffer = ctx.resourceManager->GetBuffer(bindingInfo.handle);

      u32 sceneDataOffset =
          bindingInfo.global_offset + bindingInfo.local_offset;
      auto instanceBindingInfo =
          ctx.resourceManager->GetBindingInfo("instance_data");
      auto &instanceBuffer =
          ctx.resourceManager->GetBuffer(instanceBindingInfo.handle);

      u32 instanceDataOffset =
          instanceBindingInfo.global_offset + instanceBindingInfo.local_offset;

      binder.buffer(0, buffer, bindingInfo.alloc.size, sceneDataOffset,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

      binder.buffer(1, instanceBuffer, instanceBindingInfo.alloc.size,
                    instanceDataOffset, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

      binder.write();
    }

    auto pipeline = ctx.pipelineManager->GetPipeline(renderObj.pipelineDesc);
    auto pipelineLayout = ctx.pipelineManager->GetPipelineLayout(
        renderObj.pipelineDesc.layoutDesc);

    // vkCmdBindPipeline(mCmdBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                   pipeline.data);
    // vkCmdBindDescriptorSets(mCmdBuffer.buffer,
    // VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                         pipelineLayout.data, 0, sets.size(), sets.data(),
    //                         0, nullptr);

    vkCmdBindPipeline(cb.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.data);
    vkCmdBindDescriptorSets(cb.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout.data, 0, sets.size(), sets.data(), 0,
                            nullptr);

    u32 N = 4;
    u32 num_workgroups_x = N;
    u32 num_workgroups_y = N;
    u32 num_workgroups_z = N;

    // vkCmdDrawMeshTasksEXT(mCmdBuffer.buffer, num_workgroups_x,
    // num_workgroups_y,
    //                       num_workgroups_z);
    vkCmdDrawMeshTasksEXT(cb.buffer, num_workgroups_x, num_workgroups_y,
                          num_workgroups_z);
  }

  // vkCmdEndRendering(mCmdBuffer.buffer);
  vkCmdEndRendering(cb.buffer);

  {
    auto bindingInfo = ctx.resourceManager->GetBindingInfo("scene_data");
    auto &buffer = ctx.resourceManager->GetBuffer(bindingInfo.handle);

    auto tQueue = ctx.device->GetCoreData().transferQueueFamily;
    auto gQueue = ctx.device->GetCoreData().graphicsQueueFamily;

    //if (buffer.state.ownerQueueType != QueueType::Transfer) 
    {
      auto qindex = ctx.device->GetCoreData().graphicsQueueFamily;
      VkBufferMemoryBarrier srcBarrier =
          SynchronizationManager::CreateBufferMemoryBarrier(
              VK_ACCESS_SHADER_READ_BIT, 0, buffer.buffer, gQueue, tQueue);
      // buffer.state.queueFamilyIndex = qindex;
      // buffer.state.ownerQueueType = QueueType::Graphic;

      vkCmdPipelineBarrier(cb.buffer, VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           1, &srcBarrier, 0, nullptr);
    }

    u32 sceneDataOffset = bindingInfo.global_offset + bindingInfo.local_offset;
    auto instanceBindingInfo =
        ctx.resourceManager->GetBindingInfo("instance_data");
    auto &instanceBuffer =
        ctx.resourceManager->GetBuffer(instanceBindingInfo.handle);

    //if (instanceBuffer.state.ownerQueueType != QueueType::Graphic) 
    {
      // auto qindex = ctx.device->GetCoreData().tran;
      VkBufferMemoryBarrier srcBarrier =
          SynchronizationManager::CreateBufferMemoryBarrier(
              VK_ACCESS_SHADER_READ_BIT, 0, instanceBuffer.buffer, gQueue, tQueue);
      // instanceBuffer.state.queueFamilyIndex = qindex;
      // instanceBuffer.state.ownerQueueType = QueueType::Graphic;

      vkCmdPipelineBarrier(cb.buffer, VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           1, &srcBarrier, 0, nullptr);
    }
  }

  {
    ImageTransitionDesc colordesc;
    // colordesc.cmd = mCmdBuffer.buffer;
    colordesc.cmd = cb.buffer;
    colordesc.image = ctx.resourceManager->GetImage(mColorTarget).image;
    colordesc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colordesc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    TransitionImageResource(colordesc);

    ImageTransitionDesc swapdesc;
    // swapdesc.cmd = mCmdBuffer.buffer;
    swapdesc.cmd = cb.buffer;
    swapdesc.image = scImage;
    swapdesc.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapdesc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    TransitionImageResource(swapdesc);

    CopyImageToImageDesc copyDesc;
    // copyDesc.cmd = mCmdBuffer.buffer;
    copyDesc.cmd = cb.buffer;
    copyDesc.source = ctx.resourceManager->GetImage(mColorTarget).image;
    copyDesc.destination = scImage;
    // mark todo
    copyDesc.srcSize = {ctx.viewport.width, ctx.viewport.height};
    copyDesc.dstSize = {ctx.viewport.width, ctx.viewport.height};
    CopyImageToImage(copyDesc);

    swapdesc.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    swapdesc.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    TransitionImageResource(swapdesc);

    //
    // UI?
    //

    ImageTransitionDesc desc;
    // desc.cmd = mCmdBuffer.buffer;
    desc.cmd = cb.buffer;
    desc.image = scImage;
    desc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    desc.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    TransitionImageResource(desc);
  }

  // VK_CHECK(vkEndCommandBuffer(mCmdBuffer.buffer));
  // VK_CHECK(vkEndCommandBuffer(cb.buffer));
  ctx.commandBufferManager->EndCommandBuffer(cb);

  VkCommandBufferSubmitInfo cmdinfo = {};
  cmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdinfo.pNext = nullptr;
  cmdinfo.commandBuffer = cb.buffer; // mCmdBuffer.buffer;
  cmdinfo.deviceMask = 0;

  VkSemaphoreSubmitInfo waitinfo = {};
  waitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  waitinfo.pNext = nullptr;
  // waitinfo.semaphore = mSwapchainSemaphore.data;
  waitinfo.semaphore = mResourceSemaphore.data;
  waitinfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
  waitinfo.deviceIndex = 0;
  waitinfo.value = 1;

  VkSemaphoreSubmitInfo signalinfo = {};
  signalinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signalinfo.pNext = nullptr;
  signalinfo.semaphore = mRenderSemaphore.data;
  signalinfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  signalinfo.deviceIndex = 0;
  signalinfo.value = 1;

  VkSubmitInfo2 submitinfo = {};
  submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submitinfo.pNext = nullptr;

  submitinfo.waitSemaphoreInfoCount = 1;
  submitinfo.pWaitSemaphoreInfos = &waitinfo;

  submitinfo.signalSemaphoreInfoCount = 1;
  submitinfo.pSignalSemaphoreInfos = &signalinfo;

  submitinfo.commandBufferInfoCount = 1;
  submitinfo.pCommandBufferInfos = &cmdinfo;

  VK_CHECK(vkQueueSubmit2(mGraphicsQueue, 1, &submitinfo, mRenderFence.data));
}

void Frame::End(VulkanContext ctx, VkImage scImage) {}

void Frame::ResetDescriptorSet() { mDescriptorSetAllocator.Reset(); }
} // namespace unknown::renderer::vulkan