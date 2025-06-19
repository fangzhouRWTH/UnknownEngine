#pragma once
#include "core/bit.hpp"

namespace unknown::renderer::vulkan {
#define MB 1048576u

struct MemoryAllocation { 
  void *ptr = nullptr;
  u32 size = 0u;
  u32 reserve = 0u;
};

inline bool isPow2(u32 size)
{
    return (size&(size-1))==0;
}

inline u32 alignUp(u32 size, u32 alignment) {
    return (size + (alignment -1)) & ~(alignment - 1);
}

class LinearMemory {
public:
  LinearMemory(u32 sizeByte, u32 alignment);
  ~LinearMemory();

  LinearMemory(const LinearMemory &) = delete;
  LinearMemory & operator=(const LinearMemory &) = delete;

  LinearMemory(LinearMemory& source, u32 size, u32 alignment);

  void reset() { mCurrentOffset = 0; }
  void rewind(u32 pos) { assert(pos <= mCurrentOffset && pos < mSizeByte); }
  u32 getOffset() const { return mCurrentOffset; }
  void *getBasePtr() const { return (void *)mMemPtr; }

  MemoryAllocation allocate(u32 size);
  MemoryAllocation allocate(u32 size,u32 alignment);
private:
  void *mMemPtr = nullptr;
  u32 mCurrentOffset = 0;
  const u32 mSizeByte;
  const u32 mAlignment;
  bool isSource = true;
};
} // namespace unknown::renderer::vulkan