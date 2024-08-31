#pragma once

#include "platform/type.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>

namespace unknown
{
    template<typename Handle>
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
            u32 index = handle.Get();
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
            assert(handle.Get() != HandleType::kInvalidHandle);
            assert(handle.Get() == mHashes.size());
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
            u32 index = handle.Get();
            if (index >= mResourceArray.size())
                return false;

            info = mResourceArray[index];
            return true;
        }

        HandleType AddResource(ResourceInfo info)
        {
            HandleType handle = HandleType::CreateHandle();
            assert(handle.Get() != HandleType::kInvalidHandle);
            assert(handle.Get() == mResourceArray.size());
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
}