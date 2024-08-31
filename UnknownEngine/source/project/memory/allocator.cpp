#include "allocator.hpp"
#include <stdlib.h>
#include <cassert>

namespace unknown::memory
{
    IAllocator::IAllocator(const u32 size) : _memory_size(size) {}

    LinearAllocator::LinearAllocator(u32 size) : IAllocator(size)
    {
        // ???
        if (_data != nullptr)
        {
            free(_data);
        }

        _data = malloc(size);
        _used_memory_size = 0u;
    }

    LinearAllocator::~LinearAllocator()
    {
        free(_data);
    }

    void *LinearAllocator::Allocate(const u32 size, const u32 alignment)
    {
        // TODO
        u32 _padding = 0u;
        u64 _next_free = (u64)_data + (u64)_used_memory_size;

        if (alignment != 0u)
            _padding = Padding(_next_free, alignment);

        _next_free += u64(_padding);

        assert(_next_free<(u64)_data + _memory_size);

        void *ptr = (void *)_next_free;

        _used_memory_size += (size + _padding);

        return ptr;
    }

    void LinearAllocator::Free(void *ptr)
    {
        _used_memory_size = 0;
    }

    PoolAllocator::PoolAllocator(u32 size, u32 blockSize) : IAllocator(size), _block_size(blockSize)
    {
        if (_data != nullptr)
        {
            free(_data);
        }

        _data = malloc(size);
        _used_memory_size = 0u;
        _allocs_number = 0u;

        _node_number = size / blockSize;
        void *_address = _data;
        u64 _next_free = (u64)_address;

        for (u32 i = 0u; i < _node_number; i++)
        {
            _free_list.push_back(_address);
            _next_free += blockSize;
            _address = (void *)_next_free;
            _allocs_number++;
        }
    }

    PoolAllocator::~PoolAllocator()
    {
        free(_data);
        //_free_list.empty();
        //_used_memory_size = 0u;
        //_allocs_number = 0u;
    }

    void *PoolAllocator::Allocate(const u32 size, const u32 alignment)
    {
        assert(size == _block_size);
        assert(!_free_list.empty());
        void *_next_free = _free_list.back();
        _free_list.pop_back();
        _allocs_number--;
        _used_memory_size += _block_size;
        return _next_free;
    }

    void PoolAllocator::Free(void *ptr)
    {
        assert(_allocs_number < _node_number);
        _free_list.push_back(ptr);
        _allocs_number++;
        _used_memory_size -= _block_size;
    }
}