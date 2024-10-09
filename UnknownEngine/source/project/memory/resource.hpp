#pragma once

#include "platform/type.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>
#include <type_traits>
#include <queue>

namespace unknown
{
    template <typename Handle>
    struct ResourceInfos
    {
        u32 id;
        u64 hash;
        Handle handle;
        bool valid = false;
    };

    template <typename HandleType, typename ResourceInfo>
    class ResourceMap
    {
    public:
        ResourceMap() {}
        bool IsFull()
        {
            return HandleType::Current() == HandleType::kInvalidHandle;
        }

        bool FindResource(HandleType handle, ResourceInfo &info) const
        {
            u32 index = handle.value();
            if (index >= mHashes.size())
                return false;
            u32 hash = mHashes[index];

            return FindResource(hash, info);
        }

        bool FindResource(u32 hash, ResourceInfo &info) const
        {
            auto res = mResourceInfoMap.find(hash);
            if (res != mResourceInfoMap.end())
            {
                info = res->second;
                return true;
            }
            else
                return false;
        }

        HandleType AddResource(u32 hash, ResourceInfo info)
        {
            ResourceInfo in;
            // assert(!findResource(hash, in));

            if (FindResource(hash, in))
                return in.handle;

            HandleType handle = HandleType::CreateHandle();
            assert(handle.value() != HandleType::kInvalidHandle);
            assert(handle.value() == mHashes.size());
            mHashes.emplace_back(hash);
            info.handle = handle;
            info.valid = true;
            mResourceInfoMap[hash] = info;
            return handle;
        }

        void RemoveResource(HandleType handle)
        {
            // FindResource(handle,)
        }

    private:
        std::vector<u32> mHashes;
        std::unordered_map<u32, ResourceInfo> mResourceInfoMap;
    };

    template <typename HandleType, typename ResourceInfo>
    class ResourceArray
    {
    public:
        ResourceArray() {}
        bool IsFull()
        {
            return mResourceArray.size() == HandleType::kInvalidHandle;
        }

        bool FindResource(HandleType handle, ResourceInfo &info) const
        {
            u32 index = handle.value();
            if (index >= mResourceArray.size())
                return false;

            info = mResourceArray[index];
            return true;
        }

        HandleType AddResource(ResourceInfo info)
        {
            HandleType handle = HandleType::CreateHandle();
            assert(handle.value() != HandleType::kInvalidHandle);
            assert(handle.value() == mResourceArray.size());
            info.handle = handle;
            info.valid = true;
            mResourceArray.emplace_back(info);
            return handle;
        }

        void RemoveResource(HandleType handle)
        {
            // FindResource(handle,)
        }

    private:
        std::vector<ResourceInfo> mResourceArray;
    };

    template<typename DataType>
    struct RecycleArray
    {
        std::vector<DataType> data;
        std::queue<u32> freed;

        u32 reserve(u32 size)
        {
            data.reserve(size);
        }

        u32 create()
        {
            u32 idx;
            if(!freed.empty())
            {
                idx = freed.front();
                freed.pop();
            }
            else
            {
                idx = data.size();
                data.emplace_back();
            }
            return idx;
        }

        void free(u32 idx)
        {
            freed.push(idx);
        }

        DataType& get(u32 idx)
        {
            assert(idx < data.size());
            return data[idx];
        }

        const DataType& get(u32 idx) const
        {
            assert(idx < data.size());
            return data[idx];
        }
    };

    template <typename HandleType, typename DataType>
    class ResourceTable
    {
        //todo
        //static_assert(std::is_base_of<HandleTemplate<HandleType>, HandleType>);
        public:
            HandleType Create()
            {
                HandleType handle;
                if(!IsFull())
                {
                    u32 h2 = mContainer.create();
                    handle.value() = mKeys.size();
                    mKeys.push_back(h2);
                }
                return handle;
            }

            void Remove(HandleType handle)
            {
                if(handle.value()<mKeys.size())
                {
                    u32 k = mKeys[handle.value()];
                    if(k >= mCapacity)
                        return;

                    mKeys[handle.value()] = mCapacity;
                    mContainer.free(k);
                }
            }

            DataType* Get(HandleType handle)
            {
                if(handle.value()>=mKeys.size())
                    return nullptr;

                u32 key = mKeys[handle.value()];
                if(key < mCapacity)
                {
                    return &mContainer.get(key);
                }
                else
                {
                    return nullptr;
                }
            }

            bool Get(HandleType handle, DataType & data) const
            {
                if(handle.value()>=mKeys.size())
                    return false;

                u32 key = mKeys[handle.value()];
                if(key < mCapacity)
                {
                    data = mContainer.get(key);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            bool Set(HandleType handle, const DataType & data)
            {
                if(handle.value()>=mKeys.size())
                    return false;

                u32 key = mKeys[handle.value()];
                if(key < mCapacity)
                {
                    mContainer.get(key) = data;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            bool IsFull()
            {
                return Count() >= mCapacity;
            }

            u32 KeySize()
            {
                return mKeys.size();
            }

            u32 Size()
            {
                return mContainer.data.size();
            }

            u32 FreeQueueSize()
            {
                return mContainer.freed.size();
            }

            u32 Count()
            {
                return Size() - FreeQueueSize();
            }
        private:
            u32 mCapacity = HandleType::kInvalidHandle;
            std::vector<u32> mKeys;
            RecycleArray<DataType> mContainer;
    };
}