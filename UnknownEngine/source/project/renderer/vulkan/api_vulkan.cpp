#include "api_vulkan.hpp"

namespace unknown::renderer::vulkan {
void API_Vulkan::initialize(const RendererInitInfo &info) {
  assert(info.windowPtr);
  mCore.init(info.windowPtr);
}

void API_Vulkan::shutdown() 
{ 
    auto buffers = mGPUMeshBufferTable.GetAll();
    for(auto buffer:buffers)
    {
        mCore.removeMesh(*buffer);
    }
    mGPUMeshBufferTable.Clear();
    mCore.cleanup();   
}
void API_Vulkan::try_resize(u32 width, u32 height) {
  mCore.test_try_resize_swapchain(width, height);
}
void API_Vulkan::push_render_objects(std::span<RenderObject> objects) {

  for (const auto &o : objects) {
    auto meshBuffer = mGPUMeshBufferTable.Get(o.meshBufferHandle);
    // todo
    // meshData->meshBufferHandle;
    // meshInfo.meshBuffer = meshData->buffers;
    // mMeshBufferBank.meshInfos.insert({meshInfo.meshDataHandle, meshInfo});
    // sceneMesh->meshGpuInfoHash = meshHandle;

    VulkanRenderObject def;
    def.indexCount = o.indicesCount;
    def.firstIndex = 0u;
    def.indexBuffer = meshBuffer->indexBuffer.buffer;
    def.material = &mCore.defaultData;
    //todo
    def.materialKey = o.materialKey;

    def.transform = o.transform;

    def.vertexBufferAddress = meshBuffer->vertexBufferAddress;

    mCore.push_dynamic_render_object(def);
  }
}
void API_Vulkan::frame(u32 width, u32 height, EngineContext context) {
  // temp
  mCore.draw(width, height, context);
}
GPUMeshBufferHandle API_Vulkan::upload_mesh(std::span<uint32_t> indices,
                                            std::span<Vertex> vertices) {
  assert(indices.size() > 0 && vertices.size() > 0);
  auto handle = mGPUMeshBufferTable.Create();
  *mGPUMeshBufferTable.Get(handle) = mCore.uploadMesh(indices, vertices);
  return handle;
}
void API_Vulkan::test_init_indirect_draw(u64 indicesCount)
{
  mCore.init_indirect_draw(indicesCount);
}
void API_Vulkan::remove_mesh(GPUMeshBuffers buffer)
{
    mCore.removeMesh(buffer);
}
} // namespace unknown::renderer::vulkan