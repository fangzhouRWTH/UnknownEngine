#pragma once
#include "renderer/api_interface.hpp"
#include "renderer/vulkan/vkCore.hpp"

namespace unknown::renderer::vulkan {
class API_Vulkan : public GraphicAPI {
public:
  virtual void initialize(const RendererInitInfo &info) override;
  virtual void shutdown() override;
  virtual void try_resize(u32 width, u32 height) override;
  virtual void *get_core() override { return &mCore; }

  virtual void push_render_objects(std::span<RenderObject> objects) override;

  virtual void frame(u32 width, u32 height, EngineContext context) override;

  virtual GPUMeshBufferHandle upload_mesh(std::span<uint32_t> indices,
                                          std::span<Vertex> vertices) override;
  
private:
  //temp
  void remove_mesh(GPUMeshBuffers buffer);

private:
  VulkanCore mCore;
  ResourceTable<GPUMeshBufferHandle, GPUMeshBuffers> mGPUMeshBufferTable;
};
} // namespace unknown::renderer::vulkan