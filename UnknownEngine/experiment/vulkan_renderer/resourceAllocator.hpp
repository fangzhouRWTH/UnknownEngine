#pragma once

#include "utils/container.hpp"
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/types.hpp"

namespace unknown::renderer::vulkan {

template <typename HANDLE, typename RESOURCE> class ResourceContainer {
public:
  std::vector<std::pair<bool, RESOURCE>> getRawResourceStorage() {
    return mContainer.getContainer();
  }
  void getResources(std::vector<RESOURCE> &resources) {
    auto &container = mContainer.getContainer();
    for (auto &r : container) {
      if (r.first)
        resources.push_back(r.second);
    }
  }
  HANDLE addResource(const RESOURCE &res) { return mContainer.push(res); }
  RESOURCE getResource(const HANDLE &handle) const {
    return mContainer.get(handle);
  }
  RESOURCE &getResource(const HANDLE &handle) { return mContainer.get(handle); }
  void releaseResource(const HANDLE &handle) { mContainer.release(handle); }
  void resetContainer() { mContainer.reset(); }

private:
  VectorContainer<HANDLE, RESOURCE> mContainer;
};

class GPUResourceAllocator {
public:
  ~GPUResourceAllocator();
  VmaAllocator get() const { return mAllocator; }
  void Init(const VulkanCoreData &vkData);

  BufferHandle CreateBuffer(const BufferDesc &desc);

  Buffer GetBuffer(const BufferHandle &handle) const;
  Buffer &GetBuffer(const BufferHandle &handle);

  void Release(const BufferHandle &handle);
  void ReleaseAllBuffers();

  ImageHandle CreateImage(const ImageDesc &desc);
  Image GetImage(const ImageHandle &handle);
  void TransitionImageResource(const VkCommandBuffer & cmd, const ImageHandle &handle, VkImageLayout dst);
 
  //todo
  ImageHandle LoadCreateImage(std::shared_ptr<CommandBufferManager<FRAME_OVERLAP>> & cmdManager, bool useTransferQueue, ImageDesc &desc, const std::string & filePath, u32 & width,u32 & height,u32 & channels);
  void Release(const ImageHandle &handle);
  void ReleaseAllImages();

  void ReleaseOwnership(const CommandBuffer & cmd, BufferHandle handle, QueueType from, QueueType to);
  void AcquireOwnership(const CommandBuffer & cmd, BufferHandle handle, QueueType from, QueueType to);
  
  void ReleaseOwnership(const CommandBuffer & cmd, ImageHandle handle, QueueType from, QueueType to);
  void AcquireOwnership(const CommandBuffer & cmd, ImageHandle handle, QueueType from, QueueType to);

  void MapCopyBuffer(void *src, BufferHandle dstHandle, u32 dstOffset,
                     u32 size);

  void GPUCopyBuffer(const CommandBuffer &cmd, BufferHandle srcHandle,
                     BufferHandle dstHandle, u32 srcOffset, u32 dstOffset,
                     u32 size, ResourceStage stage);

  void ReleaseAll();

  void Destroy();

private:
  void unmapMappedBuffer(Buffer buffer);
  ResourceOwnershipInfo
  getResourceTransferInfo(const ResourceOwnershipTransition &transition);

  void getQueueInfo(QueueType type, VkQueue & queue, u32 & index);

private:
  VulkanCoreData mVkData;

  VmaAllocator mAllocator;

  ResourceContainer<ImageHandle, Image> mImages;
  ResourceContainer<BufferHandle, Buffer> mBuffers;
};

} // namespace unknown::renderer::vulkan