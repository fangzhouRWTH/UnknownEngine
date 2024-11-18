#pragma once
#include "renderer/api_interface.hpp"
#include "vulkan/vkCore.hpp"

namespace unknown::renderer::vulkan
{
    class API_Vulkan:public GraphicAPI
    {
        public:
            virtual void initialize(const RendererInitInfo & info) override 
            {
                assert(info.windowPtr);
                mCore.init(info.windowPtr);
            }
            virtual void shutdown() override 
            {
                mCore.cleanup();
            }
            virtual void try_resize(u32 width, u32 height) override
            {
                mCore.test_try_resize_swapchain(width,height);
            }
            virtual void* get_core() override
            {
                return &mCore;
            }

            virtual void frame(u32 width, u32 height, EngineContext context) override
            {
                //temp
                mCore.draw(width,height,context);
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