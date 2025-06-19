#pragma once

#include "vulkan_renderer/common.hpp"

#include <vector>

namespace unknown::renderer::vulkan {
struct Frame;

struct SwapchainInitInfo {
  u32 width;
  u32 height;

  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkSurfaceKHR surface;
  VkQueue graphicsQueue;
};
class Swapchain {
public:
  void Init(const SwapchainInitInfo &info);
  void Destroy();
  bool UpdateNextIndex(const Frame& frame);
  VkImage GetCurrentImage();
  void Present(const Frame& frame);

  bool NeedResize() {return isResizeRequired;}
  void SetResize(bool b) {isResizeRequired = b;}

//private:
  u32 nextIndex;
  bool isResizeRequired = false;

  VkDevice device;
  VkQueue graphicsQueue;
  VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkExtent2D extent;
  VkSwapchainKHR swapchain;
  std::vector<VkImage> images;
  std::vector<VkImageView> views;
};
} // namespace unknown::renderer::vulkan