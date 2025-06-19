#pragma once
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/command.hpp"
#include "utils/container.hpp"
#include "vulkan_renderer/resourceAllocator.hpp"

namespace unknown::renderer::vulkan {

struct BufferPage
{
  u32 size;
};

struct BufferTracker
{
  BufferHandle handle;
  u32 size = 0;
  u32 currentOffset = 0;
};

class ResourceTrackerManager
{
public:
  std::vector<BufferTracker> mActivePools;
  //std::vector<BufferTracker>

  //u32 mAlignment = 16u;
};

class UniformRegistrationManager
{
  public:
    UniformHandle Add(const UniformRegistrationInfo & uReg)
    {
      return mUniformRegistration.push(uReg);
    }

    UniformRegistrationInfo& Get(const UniformHandle & handle)
    {
      return mUniformRegistration.get(handle);
    }

    const UniformRegistrationInfo& Get(const UniformHandle & handle) const
    {
      return mUniformRegistration.get(handle);
    }

    void Set(const UniformHandle & handle,UniformRegistrationInfo info)
    {
      mUniformRegistration.get(handle) = info;
    }

    void SetMemAlloc(const UniformHandle & handle, const MemoryAllocation & alloc)
    {
      auto & info = mUniformRegistration.get(handle);
      info.alloc = alloc;
      info.dirty = true;
    }

    void SetDirty(const UniformHandle & handle,bool dirty)
    {
      auto & info = mUniformRegistration.get(handle);
      info.dirty = dirty;
    }

    UniformRegistrationInfo Remove(const UniformHandle & handle)
    {
      auto info = mUniformRegistration.get(handle);
      mUniformRegistration.release(handle);
      return info;
    }

    
  private:
    VectorContainer<UniformHandle,UniformRegistrationInfo> mUniformRegistration;

};



} // namespace unknown::renderer::vulkan