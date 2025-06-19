#pragma once

#include "utils/container.hpp"
#include "volk.h"
#include "vulkan_renderer/memAllocation.hpp"
#include "vulkan_renderer/sstring.hpp"

namespace unknown::renderer::vulkan {

struct alignas(16) TestSceneData {
  Mat4f view;
  Mat4f proj;
  Mat4f view_proj;

  Vec4f color1;
  Vec4f color2;
  Vec4f color3;
  Vec4f color4;

  float cull_center_x = -1.0f;
  float cull_center_y = -1.0f;
  float cull_radius = 1.0f;
  float meshlet_density = 2.0f;

  u32 instanceCount = 0u;
};

struct alignas(16) TestInstanceData {
  Mat4f transform;
  Vec4f color;
};

struct TaskUniform {
  Mat4f view;
  Mat4f proj;
  Mat4f view_proj;

  float cull_center_x = -1.0f;
  float cull_center_y = -1.0f;
  float cull_radius = 1.0f;
  float meshlet_density = 2.0f;
};

enum class QueueType
{
  Graphic,Transfer,Compute,
};

struct VulkanCoreData {
  VkDevice device;
  VkPhysicalDevice physicalDevice;
  VkInstance instance;
  VkSurfaceKHR surface;

  VkQueue graphicsQueue;
  u32 graphicsQueueFamily;

  bool useTransferQueue = true;
  bool hasTransferQueue = false;
  VkQueue transferQueue;
  u32 transferQueueFamily;

  bool useComputeQueue = true;
  bool hasComputeQueue = false;
  VkQueue computeQueue;
  u32 computeQueueFamily;
};

struct Viewport {
  u32 width;
  u32 height;
};

inline bool isSet(u64 src, u64 bit) { return src & bit; }

struct BufferUsage {
  BufferUsage &uniform() {
    flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    return *this;
  }

  BufferUsage &transfer_dst() {
    flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return *this;
  }

  BufferUsage &transfer_src() {
    flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    return *this;
  }

  BufferUsage &shader_storage() {
    flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    return *this;
  }

  BufferUsage &shader_device() {
    flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    return *this;
  }

  void reset() { flags = 0; }

  VkBufferUsageFlags get() const { return flags; }

private:
  VkBufferUsageFlags flags = 0;
};

struct MemoryUsage {
  // MemoryUsage &gpu() {
  //   flags = VMA_MEMORY_USAGE_GPU_ONLY;
  //   return *this;
  // }
  // MemoryUsage &cpu() {
  //   flags = VMA_MEMORY_USAGE_CPU_ONLY;
  //   return *this;
  // }
  // MemoryUsage &cpu_gpu() {
  //   flags = VMA_MEMORY_USAGE_CPU_TO_GPU;
  //   return *this;
  // }
  void prefer_device() { flags = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; }

  void prefer_host() { flags = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; }

  void prefer_auto() { flags = VMA_MEMORY_USAGE_AUTO; }

  void prefer_lazy() { flags = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED; }

  VmaMemoryUsage get() const {return flags;}

  // MemoryUsage &property_local() {
  //   property = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  //   return *this;
  // }
  // MemoryUsage &property_map() {
  //   property = VkMemoryPropertyFlags(VMA_ALLOCATION_CREATE_MAPPED_BIT);
  //   return *this;
  // }


  //VmaMemoryUsage getUsage() const { return flags; }
  //VkMemoryPropertyFlags getProperty() const { return property; }

private:
  VmaMemoryUsage flags = VMA_MEMORY_USAGE_UNKNOWN;
  //VkMemoryPropertyFlags property;
};

struct MemoryProperty {
  void staging() {
    property = VMA_ALLOCATION_CREATE_MAPPED_BIT |
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    onlyStaging = true;
  }
  
  VmaAllocationCreateFlags get() const
  {
    return property;
  }

  bool isOnlyStaging() const {return onlyStaging;}
private:
  VmaAllocationCreateFlags property = 0;
  bool onlyStaging;
};

struct BufferDesc {
  u64 size = 0;
  BufferUsage bufferUsage;
  MemoryUsage memoryUsage;
  MemoryProperty memoryProperty;
};

struct ImageUsage {
  ImageUsage &transfer_src() {
    flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    return *this;
  }
  ImageUsage &transfer_dst() {
    flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return *this;
  }
  ImageUsage &shader_storage() {
    flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    return *this;
  }
  ImageUsage &color_attachment() {
    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    return *this;
  }
  ImageUsage &depth_attachment() {
    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return *this;
  }

  VkImageUsageFlags get() const { return flags; }

private:
  VkImageUsageFlags flags = 0;
};

struct ImageFormat {
  ImageFormat &size(u32 width, u32 height, u32 depth = 1u) {
    extent.width = width;
    extent.height = height;
    extent.depth = depth;
    return *this;
  }

  ImageFormat &rgba_16_sf() {
    format = VK_FORMAT_R16G16B16A16_SFLOAT;
    return *this;
  }

  ImageFormat &d_32_sf() {
    format = VK_FORMAT_D32_SFLOAT;
    return *this;
  }

  ImageFormat &color_aspect() {
    aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    return *this;
  }

  ImageFormat &depth_aspect() {
    aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    return *this;
  }

  ImageFormat &stencil_aspect() {
    aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    return *this;
  }

  ImageFormat &depth_stencil_aspect() {
    aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    return *this;
  }

  VkExtent3D getExtent() const { return extent; }
  VkFormat getFormat() const { return format; }
  VkImageAspectFlags getAspect() const { return aspect; }

private:
  VkFormat format;
  VkExtent3D extent;
  VkImageAspectFlags aspect;
};

struct ImageDesc {
  ImageFormat imageFormat;
  ImageUsage imageUsage;
  MemoryUsage memoryUsage;
  MemoryProperty memoryProperty;
};

struct BufferState {
  bool isMapped = false;
  bool onlyStaging = false;
  u32 queueFamilyIndex;
  QueueType ownerQueueType;
  void* mapPtr = nullptr;
};

struct Buffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo info;

  BufferState state;
};

struct Image {
  VkImage image;
  VkImageView view;
  VmaAllocation allocation;
  VkExtent3D extent;
  VkFormat format;

  //ResourceState state;
};

enum struct ResourceStage {
  UniformTaskMesh,
  UniformVertexFragment,
  UniformCompute,

  StorageTaskMesh,
  StorageVertexFragment,
  StorageCompute,

  TransferRead,
  TransferWrite,

  General
};

enum struct UniformType { FrameUniform, FrameStorage, Uniform, Storage };

// enum class ResourceType { Image, Buffer, NotDefined };

// union ResourceUnion {
//   Buffer buffer;
//   Image image;
// };

// struct Resource {
//   ResourceUnion data;
//   ResourceType type;
//   ResourceState state;
// };

// DECL_CONTAINER_HANDLE(ResourceHandle, Recycle, VectorContainer, Resource);

DECL_CONTAINER_HANDLE(ImageHandle, Recycle, VectorContainer, Image);
DECL_CONTAINER_HANDLE(BufferHandle, Recycle, VectorContainer, Buffer);

// typedef ResourceHandle ImageHandle;
// typedef ResourceHandle BufferHandle;

struct UniformDesc {
  // UniformUpdate update;
  UniformType type;
  SString name;
  u32 size;
};

struct UniformRegistrationInfo {
  MemoryAllocation alloc;
  UniformDesc desc;
  BufferHandle handle;
  u32 local_offset = 0u;
  u32 global_offset = 0u;
  bool dirty;
};

DECL_CONTAINER_HANDLE(UniformHandle, Recycle, VectorContainer,
                      UniformRegistrationInfo);

struct DynamicMemoryBlock {
  BufferHandle handle;
  u32 size;
};
} // namespace unknown::renderer::vulkan