#pragma once
#include <stdint.h>

#include <deque>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "VkBootstrap.h"
#include "expVkCreate.hpp"
#include "expVkDefines.hpp"
#include "expVkDescriptor.hpp"
#include "expVkPipeline.hpp"
#include "renderer/vulkan/sdk/vk_enum_string_helper.h"
#include "renderer/vulkan/sdk/vk_mem_alloc.h"
#include "simpleVK.hpp"
#include "vulkan_renderer/resource.hpp"

namespace unknown::exp {
struct DefaultDrawData {
  VkDescriptorSetLayout drawBackgoundDescriptorSetLayout;
  VkDescriptorSetLayout imageDescriptorSetLayout;
  VkDescriptorSetLayout environmentDataDescriptorSetLayout;

  VkDescriptorSetLayout taskShaderDataDescriptorSetLayout;

  VkDescriptorSet drawBackgroundDescriptorSet;

  VkPipelineLayout backgroundComputeLayout;
  VkPipeline backgroundComputePipeline;

  VkPipelineLayout meshShaderPipelineLayout;
  VkPipeline meshShaderPipeline;

} sDefaultDrawData;

struct IndirectDrawData {
  VkDescriptorSetLayout meshDrawDescriptorSetLayout_0;
  VkPipelineLayout meshDrawLayout_0;
  VkPipeline meshDrawPipeline_0;
} sIndirectDrawData;

struct CleanUpQueue {
  std::deque<std::function<void()>> deletors;
  void push(std::function<void()> &&function) { deletors.push_back(function); }

  void execute() {
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
      (*it)();
    }
    deletors.clear();
  }
};

struct VulkanFrameData {
  VkSemaphore swapchainSemaphore;
  VkSemaphore renderSemaphore;
  VkFence renderFence;

  VkCommandPool commandPool;
  VkCommandBuffer mainCommandBuffer;

  CleanUpQueue frameResetQueue;
  CleanUpQueue cleanUpQueue;

  // desc
  DescriptorSetAllocator descriptorSetAllocator;
};

struct VulkanImmediateData {
  VkFence fence;
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;
};

struct AllocatedImage {
  VkImage image;
  VkImageView view;
  VmaAllocation allocation;
  VkExtent3D extent;
  VkFormat format;
};

struct AllocatedBuffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo info;
};

struct VulkanEngineData {
  VkInitInfo info;

  bool isResizeRequested = false;
  bool isInitialized = false;
  uint32_t currentFrame = 0u;
  static constexpr uint32_t frameOverlap = 2u;

  float renderScale = 1.f;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;

  VkDevice device;
  VkPhysicalDevice gpu;

  VkQueue graphicsQueue;
  uint32_t graphicsQueueFamily;

  VkExtent2D drawExtent;

  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  AllocatedImage mainImage;
  AllocatedImage depthImage;

  VulkanFrameData frames[frameOverlap];
  VulkanImmediateData immediate;

  // temp
  VmaAllocator allocator;
  renderer::vulkan::ResourceManager resourceManager;

  DescriptorSetAllocator globalDescriptorSetAllocator;

  CleanUpQueue baseCleanUpQueue;

  std::mt19937 rEngine;
};

struct MeshDrawPushConstants {
  Mat4f worldMatrix;
  VkDeviceAddress vertexBuffer;
};

void _create_swapchain(VulkanEngineData &data) {
  vkb::SwapchainBuilder swapchainBuilder{data.gpu, data.device, data.surface};
  data.swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          //.use_default_format_selection()
          .set_desired_format(VkSurfaceFormatKHR{
              .format = data.swapchainImageFormat,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // use vsync present mode
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(data.info.width, data.info.height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  data.swapchainExtent = vkbSwapchain.extent;
  // store swapchain and its related images
  data.swapchain = vkbSwapchain.swapchain;
  data.swapchainImages = vkbSwapchain.get_images().value();
  data.swapchainImageViews = vkbSwapchain.get_image_views().value();

  data.mainImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  data.mainImage.extent.width = data.info.width;
  data.mainImage.extent.height = data.info.height;
  data.mainImage.extent.depth = 1u;

  VkImageUsageFlags mainImageUsages{};
  mainImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  // drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  mainImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  mainImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo mainImageInfo = _default_image_create_info(
      data.mainImage.format, mainImageUsages, data.mainImage.extent);

  VmaAllocationCreateInfo mainImageAllocInfo{};
  mainImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  mainImageAllocInfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vmaCreateImage(data.allocator, &mainImageInfo, &mainImageAllocInfo,
                          &data.mainImage.image, &data.mainImage.allocation,
                          nullptr));

  VkImageViewCreateInfo mainViewInfo = _default_imageview_create_info(
      data.mainImage.format, data.mainImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
  VK_CHECK(vkCreateImageView(data.device, &mainViewInfo, nullptr,
                             &data.mainImage.view));

  data.depthImage.format = VK_FORMAT_D32_SFLOAT;
  data.depthImage.extent.width = data.info.width;
  data.depthImage.extent.height = data.info.height;
  data.depthImage.extent.depth = 1u;

  VkImageUsageFlags depthImageUsages{};
  depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkImageCreateInfo depthImageInfo = _default_image_create_info(
      data.depthImage.format, depthImageUsages, data.depthImage.extent);
  VK_CHECK(vmaCreateImage(data.allocator, &depthImageInfo, &mainImageAllocInfo,
                          &data.depthImage.image, &data.depthImage.allocation,
                          nullptr));

  VkImageViewCreateInfo depthViewInfo = _default_imageview_create_info(
      data.depthImage.format, data.depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);
  VK_CHECK(vkCreateImageView(data.device, &depthViewInfo, nullptr,
                             &data.depthImage.view));

  data.baseCleanUpQueue.push([&]() {
    _destroy_image(data.device, data.allocator, data.mainImage.allocation,
                   data.mainImage.image, data.mainImage.view);
    _destroy_image(data.device, data.allocator, data.depthImage.allocation,
                   data.depthImage.image, data.depthImage.view);
  });
}

void _destroy_swapchain(VulkanEngineData &data) {
  vkDestroySwapchainKHR(data.device, data.swapchain, nullptr);
  size_t count = data.swapchainImageViews.size();
  for (size_t i = 0; i < count; i++) {
    vkDestroyImageView(data.device, data.swapchainImageViews[i], nullptr);
  }
}

void _create_commands(VulkanEngineData &data) {
  VkCommandPoolCreateInfo cmdPoolInfo = _default_cmd_pool_create_info(
      data.graphicsQueueFamily,
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  // frame command
  for (size_t i = 0; i < data.frameOverlap; i++) {
    VK_CHECK(vkCreateCommandPool(data.device, &cmdPoolInfo, nullptr,
                                 &data.frames[i].commandPool));
    VkCommandBufferAllocateInfo cmdAllocInfo =
        _default_cmd_buffer_allocate_info(data.frames[i].commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(data.device, &cmdAllocInfo,
                                      &data.frames[i].mainCommandBuffer));

    auto dev = data.device;
    auto cmdPool = data.frames[i].commandPool;
    data.frames[i].cleanUpQueue.push(
        [=]() { vkDestroyCommandPool(dev, cmdPool, nullptr); });
  }

  // imm command
  VK_CHECK(vkCreateCommandPool(data.device, &cmdPoolInfo, nullptr,
                               &data.immediate.commandPool));
  VkCommandBufferAllocateInfo cmdAllocInfo =
      _default_cmd_buffer_allocate_info(data.immediate.commandPool, 1);
  VK_CHECK(vkAllocateCommandBuffers(data.device, &cmdAllocInfo,
                                    &data.immediate.commandBuffer));

  auto dev = data.device;
  auto cmdPool = data.immediate.commandPool;
  data.baseCleanUpQueue.push(
      [=]() { vkDestroyCommandPool(dev, cmdPool, nullptr); });
}

void _create_sync(VulkanEngineData &data) {
  VkFenceCreateInfo fenceCreateInfo =
      _default_fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = _default_semaphore_create_info();

  for (size_t i = 0; i < data.frameOverlap; i++) {
    VK_CHECK(vkCreateFence(data.device, &fenceCreateInfo, nullptr,
                           &data.frames[i].renderFence));
    VK_CHECK(vkCreateSemaphore(data.device, &semaphoreCreateInfo, nullptr,
                               &data.frames[i].swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(data.device, &semaphoreCreateInfo, nullptr,
                               &data.frames[i].renderSemaphore));

    auto dev = data.device;
    auto fen = data.frames[i].renderFence;
    auto rSema = data.frames[i].renderSemaphore;
    auto sSema = data.frames[i].swapchainSemaphore;

    data.frames[i].cleanUpQueue.push([=]() {
      vkDestroyFence(dev, fen, nullptr);
      vkDestroySemaphore(dev, rSema, nullptr);
      vkDestroySemaphore(dev, sSema, nullptr);
    });
  }

  // imm
  VK_CHECK(vkCreateFence(data.device, &fenceCreateInfo, nullptr,
                         &data.immediate.fence));
  auto dev = data.device;
  auto fen = data.immediate.fence;
  data.baseCleanUpQueue.push([=]() { vkDestroyFence(dev, fen, nullptr); });
}

void _create_descriptor(VulkanEngineData &data) {
  {
    std::vector<DescriptorPoolPart> parts = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

    data.globalDescriptorSetAllocator.init(data.device, 10, parts);
  }

  data.baseCleanUpQueue.push(
      [&]() { data.globalDescriptorSetAllocator.destroy(); });

  {
    DescriptorSetLayoutBuilder builder;

    builder.add(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    sDefaultDrawData.drawBackgoundDescriptorSetLayout =
        builder.build(data.device, VK_SHADER_STAGE_COMPUTE_BIT);
    builder.clear();

    builder.add(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    sDefaultDrawData.imageDescriptorSetLayout =
        builder.build(data.device, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.clear();

    builder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    sDefaultDrawData.environmentDataDescriptorSetLayout = builder.build(
        data.device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.clear();

    builder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    sDefaultDrawData.taskShaderDataDescriptorSetLayout =
        builder.build(data.device, VK_SHADER_STAGE_TASK_BIT_EXT);
    builder.clear();

    sDefaultDrawData.drawBackgroundDescriptorSet =
        data.globalDescriptorSetAllocator.allocate(
            sDefaultDrawData.drawBackgoundDescriptorSetLayout);
    DescriptorWriter writer;
    writer.push_image(0, data.mainImage.view, VK_NULL_HANDLE,
                      VK_IMAGE_LAYOUT_GENERAL,
                      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.write(data.device, sDefaultDrawData.drawBackgroundDescriptorSet);

    data.baseCleanUpQueue.push([&]() {
      vkDestroyDescriptorSetLayout(
          data.device, sDefaultDrawData.drawBackgoundDescriptorSetLayout,
          nullptr);
      vkDestroyDescriptorSetLayout(
          data.device, sDefaultDrawData.imageDescriptorSetLayout, nullptr);
      vkDestroyDescriptorSetLayout(
          data.device, sDefaultDrawData.environmentDataDescriptorSetLayout,
          nullptr);
      vkDestroyDescriptorSetLayout(
          data.device, sDefaultDrawData.taskShaderDataDescriptorSetLayout,
          nullptr);
    });
  }

  {
    std::vector<DescriptorPoolPart> parts = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3.f},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3.f},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f},
        //{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    };

    for (size_t i = 0; i < data.frameOverlap; i++) {
      data.frames[i].descriptorSetAllocator.init(data.device, 1024, parts);
      data.baseCleanUpQueue.push(
          [&, i]() { data.frames[i].descriptorSetAllocator.destroy(); });
    }
  }
}

void _prepare_compute_background_pipelines(VulkanEngineData &data) {
  // Compute Pipeline
  VkPipelineLayoutCreateInfo computeLayoutInfo{};
  computeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  computeLayoutInfo.pNext = nullptr;
  computeLayoutInfo.pSetLayouts =
      &sDefaultDrawData.drawBackgoundDescriptorSetLayout;
  computeLayoutInfo.setLayoutCount = 1;
  // VkPushConstantRange

  VK_CHECK(vkCreatePipelineLayout(data.device, &computeLayoutInfo, nullptr,
                                  &sDefaultDrawData.backgroundComputeLayout));

  VkShaderModule backgroundComputeShader;
  std::string bgShaderPath = "C:/Users/franz/Downloads/Documents/Git/"
                             "UnknownEngine/UnknownEngine/UnknownEngine/"
                             "experiment/expShader/cs_background.comp.spv";
  bool bLoaded = utils::_load_shader_module(bgShaderPath.data(), data.device,
                                            backgroundComputeShader);
  if (!bLoaded)
    fmt::print("Error when building the compute shader \n");

  VkPipelineShaderStageCreateInfo stageInfo{};
  stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageInfo.pNext = nullptr;
  stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stageInfo.module = backgroundComputeShader;
  stageInfo.pName = "main";

  VkComputePipelineCreateInfo computePipelineCreateInfo{};
  computePipelineCreateInfo.sType =
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  computePipelineCreateInfo.pNext = nullptr;
  computePipelineCreateInfo.layout = sDefaultDrawData.backgroundComputeLayout;
  computePipelineCreateInfo.stage = stageInfo;

  VK_CHECK(vkCreateComputePipelines(
      data.device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
      &sDefaultDrawData.backgroundComputePipeline));

  vkDestroyShaderModule(data.device, backgroundComputeShader, nullptr);
  data.baseCleanUpQueue.push([&]() {
    vkDestroyPipelineLayout(data.device,
                            sDefaultDrawData.backgroundComputeLayout, nullptr);
    vkDestroyPipeline(data.device, sDefaultDrawData.backgroundComputePipeline,
                      nullptr);
  });
}

void _prepare_indirect_draw_data(VulkanEngineData &data) {
  VkShaderModule vertShader;
  VkShaderModule fragShader;
  std::string vertShaderPath = "C:/Users/franz/Downloads/Documents/Git/"
                               "UnknownEngine/UnknownEngine/UnknownEngine/"
                               "experiment/expShader/vs_common.vert.spv";
  std::string fragShaderPath = "C:/Users/franz/Downloads/Documents/Git/"
                               "UnknownEngine/UnknownEngine/UnknownEngine/"
                               "experiment/expShader/fs_mesh.frag.spv";
  utils::_load_shader_module(fragShaderPath.data(), data.device, fragShader);
  utils::_load_shader_module(vertShaderPath.data(), data.device, vertShader);

  VkPushConstantRange pushRange{};
  pushRange.offset = 0;
  pushRange.size = sizeof(MeshDrawPushConstants);
  pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  DescriptorSetLayoutBuilder layoutBuilder;
  layoutBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  sIndirectDrawData.meshDrawDescriptorSetLayout_0 = layoutBuilder.build(
      data.device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  VkDescriptorSetLayout layouts[] = {
      sDefaultDrawData.environmentDataDescriptorSetLayout,
      sIndirectDrawData.meshDrawDescriptorSetLayout_0};

  VkPipelineLayoutCreateInfo meshLayoutInfo =
      _default_pipeline_layout_create_info();
  meshLayoutInfo.setLayoutCount = 2;
  meshLayoutInfo.pSetLayouts = layouts;
  meshLayoutInfo.pushConstantRangeCount = 1;
  meshLayoutInfo.pPushConstantRanges = &pushRange;

  VK_CHECK(vkCreatePipelineLayout(data.device, &meshLayoutInfo, nullptr,
                                  &sIndirectDrawData.meshDrawLayout_0));

  PipelineBuilder pipelineBuilder;
  pipelineBuilder.set_shaders(vertShader, fragShader);
  pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  pipelineBuilder.set_multisampling_none();
  pipelineBuilder.disable_blending();
  pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
  pipelineBuilder.set_color_attachment_format(data.mainImage.format);
  pipelineBuilder.set_depth_format(data.depthImage.format);
  pipelineBuilder.pipelineLayout = sIndirectDrawData.meshDrawLayout_0;
  sIndirectDrawData.meshDrawPipeline_0 = pipelineBuilder.build(data.device);

  vkDestroyShaderModule(data.device, vertShader, nullptr);
  vkDestroyShaderModule(data.device, fragShader, nullptr);

  data.baseCleanUpQueue.push([&]() {
    vkDestroyDescriptorSetLayout(
        data.device, sIndirectDrawData.meshDrawDescriptorSetLayout_0, nullptr);
    vkDestroyPipelineLayout(data.device, sIndirectDrawData.meshDrawLayout_0,
                            nullptr);
    vkDestroyPipeline(data.device, sIndirectDrawData.meshDrawPipeline_0,
                      nullptr);
  });
}

void _prepare_mesh_shader_data(VulkanEngineData &data) {
  VkPipelineLayoutCreateInfo meshShaderLayoutInfo{};
  meshShaderLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  meshShaderLayoutInfo.pNext = nullptr;
  meshShaderLayoutInfo.pSetLayouts =
      &sDefaultDrawData.taskShaderDataDescriptorSetLayout;
  meshShaderLayoutInfo.setLayoutCount = 1;

  VK_CHECK(vkCreatePipelineLayout(data.device, &meshShaderLayoutInfo, nullptr,
                                  &sDefaultDrawData.meshShaderPipelineLayout));

  VkShaderModule taskShader;
  VkShaderModule meshShader;
  VkShaderModule fragShader;
  std::string taskShaderPath =
      "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
      "UnknownEngine/"
      "experiment/expShader/mesh_shader_culling.task.spv";
  std::string meshShaderPath =
      "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
      "UnknownEngine/"
      "experiment/expShader/mesh_shader_culling.mesh.spv";
  std::string fragShaderPath =
      "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/"
      "UnknownEngine/"
      "experiment/expShader/mesh_shader_culling.frag.spv";
  utils::_load_shader_module(taskShaderPath.data(), data.device, taskShader);
  utils::_load_shader_module(meshShaderPath.data(), data.device, meshShader);
  utils::_load_shader_module(fragShaderPath.data(), data.device, fragShader);
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
  shaderStages.clear();
  shaderStages.push_back(_default_pipeline_shader_stage_create_info(
      VK_SHADER_STAGE_TASK_BIT_EXT, taskShader));
  shaderStages.push_back(_default_pipeline_shader_stage_create_info(
      VK_SHADER_STAGE_MESH_BIT_EXT, meshShader));
  shaderStages.push_back(_default_pipeline_shader_stage_create_info(
      VK_SHADER_STAGE_FRAGMENT_BIT, fragShader));
  //
  VkPipelineRasterizationStateCreateInfo rasterizationState = {};
  rasterizationState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  VkPipelineColorBlendAttachmentState blend_attachment = {};
  blend_attachment.colorWriteMask = 0xf;
  blend_attachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.pNext = nullptr;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &blend_attachment;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicInfo = {};
  dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicInfo.pDynamicStates = &state[0];
  dynamicInfo.dynamicStateCount = 2;

  VkPipelineRenderingCreateInfo renderInfo = {};
  renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderInfo.depthAttachmentFormat = data.depthImage.format;
  renderInfo.colorAttachmentCount = 1;
  renderInfo.pColorAttachmentFormats = &(data.mainImage.format);

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pVertexInputState = nullptr;
  pipelineInfo.pInputAssemblyState = nullptr;
  pipelineInfo.pRasterizationState = &rasterizationState;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pDynamicState = &dynamicInfo;
  pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineInfo.pStages = shaderStages.data();

  pipelineInfo.layout = sDefaultDrawData.meshShaderPipelineLayout;
  pipelineInfo.pNext = &renderInfo;

  VkPipeline pipeline;
  if (vkCreateGraphicsPipelines(data.device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &pipeline) != VK_SUCCESS) {
    INFO_PRINT("failed to create pipeline");
  }
  sDefaultDrawData.meshShaderPipeline = pipeline;
  //

  vkDestroyShaderModule(data.device, taskShader, nullptr);
  vkDestroyShaderModule(data.device, meshShader, nullptr);
  vkDestroyShaderModule(data.device, fragShader, nullptr);

  data.baseCleanUpQueue.push([&]() {
    vkDestroyPipelineLayout(data.device,
                            sDefaultDrawData.meshShaderPipelineLayout, nullptr);
    vkDestroyPipeline(data.device, sDefaultDrawData.meshShaderPipeline,
                      nullptr);
  });
}

void _create_pipelines(VulkanEngineData &data) {
  _prepare_compute_background_pipelines(data);
  _prepare_indirect_draw_data(data);
  _prepare_mesh_shader_data(data);
}

// AllocatedBuffer create_buffer(VulkanEngineData & data, size_t allocSize,
// VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
// {
//     // allocate buffer
//     VkBufferCreateInfo bufferInfo = {.sType =
//     VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO}; bufferInfo.pNext = nullptr;
//     bufferInfo.size = allocSize;

//     bufferInfo.usage = usage;

//     VmaAllocationCreateInfo vmaallocInfo = {};
//     vmaallocInfo.usage = memoryUsage;
//     vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
//     AllocatedBuffer newBuffer;

//     // allocate the buffer
//     VK_CHECK(vmaCreateBuffer(data.resourceManager.get(), &bufferInfo,
//     &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
//                              &newBuffer.info));

//     return newBuffer;
// }

// void destroy_buffer(VulkanEngineData & data,const AllocatedBuffer &buffer)
// {
//     vmaDestroyBuffer(data.resourceManager.get(), buffer.buffer,
//     buffer.allocation);
// }
} // namespace unknown::exp