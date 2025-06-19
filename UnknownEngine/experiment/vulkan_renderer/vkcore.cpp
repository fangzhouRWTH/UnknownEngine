#include "vkcore.hpp"

#include "math.h"

#include "vulkan_renderer/common.hpp"

#include "vulkan_renderer/descriptor.hpp"
#include "vulkan_renderer/device.hpp"
#include "vulkan_renderer/frame.hpp"
#include "vulkan_renderer/pipeline.hpp"
#include "vulkan_renderer/resource.hpp"
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/swapchain.hpp"
#include "vulkan_renderer/sync.hpp"
#include "vulkan_renderer/types.hpp"

#include "vulkan_renderer/vulkanContext.hpp"

#include "vulkan_renderer/memAllocation.hpp"

#include "utils/randomGenerator.hpp"

#include <memory>

namespace unknown::renderer::vulkan {

u32 instanceCount = 18432;
std::vector<TestInstanceData> instanceData;

class Core::Impl {
private:
  Frames<2u> mFrames;

  VulkanContext mContext;

public:
  Impl() {}
  bool init(InitDesc desc);
  void preframe();
  void frame();
  void postframe();
  void shutdown();

  TempUpdateData tempData;

private:
};

bool Core::Impl::init(InitDesc desc) {
  mContext.viewport.width = desc.width;
  mContext.viewport.height = desc.height;

  mContext.device = std::make_shared<Device>();
  DeviceInitInfo dInfo;
  dInfo.windowPtr = desc.glfwWptr;
  mContext.device->Init(dInfo);

  auto gpu = mContext.device->GetGPU();
  auto dvc = mContext.device->GetDevice();
  auto ist = mContext.device->GetInstance();
  auto srf = mContext.device->GetSurface();

  ResourceManagerDesc reDesc;
  reDesc.uniformAlignment = mContext.device->GetUniformBufferAlignment();
  reDesc.storageAlignment = mContext.device->GetStorageBufferAlignment();
  reDesc.vkData = mContext.device->GetCoreData();
  reDesc.vkData.useTransferQueue = true;
  mContext.resourceManager = std::make_shared<ResourceManager>();
  mContext.resourceManager->Init(reDesc);

  mContext.commandBufferManager = std::make_shared<CommandBufferManager<FRAME_OVERLAP>>();
  mContext.commandBufferManager->Init(mContext.device->GetCoreData());

  UniformDesc uDesc;
  uDesc.name = "scene_data";
  uDesc.size = sizeof(TestSceneData);
  uDesc.type = UniformType::FrameUniform;
  // uDesc.update = UniformUpdate::Frame;
  mContext.resourceManager->RegisterUniform(uDesc);

  UniformDesc uDesc2;
  u32 maxInstanceCount = 1024 * 128;
  uDesc2.name = "instance_data";
  uDesc2.size = sizeof(TestInstanceData) * maxInstanceCount;
  uDesc2.type = UniformType::FrameStorage;
  // uDesc2.update = UniformUpdate::Frame;
  mContext.resourceManager->RegisterUniform(uDesc2);

  mContext.synchronizationManager = std::make_shared<SynchronizationManager>();
  SyncInitDesc syncDesc;
  syncDesc.device = dvc;
  mContext.synchronizationManager->Init(syncDesc);

  mContext.pipelineManager = std::make_shared<PipelineManager>();
  PipelineManagerInitDesc pmDesc;
  pmDesc.device = dvc;
  mContext.pipelineManager->Init(pmDesc);

  mContext.globalDescriptorSetAllocator =
      std::make_shared<DescriptorSetAllocator>();
  DescriptorSetAllocatorInitDesc mainDescAllocatorDesc;
  mainDescAllocatorDesc.device = mContext.device->GetDevice();
  mainDescAllocatorDesc.storage_image(1).uniform_buffer(1);
  mContext.globalDescriptorSetAllocator->Init(mainDescAllocatorDesc);

  mFrames.Init(mContext);

  {
    DescriptorSetLayoutBuilder taskdsl;
    taskdsl.stage_task().stage_mesh().unifrom_buffer(0).storage_buffer(1);
    ;

    auto dslKey = mContext.pipelineManager->CreateDescriptorSetLayout(taskdsl);

    bool defaultMode = false;
    ShaderDesc taskS;

    if (defaultMode) {
      taskS.path =
          "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
          "UnknownEngine/"
          "experiment/expShader/mesh_shader_culling.task.spv";
    } else {
      taskS.path =
          "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
          "UnknownEngine/"
          "experiment/expShader/mesh_test.task.spv";
    }
    taskS.type = ShaderType::Task;

    ShaderDesc meshS;
    if (defaultMode) {
      meshS.path =
          "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
          "UnknownEngine/"
          "experiment/expShader/mesh_shader_culling.mesh.spv";
    } else {
      meshS.path =
          "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
          "UnknownEngine/"
          "experiment/expShader/mesh_test.mesh.spv";
    }
    meshS.type = ShaderType::Mesh;

    ShaderDesc fragS;
    fragS.path =
        "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
        "UnknownEngine/"
        "experiment/expShader/mesh_shader_culling.frag.spv";
    fragS.type = ShaderType::Fragment;

    assert(mContext.pipelineManager->CreateShader(taskS));
    assert(mContext.pipelineManager->CreateShader(meshS));
    assert(mContext.pipelineManager->CreateShader(fragS));

    PipelineLayoutDesc plDesc;
    plDesc.setLayouts.layouts.push_back(dslKey);

    PipelineDesc meshPipelineDesc;
    meshPipelineDesc.layoutDesc = plDesc;
    meshPipelineDesc.type = PipelineType::Mesh;
    meshPipelineDesc.shaders.insert({ShaderType::Task, taskS});
    meshPipelineDesc.shaders.insert({ShaderType::Mesh, meshS});
    meshPipelineDesc.shaders.insert({ShaderType::Fragment, fragS});
    mContext.pipelineManager->GetPipeline(meshPipelineDesc);

    mContext.pipelineManager->ClearShaderCache();

    RenderObject meshObject;
    meshObject.pipelineDesc = meshPipelineDesc;
    meshObject.bindings.push_back(
        BindingDesc{.set = 0, .bindingPoint = 0, .name = "scene_data"});
    mContext.renderObjects.push_back(meshObject);
  }

  instanceData.resize(instanceCount);

  SimpleBallPositionGenerator ballGen(50.0);
  ColorGenerator colorGen;
  for (auto i = 0; i < instanceCount; i++) {
    TestInstanceData iData;
    iData.transform = Mat4f::Identity();
    auto position = ballGen.Generate();
    // Vec4f position = {1.0,0.0,0.0,1.0};
    auto color = colorGen.Generate();
    // Vec4f color = {1.0,0.0,0.0,1.0};
    iData.transform.col(3) =
        Vec4f{position.x(), position.y(), position.z(), 1.0};
    iData.color = Vec4f(color.x(), color.y(), color.z(), 1.0);
    instanceData[i] = iData;
  }

  return true;
}

void Core::Impl::preframe() { mFrames.Wait(mContext); }

void Core::Impl::frame() {
  // resource?

  TestSceneData sceneData;
  float t_s = (std::sin(tempData.time) + 1.0) / 2.0;
  float t_c = (std::cos(tempData.time) + 1.0) / 2.0;

  sceneData.color1 = Vec4f(t_s, 0.0, 0.0, 1.0);
  sceneData.color2 = Vec4f(t_c, 0.0, 0.0, 1.0);
  sceneData.color3 = Vec4f(0.0, 0.0, t_s, 1.0);
  sceneData.color4 = Vec4f(0.0, 0.0, t_c, 1.0);

  sceneData.view = tempData.view;
  sceneData.proj = tempData.proj;
  sceneData.view_proj = tempData.view_proj;

  sceneData.instanceCount = instanceCount;

  mContext.resourceManager->StageUniform("scene_data", (byte *)&sceneData,
                                         sizeof(sceneData));
  mContext.resourceManager->StageUniform(
      "instance_data", (byte *)instanceData.data(),
      instanceData.size() * sizeof(TestInstanceData));

  // resize?
  mFrames.Reset(mContext);

  mFrames.Begin(mContext);

  mFrames.Render(mContext);

  mFrames.End(mContext);

  mFrames.Present(mContext);
}

void Core::Impl::postframe() { 
  mFrames.Advance(); 
  mContext.commandBufferManager->Advance();
}

void Core::Impl::shutdown() {
  vkDeviceWaitIdle(mContext.device->GetDevice());

  mFrames.Destroy();

  mContext.pipelineManager->Destroy();
  mContext.globalDescriptorSetAllocator->Destroy();
  mContext.synchronizationManager->Destroy();
  mContext.commandBufferManager->Destroy();
  mContext.resourceManager->Destroy();
  mContext.device->ShutDown();
}

bool Core::init(InitDesc desc) { return impl->init(desc); }
void Core::shutdown() { return impl->shutdown(); }

void Core::updateData(const TempUpdateData &data) { impl->tempData = data; }

void Core::preframe() { return impl->preframe(); }

void Core::frame() { return impl->frame(); }

void Core::postframe() { return impl->postframe(); }

Core::Core() : impl(std::make_unique<Core::Impl>()) {}

Core::~Core() {}
} // namespace unknown::renderer::vulkan