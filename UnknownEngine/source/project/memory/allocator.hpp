#pragma once
#include "platform/type.hpp"
#include <list>

namespace unknown::memory
{
#define MEM_KB 1024u
#define MEM_MB 1024u * 1024u
#define MEM_GB 1024u * 1024u * 1024u

    class IAllocator
    {
    public:
        IAllocator(const u32 size);
        const u32 _memory_size;
        u32 _used_memory_size;
        void *_data = nullptr;
        virtual void *Allocate(const u32 size, const u32 alignment) = 0;
        virtual void Free(void *ptr) = 0;
    };

    inline u32 Padding(const u64 offset, const u32 alignment)
    {
        return u32((~offset + 1) & (u64(alignment) - 1));
    }

    template <typename T, typename... Args>
    inline T *Allocate(IAllocator &allocator, Args &&...args)
    {
        return new (allocator.Allocate(sizeof(T), __alignof(T))) T(std::forward<Args>(args)...);
    }

    template <typename T>
    inline void Delete(IAllocator &allocator, T *ptr)
    {
        ptr->~T();
        allocator.Free(ptr);
    }

    class LinearAllocator : public IAllocator
    {
    public:
        LinearAllocator(u32 size);
        ~LinearAllocator();
        virtual void *Allocate(const u32 size, const u32 alignment) override;
        virtual void Free(void *ptr) override;
    };

    class PoolAllocator : public IAllocator
    {
    public:
        PoolAllocator(const u32 size, const u32 blockSize);
        ~PoolAllocator();
        virtual void *Allocate(const u32 size, const u32 alignment) override;
        virtual void Free(void *ptr) override;

    private:
        std::list<void *> _free_list;
        u32 _block_size;
        u32 _node_number;
        u32 _allocs_number;
    };
}