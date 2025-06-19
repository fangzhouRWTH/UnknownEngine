#include "vulkan_renderer/memAllocation.hpp"
#include "memAllocation.hpp"
#include "platform/type.hpp"

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <new>
#include <iostream>

#if defined(_MSC_VER)
    #include <malloc.h>
#endif

inline void* ue_aligned_alloc(size_t alignment, size_t size) {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__linux__)
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#else
    return std::aligned_alloc(alignment, size);
#endif
}

inline void ue_aligned_free(void* ptr) {
#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

namespace unknown::renderer::vulkan {
LinearMemory::LinearMemory(u32 sizeByte, u32 alignment)
    : mSizeByte(alignUp(sizeByte,alignment)), mAlignment(alignment) {
  assert(isPow2(alignment));
  mMemPtr = static_cast<u8 *>(ue_aligned_alloc(mAlignment, mSizeByte));
  assert(mMemPtr!=nullptr);
}

LinearMemory::~LinearMemory() 
{
    if(isSource)
        ue_aligned_free(mMemPtr);
}

LinearMemory::LinearMemory(LinearMemory & source, u32 size, u32 alignment) :
LinearMemory(size,alignment) 
{
    MemoryAllocation alloc = source.allocate(size,alignment);
    mMemPtr = alloc.ptr;
    assert(mMemPtr != nullptr);
    isSource = false;
}

MemoryAllocation LinearMemory::allocate(u32 size) {
  u32 pos = alignUp(mCurrentOffset, mAlignment);
  u32 allcSize = alignUp(size, mAlignment);
  u32 end = pos + allcSize;
  assert(end <= mSizeByte);
  if (end > mSizeByte)
    return MemoryAllocation();

  mCurrentOffset = end;

  MemoryAllocation alloc;
  alloc.ptr = (byte*)mMemPtr + pos;
  alloc.reserve = allcSize;
  alloc.size = size;

  memset(alloc.ptr, 'x', alloc.reserve);

  return alloc;
}

MemoryAllocation LinearMemory::allocate(u32 size, u32 alignment) 
{ 
    assert(isPow2(alignment));
    assert(alignment<=mAlignment);
    if(!isPow2(alignment))
        return MemoryAllocation(); 

    u32 pos = alignUp(mCurrentOffset,alignment);
    u32 allcSize = alignUp(size,alignment);
    u32 end = pos + allcSize;
    assert(end<=mSizeByte);
    if(end>mSizeByte)
        return MemoryAllocation(); 
    
    mCurrentOffset = end;

    MemoryAllocation alloc;
    alloc.ptr = (byte*)mMemPtr + pos;
    alloc.reserve = allcSize;
    alloc.size = size;

    memset(alloc.ptr,'x',alloc.reserve);

    return alloc;
}
} // namespace unknown::renderer::vulkan