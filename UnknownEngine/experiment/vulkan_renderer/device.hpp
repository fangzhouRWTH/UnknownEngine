#pragma once

#include "common.hpp"

namespace unknown::renderer::vulkan {
struct DeviceInitInfo {
  void *windowPtr;
};

class Device {
public:
  void Init(const DeviceInitInfo &info);

  VulkanCoreData GetCoreData() const {return vkData; }

  VkDevice GetDevice() const { return vkData.device; }
  VkPhysicalDevice GetGPU() const { return vkData.physicalDevice; }
  VkInstance GetInstance() const {return vkData.instance;}
  VkSurfaceKHR GetSurface() const { return vkData.surface; }

  uint32_t GetGraphicsQueueIndex() const {return vkData.graphicsQueueFamily;}
  VkQueue GetGraphicsQueue() const {return vkData.graphicsQueue;}
  uint32_t GetTransferQueueIndex() const {return vkData.transferQueueFamily;}
  VkQueue GetTransferQueue() const {return vkData.transferQueue;}
  uint32_t GetComputeQueueIndex() const {return vkData.computeQueueFamily;}
  VkQueue GetComputeQueue() const {return vkData.computeQueue;}

  VkDeviceSize GetUniformBufferAlignment() const {return properties.limits.minUniformBufferOffsetAlignment;}
  VkDeviceSize GetStorageBufferAlignment() const {return properties.limits.minStorageBufferOffsetAlignment;}
  VkDeviceSize GetCopyAlignment() const {return properties.limits.optimalBufferCopyOffsetAlignment;}
  VkDeviceSize GetNonCoherentAlignment() const {return properties.limits.nonCoherentAtomSize;}
  VkDeviceSize GetUniformMaxRange() const {return properties.limits.maxUniformBufferRange;}
  VkDeviceSize GetStorageMaxRange() const {return properties.limits.maxStorageBufferRange;}

  void ShutDown();

private:
  bool bInit = false;

  VulkanCoreData vkData;

  VkPhysicalDeviceProperties properties;

  VkDebugUtilsMessengerEXT debugMessenger;
};
} // namespace unknown::renderer::vulkan