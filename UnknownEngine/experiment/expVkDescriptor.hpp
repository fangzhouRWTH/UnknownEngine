#pragma once
#include "volk.h"
#include <stdint.h>
#include <vector>
#include <deque>
#include "expVkDefines.hpp"

namespace unknown::exp
{
    struct DescriptorPoolPart
    {
        VkDescriptorType type;
        float ratio;
    };

    struct DescriptorSetAllocator
    {
    public:
        explicit DescriptorSetAllocator(float scaleValue = 1.5f, uint32_t limits = 4096) : scale(scaleValue), growLimits(limits) {}

        void init(VkDevice device, uint32_t initialMaxSetCount, std::vector<DescriptorPoolPart> poolParts)
        {
            deviceRef = device;
            float sum = 0.f;
            for (auto p : poolParts)
            {
                sum += p.ratio;
            }
            if (sum <= 0.f)
                return;
            for (auto p : poolParts)
            {
                parts.emplace_back(DescriptorPoolPart{p.type, p.ratio / sum});
            }
            auto newPool = create_pool(device, initialMaxSetCount, poolParts);
            readyPools.push_back(newPool);
            setPerPool_nextAlloc = uint32_t(initialMaxSetCount * scale);
        }
        void clear()
        {
            for (auto p : readyPools)
            {
                vkResetDescriptorPool(deviceRef, p, 0);
            }
            for (auto p : fullPools)
            {
                vkResetDescriptorPool(deviceRef, p, 0);
                readyPools.push_back(p);
            }
            fullPools.clear();
        }
        void destroy()
        {
            for (auto p : readyPools)
            {
                vkDestroyDescriptorPool(deviceRef, p, nullptr);
            }
            readyPools.clear();
            for (auto p : fullPools)
            {
                vkDestroyDescriptorPool(deviceRef, p, nullptr);
            }
            fullPools.clear();
        }

        VkDescriptorSet allocate(VkDescriptorSetLayout layout, void *pNext = nullptr)
        {
            VkDescriptorPool poolToUse = get_pool();

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.pNext = pNext;
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = poolToUse;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;

            VkDescriptorSet ds;
            VkResult result = vkAllocateDescriptorSets(deviceRef, &allocInfo, &ds);

            if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
            {
                fullPools.push_back(poolToUse);
                poolToUse = get_pool();
                allocInfo.descriptorPool = poolToUse;
                result = vkAllocateDescriptorSets(deviceRef, &allocInfo, &ds);

                if (result != VK_SUCCESS)
                    assert(false);
            }

            readyPools.push_back(poolToUse);
            return ds;
        }

    private:
        VkDevice deviceRef;
        std::vector<DescriptorPoolPart> parts;
        std::vector<VkDescriptorPool> fullPools;
        std::vector<VkDescriptorPool> readyPools;
        const float scale;
        uint32_t setPerPool_nextAlloc;
        const uint32_t growLimits;

    private:
        VkDescriptorPool create_pool(VkDevice device, uint32_t maxSetCount, std::vector<DescriptorPoolPart> poolParts)
        {
            std::vector<VkDescriptorPoolSize> poolSizes;
            for (auto part : poolParts)
            {
                auto poolSize = VkDescriptorPoolSize{
                    .type = part.type,
                    .descriptorCount = uint32_t(part.ratio * maxSetCount),
                };
                poolSizes.push_back(poolSize);
            }

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = 0;
            poolInfo.maxSets = maxSetCount;
            poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();

            VkDescriptorPool newPool;
            vkCreateDescriptorPool(device, &poolInfo, nullptr, &newPool);
            return newPool;
        }

        VkDescriptorPool get_pool()
        {
            VkDescriptorPool newPool;
            if (readyPools.size() != 0)
            {
                newPool = readyPools.back();
                readyPools.pop_back();
            }
            else
            {
                newPool = create_pool(deviceRef, setPerPool_nextAlloc, parts);

                setPerPool_nextAlloc *= scale;
                if (setPerPool_nextAlloc > growLimits)
                    setPerPool_nextAlloc = growLimits;
            }
            return newPool;
        }
    };

    struct DescriptorSetLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        void add(uint32_t bindingPoint, VkDescriptorType type)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = bindingPoint;
            binding.descriptorCount = 1;
            binding.descriptorType = type;
            bindings.push_back(binding);
        }

        void clear()
        {
            bindings.clear();
        }

        VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void *pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0)
        {
            assert(!bindings.empty());

            for (auto &b : bindings)
            {
                b.stageFlags |= shaderStages;
            }

            VkDescriptorSetLayoutCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.pNext = pNext;
            info.pBindings = bindings.data();
            info.bindingCount = (uint32_t)bindings.size();
            info.flags = flags;

            VkDescriptorSetLayout set;
            VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));
            return set;
        }
    };

    struct DescriptorWriter
    {
    private:
        std::deque<VkDescriptorImageInfo> imageInfos;
        std::deque<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkWriteDescriptorSet> writes;
    public:
        void push_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
        {
            VkDescriptorImageInfo &info = imageInfos.emplace_back(VkDescriptorImageInfo{
                .sampler = sampler,
                .imageView = image,
                .imageLayout = layout});
    
            VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    
            write.dstBinding = binding;
            write.dstSet = VK_NULL_HANDLE; // left empty for now until we need to write it
            write.descriptorCount = 1;
            write.descriptorType = type;
            write.pImageInfo = &info;
    
            writes.push_back(write);
        }

        void push_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
        {
            VkDescriptorBufferInfo &info = bufferInfos.emplace_back(VkDescriptorBufferInfo{
                .buffer = buffer,
                .offset = offset,
                .range = size});

            VkWriteDescriptorSet write = {};           
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstBinding = binding;
            write.dstSet = VK_NULL_HANDLE;
            write.descriptorType = type;
            write.descriptorCount = 1;
            write.pBufferInfo = &info;

            writes.push_back(write);
        }

        void clear()
        {
            imageInfos.clear();
            writes.clear();
            bufferInfos.clear();
        }

        void write(VkDevice device, VkDescriptorSet set)
        {
            for (VkWriteDescriptorSet &w : writes)
            {
                w.dstSet = set;
            }

            auto d = device; auto da = writes.data();
            vkUpdateDescriptorSets(d, (uint32_t)writes.size(), da, 0, nullptr);
            //vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
        }
    };
}