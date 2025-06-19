#include "simpleVK.hpp"
#include "debug/log.hpp"
#include "expVkDefines.hpp"
#include "expVkHelper.hpp"

#include <iostream>
#include <vector>

#include "vulkan_renderer/buffer.hpp"
#include "vulkan_renderer/device.hpp"
#include "vulkan_renderer/resource.hpp"

namespace unknown::exp {
// vulkan initialization
void _init(unknown::exp::VulkanEngineData &data) {
  // renderer::vulkan::DeviceInitInfo deviceInfo;
  // deviceInfo.windowPtr = data.info.glfwWptr;

  // renderer::vulkan::Device device;
  // device.Init(deviceInfo);
  //
  VK_CHECK(volkInitialize());

  vkb::InstanceBuilder builder;

  // make the vulkan instance, with basic debug features
  auto inst_ret = builder.set_app_name("Example Vulkan Application")
                      .request_validation_layers(true)
                      .use_default_debug_messenger()
                      .require_api_version(1, 4, 0)
                      .build();

  vkb::Instance vkb_inst = inst_ret.value();

  volkLoadInstance(vkb_inst);

  data.instance = vkb_inst.instance;
  data.debugMessenger = vkb_inst.debug_messenger;

  VK_CHECK(glfwCreateWindowSurface(
      data.instance, (GLFWwindow *)(data.info.glfwWptr), NULL, &data.surface));

  // features
  VkPhysicalDeviceMeshShaderFeaturesEXT featuresMesh = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
  featuresMesh.taskShader = true;
  featuresMesh.meshShader = true;

  //  vulkan 1.4 features
  VkPhysicalDeviceVulkan14Features features14{};
  features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
  features14.maintenance5 = true;
  features14.maintenance6 = true;
  features14.pushDescriptor = true;

  //  vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features13{};
  features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  features13.dynamicRendering = true;
  features13.synchronization2 = true;
  features13.maintenance4 = true;

  // vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12{};
  features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  // vulkan features
  VkPhysicalDeviceFeatures features{};
  features.multiDrawIndirect = true;

  std::vector<const char *> extensions = {"VK_EXT_mesh_shader",
                                          "VK_KHR_shader_draw_parameters"};

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  // vkb::PhysicalDevice physicalDevice =
  auto res = selector.set_minimum_version(1, 4)
                 .add_required_extension_features(featuresMesh)
                 .set_required_features_14(features14)
                 .set_required_features_13(features13)
                 .set_required_features_12(features12)
                 .set_required_features(features)
                 .add_required_extensions(extensions)
                 .set_surface(data.surface)
                 .select();
  //                                         .value();
  // vkb::PhysicalDevice physicalDevice = res.value();
  if (!res.has_value())
    std::cout << res.full_error().type.message() << std::endl;

  // create the final vulkan device
  vkb::DeviceBuilder deviceBuilder{res.value()};
  vkb::Device vkbDevice = deviceBuilder.build().value();

  volkLoadDevice(vkbDevice);
  // Get the VkDevice handle used in the rest of a vulkan application
  data.device = vkbDevice.device;
  data.gpu = res.value().physical_device;

  data.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  data.graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  //> vma_init
  // initialize the memory allocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = data.gpu;
  allocatorInfo.device = data.device;
  allocatorInfo.instance = data.instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  VmaVulkanFunctions vmaFunctions = {};
  vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
  allocatorInfo.pVulkanFunctions = &vmaFunctions;
  vmaCreateAllocator(&allocatorInfo, &data.allocator);

  data.resourceManager.Init(data.gpu, data.device, data.instance);

  // mark
  data.baseCleanUpQueue.push([&]() { vmaDestroyAllocator(data.allocator); });

  data.baseCleanUpQueue.push([&]() { data.resourceManager.Destroy(); });
  //< vma_init

  // default
  _create_swapchain(data);
  _create_commands(data);
  _create_sync(data);
  _create_descriptor(data);
  _create_pipelines(data);

  {
    u32 instanceCount = 1000u;
    std::vector<InstanceData> instance;
    instance.resize(instanceCount);

    for (u32 i = 0; i < instanceCount; ++i) {
      InstanceData &idata = instance[i];
      std::uniform_real_distribution<float> dis_float(-10.0, 10.0);
      float x = dis_float(data.rEngine);
      float y = dis_float(data.rEngine);
      float z = dis_float(data.rEngine);

      std::uniform_real_distribution<float> dis_float_scale(1.0, 2.0);
      float scale = dis_float_scale(data.rEngine);
      idata.position = Vec3f(x, y, z);
      idata.scale = scale;
    }

    u64 bufferSize = instance.size() * sizeof(InstanceData);

    renderer::vulkan::BufferDesc bInfo;
    bInfo.size = bufferSize;
    bInfo.bufferUsage.transfer_src();
    bInfo.memoryUsage.cpu();
    renderer::vulkan::ResourceHandle instanceBufferStagingHandle =
        data.resourceManager.CreateBuffer(bInfo);
    renderer::vulkan::Buffer instanceBufferStaging =
        data.resourceManager.GetBuffer(instanceBufferStagingHandle);

    void *dataMap;
    vmaMapMemory(data.allocator, instanceBufferStaging.allocation, &dataMap);
    // void *dataMap = instanceBufferStaging.allocation->GetMappedData();
    memcpy(dataMap, instance.data(), bufferSize);
    vmaUnmapMemory(data.allocator, instanceBufferStaging.allocation);

    // destroy_buffer(data, instanceBufferStaging);
    data.resourceManager.DestroyBuffer(instanceBufferStagingHandle);
  }
}

void _draw(unknown::exp::VulkanEngineData &data, const ViewData &view) {
  VulkanFrameData &frame = data.frames[data.currentFrame];
  VK_CHECK(
      vkWaitForFences(data.device, 1, &frame.renderFence, true, 1000000000));
  frame.frameResetQueue.execute();
  frame.descriptorSetAllocator.clear();

  uint32_t swapchainImageIndex;
  VkResult res = vkAcquireNextImageKHR(data.device, data.swapchain, 1000000000,
                                       frame.swapchainSemaphore, nullptr,
                                       &swapchainImageIndex);
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    data.isResizeRequested = true;
    return;
  }

  // data.drawExtent.height = std::min(data.swapchainExtent.height,
  // data.mainImage.extent.height) * data.renderScale; data.drawExtent.width =
  // std::min(data.swapchainExtent.width, data.mainImage.extent.width) *
  // data.renderScale;
  data.drawExtent.height =
      (data.swapchainExtent.height < data.mainImage.extent.height
           ? data.swapchainExtent.height
           : data.mainImage.extent.height) *
      data.renderScale;
  data.drawExtent.width =
      (data.swapchainExtent.width < data.mainImage.extent.width
           ? data.swapchainExtent.width
           : data.mainImage.extent.width) *
      data.renderScale;

  VK_CHECK(vkResetFences(data.device, 1,
                         &data.frames[data.currentFrame].renderFence));
  VK_CHECK(vkResetCommandBuffer(
      data.frames[data.currentFrame].mainCommandBuffer, 0));
  VkCommandBuffer cmd = data.frames[data.currentFrame].mainCommandBuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = _default_cmd_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
  utils::_transition_image(cmd, data.mainImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_GENERAL);

  // draw background
  {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                      sDefaultDrawData.backgroundComputePipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            sDefaultDrawData.backgroundComputeLayout, 0, 1,
                            &sDefaultDrawData.drawBackgroundDescriptorSet, 0,
                            nullptr);
    vkCmdDispatch(cmd, std::ceil(data.drawExtent.width / 16.f),
                  std::ceil(data.drawExtent.height / 16.f), 1);
  }

  utils::_transition_image(cmd, data.mainImage.image, VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  utils::_transition_image(cmd, data.depthImage.image,
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  // draw geo
  {
    VkRenderingAttachmentInfo colorAttachment = _default_attachment_info(
        data.mainImage.view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = _default_depth_attachment_info(
        data.depthImage.view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = _default_rendering_info(
        data.drawExtent, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderInfo);

    {
      // set dynamic viewport and scissor
      VkViewport viewport = {};
      viewport.x = 0;
      viewport.y = 0;
      viewport.width = data.drawExtent.width;
      viewport.height = data.drawExtent.height;
      viewport.minDepth = 0.f;
      viewport.maxDepth = 1.f;

      vkCmdSetViewport(cmd, 0, 1, &viewport);

      VkRect2D scissor = {};
      scissor.offset.x = 0;
      scissor.offset.y = 0;
      scissor.extent.width = data.drawExtent.width;
      scissor.extent.height = data.drawExtent.height;

      vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    {

      // AllocatedBuffer taskUboBuffer =
      // create_buffer(data,sizeof(TaskUniform),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      // VMA_MEMORY_USAGE_CPU_TO_GPU);
      // data.frames[data.currentFrame].frameResetQueue.push([&data,taskUboBuffer]()
      // { destroy_buffer(data,taskUboBuffer); });

      renderer::vulkan::BufferDesc bInfo;
      bInfo.size = sizeof(TaskUniform);
      bInfo.bufferUsage.uniform();
      bInfo.memoryUsage.cpu_gpu();
      renderer::vulkan::ResourceHandle taskUboBufferHandle =
          data.resourceManager.CreateBuffer(bInfo);
      renderer::vulkan::Buffer taskUboBuffer =
          data.resourceManager.GetBuffer(taskUboBufferHandle);

      renderer::vulkan::ResourceManager &rManager = data.resourceManager;

      data.frames[data.currentFrame].frameResetQueue.push(
          [&rManager, taskUboBufferHandle]() {
            rManager.DestroyBuffer(taskUboBufferHandle);
          });

      // write buffer
      TaskUniform newTaskUniform = {};
      newTaskUniform.view = view.view;
      newTaskUniform.proj = view.proj;
      newTaskUniform.view_proj = view.vp;

      void *taskUniformData; // = (TaskUniform
                             // *)taskUboBuffer.allocation->GetMappedData();
      vmaMapMemory(data.allocator, taskUboBuffer.allocation, &taskUniformData);
      //*taskUniformData = newTaskUniform;
      memcpy(taskUniformData, &newTaskUniform, sizeof(TaskUniform));
      vmaUnmapMemory(data.allocator, taskUboBuffer.allocation);
      // DescriptorSet
      VkDescriptorSet taskSet =
          data.frames[data.currentFrame].descriptorSetAllocator.allocate(
              sDefaultDrawData.taskShaderDataDescriptorSetLayout);
      DescriptorWriter writer;
      writer.push_buffer(0, taskUboBuffer.buffer, sizeof(TaskUniform), 0,
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
      writer.write(data.device, taskSet);

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        sDefaultDrawData.meshShaderPipeline);
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              sDefaultDrawData.meshShaderPipelineLayout, 0, 1,
                              &taskSet, 0, nullptr);
      u32 N = 8;
      u32 num_workgroups_x = N;
      u32 num_workgroups_y = N;
      u32 num_workgroups_z = 1;

      vkCmdDrawMeshTasksEXT(cmd, num_workgroups_x, num_workgroups_y,
                            num_workgroups_z);
    }

    vkCmdEndRendering(cmd);
  }

  utils::_transition_image(cmd, data.mainImage.image,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  utils::_transition_image(cmd, data.swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  utils::_copy_image_to_image(cmd, data.mainImage.image,
                              data.swapchainImages[swapchainImageIndex],
                              data.drawExtent, data.swapchainExtent);

  utils::_transition_image(cmd, data.swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  // draw ui

  utils::_transition_image(cmd, data.swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  VK_CHECK(vkEndCommandBuffer(cmd));

  // submit
  VkCommandBufferSubmitInfo cmdInfo = _default_cmd_buffer_submit_info(cmd);
  VkSemaphoreSubmitInfo waitInfo = _default_semaphore_submit_info(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      data.frames[data.currentFrame].swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = _default_semaphore_submit_info(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
      data.frames[data.currentFrame].renderSemaphore);

  VkSubmitInfo2 submit = _default_submit_info(&cmdInfo, &signalInfo, &waitInfo);

  VK_CHECK(vkQueueSubmit2(data.graphicsQueue, 1, &submit,
                          data.frames[data.currentFrame].renderFence));
                          
  VkPresentInfoKHR presentInfo = _default_present_info();
  presentInfo.pSwapchains = &data.swapchain;
  presentInfo.swapchainCount = 1;
  presentInfo.pWaitSemaphores = &data.frames[data.currentFrame].renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pImageIndices = &swapchainImageIndex;

  VkResult presentResult = vkQueuePresentKHR(data.graphicsQueue, &presentInfo);

  std::random_device rd;
  data.rEngine = std::mt19937(rd());
}

void _shutdown(unknown::exp::VulkanEngineData &data) {
  if (!data.isInitialized)
    assert(false);

  vkDeviceWaitIdle(data.device);

  for (size_t i = 0; i < data.frameOverlap; i++) {
    data.frames[i].cleanUpQueue.execute();
    data.frames[i].frameResetQueue.execute();
  }

  data.resourceManager.Reset();
  data.baseCleanUpQueue.execute();
  _destroy_swapchain(data);
  vkDestroySurfaceKHR(data.instance, data.surface, nullptr);
  vkDestroyDevice(data.device, nullptr);
  vkb::destroy_debug_utils_messenger(data.instance, data.debugMessenger);
  vkDestroyInstance(data.instance, nullptr);
}

class SimpleVK::Impl {
public:
  Impl() {}
  bool init(VkInitInfo info) {
    vkData.info = info;
    _init(vkData);

    vkData.isInitialized = true;
    return true;
  }
  void draw(const ViewData &view) { _draw(vkData, view); }
  void frame() {

  vkData.currentFrame = (vkData.currentFrame + 1u) % vkData.frameOverlap;
  }
  void shutdown() { _shutdown(vkData); }
  VulkanEngineData vkData;
};

bool SimpleVK::init(VkInitInfo info) { return impl->init(info); }
void SimpleVK::shutdown() { impl->shutdown(); }
void SimpleVK::draw(const ViewData &view) { impl->draw(view); }
void SimpleVK::frame() { impl->frame(); }
SimpleVK::SimpleVK() { impl = std::make_unique<Impl>(); }
SimpleVK::~SimpleVK() {}
} // namespace unknown::exp