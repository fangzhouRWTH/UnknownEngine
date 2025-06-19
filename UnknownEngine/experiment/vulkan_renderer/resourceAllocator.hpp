#pragma once

#include "utils/container.hpp"
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/types.hpp"


namespace unknown::renderer::vulkan {

template<typename HANDLE, typename RESOURCE>
class ResourceContainer 
{
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
  HANDLE addResource(const RESOURCE &res) {
    return mContainer.push(res);
  }
  RESOURCE getResource(const HANDLE &handle) const {
    return mContainer.get(handle);
  }
  RESOURCE & getResource(const HANDLE &handle) {
    return mContainer.get(handle);
  }
  void releaseResource(const HANDLE &handle) {
    mContainer.release(handle);
  }
  void resetContainer() { mContainer.reset(); }

private:
  VectorContainer<HANDLE, RESOURCE> mContainer;
};

class GPUResourceAllocator {
public:
  ~GPUResourceAllocator();
  VmaAllocator get() const { return mAllocator; }
  void Init(const VulkanCoreData & vkData);

  BufferHandle CreateBuffer(const BufferDesc &desc);
  Buffer GetBuffer(const BufferHandle &handle) const;
  Buffer & GetBuffer(const BufferHandle &handle);
  void Release(const BufferHandle & handle);
  void ReleaseAllBuffers();

  //Buffer AcquireBuffer(const BufferHandle & handle, QueueType queue);

  ImageHandle CreateImage(const ImageDesc &desc);
  Image GetImage(const ImageHandle &handle);
  void Release(const ImageHandle & handle);
  void ReleaseAllImages();

  //void MapBuffer(u32 size, void* src, ResourceHandle dst, u32 dstOffset);
  //void MapBuffer(u32 size, void* src, Buffer dst, u32 dstOffset);

  void MapCopyBuffer(void* src, BufferHandle dstHandle, u32 dstOffset, u32 size);

  void GPUCopyBuffer(const CommandBuffer &cmd, BufferHandle srcHandle, BufferHandle dstHandle, u32 srcOffset, u32 dstOffset, u32 size, ResourceStage stage);
  //void StagingBuffer(const CommandBuffer &cmd, u32 size, ResourceHandle src, ResourceHandle dst, u32 srcOffset, u32 dstOffset, ResourceStage stage);
  //void StagingBuffer(const CommandBuffer &cmd, u32 size, Buffer src, Buffer dst, u32 srcOffset, u32 dstOffset, ResourceStage stage);

  void ReleaseAll();

  void Destroy();

private:
  void unmapMappedBuffer(Buffer buffer);
  //void releasImage(const ResourceHandle &handle);

private:
  // bool mUseTransferQueue = false;
  // u32 mQueueFamilyIndex;
  // VkQueue mQueue;
  VulkanCoreData mVkData;
  
  VmaAllocator mAllocator;
  // VkPhysicalDevice mPhysicalDevice;
  // VkDevice mDevice;
  // VkInstance mInstance;

  ResourceContainer<ImageHandle,Image> mImages;
  ResourceContainer<BufferHandle,Buffer> mBuffers;
};

} // namespace unknown::renderer::vulkan