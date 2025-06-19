#include "vulkan_renderer/swapchain.hpp"
#include "vulkan_renderer/frame.hpp"

#include "VkBootstrap.h"
#include "swapchain.hpp"

namespace unknown::renderer::vulkan {
void Swapchain::Init(const SwapchainInitInfo &info) {
  vkb::SwapchainBuilder swapchainBuilder{info.physicalDevice, info.device,
                                         info.surface};
  vkb::Swapchain vkbSwapchain =
      swapchainBuilder //.use_default_format_selection()
          .set_desired_format(VkSurfaceFormatKHR{
              .format = imageFormat,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // use vsync present mode
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(info.width, info.height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  device = info.device;
  graphicsQueue = info.graphicsQueue;
  extent = vkbSwapchain.extent;
  // store swapchain and its related images
  swapchain = vkbSwapchain.swapchain;
  images = vkbSwapchain.get_images().value();
  views = vkbSwapchain.get_image_views().value();
}

void Swapchain::Destroy()
{
  vkDestroySwapchainKHR(device, swapchain, nullptr);
  size_t count = views.size();
  for (size_t i = 0; i < count; i++) {
    vkDestroyImageView(device, views[i], nullptr);
  }
}

bool Swapchain::UpdateNextIndex(const Frame &frame) 
{ 
  u32 index;
  VkResult res = vkAcquireNextImageKHR(device, swapchain, 1000000000,
                                       frame.mSwapchainSemaphore.data, nullptr,
                                       &index);
  nextIndex = index;

  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    isResizeRequired = true;
    return false;
  }

  return true; 
}
VkImage Swapchain::GetCurrentImage() 
{ 
  return images[nextIndex];
}
} // namespace unknown::renderer::vulkan

void unknown::renderer::vulkan::Swapchain::Present(const Frame &frame) 
{
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = 0;
  //presentInfo.swapchainCount = 0;
  //presentInfo.pSwapchains = nullptr;
  //presentInfo.pWaitSemaphores = nullptr;
  //presentInfo.waitSemaphoreCount = 0;
  //presentInfo.pImageIndices = nullptr;

  presentInfo.pSwapchains = &swapchain;
  presentInfo.swapchainCount = 1;
  presentInfo.pWaitSemaphores = &frame.mRenderSemaphore.data;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pImageIndices = &nextIndex;

  VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
}
