#pragma once
#include "renderer/api_interface.hpp"
#include "vulkan/vkCore.hpp"

namespace unknown::renderer::vulkan
{
    class API_Vulkan:public GraphicAPI
    {
        public:
            virtual void initialize() override {}
            virtual void shutdown() override {}
            virtual void* get_core() override
            {
                return &mCore;
            }

            virtual GPUMeshBufferHandle upload_mesh(std::span<uint32_t> indices, std::span<Vertex> vertices) override
            {
                assert(indices.size()>0&&vertices.size()>0);
                auto handle = mGPUMeshBufferTable.Create();
                *mGPUMeshBufferTable.Get(handle) = mCore.uploadMesh(indices,vertices);
                return handle;
            }

        private:
            VulkanCore mCore;
            ResourceTable<GPUMeshBufferHandle,GPUMeshBuffers> mGPUMeshBufferTable;
    };
}