#include "vulkan_renderer/pipeline.hpp"
#include "pipeline.hpp"

namespace unknown::renderer::vulkan {

void PipelineManager::Init(const PipelineManagerInitDesc &desc) {
  mDevice = desc.device;
  mShaderManager.Init(desc.device);
  DescriptorSetLayoutManagerInitDesc dlDesc;
  dlDesc.device = desc.device;
  mDescriptorSetLayoutManager.Init(dlDesc);
}

void PipelineManager::Destroy() {
  mDescriptorSetLayoutManager.Destroy();
  mShaderManager.Clear();
  for (auto l : mLayouts) {
    vkDestroyPipelineLayout(mDevice, l.second.data, nullptr);
  }
  for (auto p : mPipelines) {
    vkDestroyPipeline(mDevice, p.second.data, nullptr);
  }
}

void PipelineManager::ClearShaderCache() { mShaderManager.Clear(); }

DescriptorSetLayoutKey
PipelineManager::CreateDescriptorSetLayout(DescriptorSetLayoutBuilder builder) {
  return mDescriptorSetLayoutManager.Create(builder);
}

DescriptorSetLayout
PipelineManager::GetDescriptorSetLayout(DescriptorSetLayoutKey key) {
  return mDescriptorSetLayoutManager.Get(key);
}

PipelineLayout
PipelineManager::GetPipelineLayout(const PipelineLayoutDesc &desc) {
  auto it = mLayouts.find(desc);
  if (it != mLayouts.end())
    return it->second;

  auto layout = createPipelineLayout(desc);

  mLayouts.insert({desc, layout});

  return layout;
}

Pipeline PipelineManager::GetPipeline(const PipelineDesc &desc) {
  auto it = mPipelines.find(desc);
  if (it != mPipelines.end())
    return it->second;

  Pipeline newPipeline = createPipeline(desc);
  mPipelines.insert({desc, newPipeline});
  return newPipeline;
}

bool PipelineManager::CreateShader(const ShaderDesc &desc) {
  ShaderModule sModule;
  bool res = mShaderManager.GetShaderModule(desc, sModule);
  return res;
}

PipelineLayout
PipelineManager::createPipelineLayout(const PipelineLayoutDesc &desc) {

  std::vector<VkDescriptorSetLayout> setLayouts;
  setLayouts.reserve(desc.setLayouts.layouts.size());
  for (auto l : desc.setLayouts.layouts) {
    auto layout = mDescriptorSetLayoutManager.Get(l);
    setLayouts.push_back(layout.data);
  }

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.pNext = nullptr;

  //INFO_LOG("Create Set number {}",setLayouts.size());

  layoutInfo.pSetLayouts = setLayouts.data();
  layoutInfo.setLayoutCount = setLayouts.size();

  if (desc.pushConstants.ranges.size() != 0) {
    layoutInfo.pPushConstantRanges = desc.pushConstants.ranges.data();
    layoutInfo.pushConstantRangeCount = desc.pushConstants.ranges.size();
  }

  PipelineLayout layout;
  VK_CHECK(vkCreatePipelineLayout(mDevice, &layoutInfo, nullptr, &layout.data));

  return layout;
}

Pipeline PipelineManager::createPipeline(const PipelineDesc &desc) {
  Pipeline pipeline;
  switch (desc.type) {
  case PipelineType::Compute: {
    pipeline.data = createComputePipeline(desc);
    break;
  }
  case PipelineType::Mesh: {
    pipeline.data = createMeshPipeline(desc);
    break;
  }
  case PipelineType::Normal: {
    pipeline.data = createGraphicPipeline(desc);
    break;
  }
  default:
    assert(false);
  }

  return pipeline;
}

VkPipeline PipelineManager::createGraphicPipeline(const PipelineDesc &desc) {
  auto plLayout = GetPipelineLayout(desc.layoutDesc);

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

  VkShaderModule vert;
  VkShaderModule frag;

  for (auto s : desc.shaders) {
    ShaderModule shader;
    auto bRes = mShaderManager.GetShaderModule(s.second, shader);
    assert(bRes);
    switch (shader.type) {
    case ShaderType::Vertex:
      vert = shader.data;
      break;
    case ShaderType::Fragment:
      frag = shader.data;
      break;
    default:
      assert(false);
    }
  }

  VkPipelineShaderStageCreateInfo vertInfo = {};
  vertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertInfo.pNext = nullptr;
  vertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertInfo.module = vert;
  vertInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragInfo = {};
  fragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragInfo.pNext = nullptr;
  fragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragInfo.module = frag;
  fragInfo.pName = "main";

  shaderStages.push_back(vertInfo);
  shaderStages.push_back(fragInfo);
  //

  //Mark Vertex
  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = 0;
  vertexInput.vertexAttributeDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  //Vertex

  VkPipelineRasterizationStateCreateInfo rasterizationState = {};
  rasterizationState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.lineWidth = 1.f;

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
  multisampling.sampleShadingEnable = VK_FALSE;
  // multisampling defaulted to no multisampling (1 sample per pixel)
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  // no alpha to coverage either
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE; 

  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicInfo = {};
  dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicInfo.pDynamicStates = &state[0];
  dynamicInfo.dynamicStateCount = 2;

  VkFormat imagef = VK_FORMAT_R16G16B16A16_SFLOAT;

  VkPipelineRenderingCreateInfo renderInfo = {};
  renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT; // mark
  renderInfo.colorAttachmentCount = 1;
  renderInfo.pColorAttachmentFormats = &(imagef); // mark

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pRasterizationState = &rasterizationState;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pDynamicState = &dynamicInfo;
  pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineInfo.pStages = shaderStages.data();

  pipelineInfo.layout = plLayout.data;
  pipelineInfo.pNext = &renderInfo;

  VkPipeline pl;
  if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &pl) != VK_SUCCESS) {
    INFO_PRINT("failed to create pipeline");
  }

  return pl;
}

VkPipeline PipelineManager::createComputePipeline(const PipelineDesc &desc) {
  auto plLayout = GetPipelineLayout(desc.layoutDesc);
  auto itShader = desc.shaders.find(ShaderType::Compute);

  ShaderModule shaderModule;
  assert(itShader != desc.shaders.end());
  assert(mShaderManager.GetShaderModule(itShader->second, shaderModule));
  assert(shaderModule.type == ShaderType::Compute);

  VkPipelineShaderStageCreateInfo stageInfo = {};
  stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageInfo.pNext = nullptr;
  stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stageInfo.module = shaderModule.data;
  stageInfo.pName = "main";

  VkComputePipelineCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.layout = plLayout.data;
  createInfo.stage = stageInfo;

  VkPipeline pipeline;
  VK_CHECK(vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo,
                                    nullptr, &pipeline));

  return pipeline;
}
VkPipeline PipelineManager::createMeshPipeline(const PipelineDesc &desc) {
  auto plLayout = GetPipelineLayout(desc.layoutDesc);

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

  VkShaderModule task;
  VkShaderModule mesh;
  VkShaderModule frag;

  for (auto s : desc.shaders) {
    ShaderModule shader;
    auto bRes = mShaderManager.GetShaderModule(s.second, shader);
    assert(bRes);
    switch (shader.type) {
    case ShaderType::Task:
      task = shader.data;
      break;
    case ShaderType::Mesh:
      mesh = shader.data;
      break;
    case ShaderType::Fragment:
      frag = shader.data;
      break;
    default:
      assert(false);
    }
  }

  VkPipelineShaderStageCreateInfo taskInfo = {};
  taskInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  taskInfo.pNext = nullptr;
  taskInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
  taskInfo.module = task;
  taskInfo.pName = "main";

  VkPipelineShaderStageCreateInfo meshInfo = {};
  meshInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  meshInfo.pNext = nullptr;
  meshInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
  meshInfo.module = mesh;
  meshInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragInfo = {};
  fragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragInfo.pNext = nullptr;
  fragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragInfo.module = frag;
  fragInfo.pName = "main";

  shaderStages.push_back(taskInfo);
  shaderStages.push_back(meshInfo);
  shaderStages.push_back(fragInfo);
  //
  VkPipelineRasterizationStateCreateInfo rasterizationState = {};
  rasterizationState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.lineWidth = 1.f;

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
  multisampling.sampleShadingEnable = VK_FALSE;
  // multisampling defaulted to no multisampling (1 sample per pixel)
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  // no alpha to coverage either
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE; 

  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicInfo = {};
  dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicInfo.pDynamicStates = &state[0];
  dynamicInfo.dynamicStateCount = 2;

  VkFormat imagef = VK_FORMAT_R16G16B16A16_SFLOAT;

  VkPipelineRenderingCreateInfo renderInfo = {};
  renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT; // mark
  renderInfo.colorAttachmentCount = 1;
  renderInfo.pColorAttachmentFormats = &(imagef); // mark

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

  pipelineInfo.layout = plLayout.data;
  pipelineInfo.pNext = &renderInfo;

  VkPipeline pl;
  if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &pl) != VK_SUCCESS) {
    INFO_PRINT("failed to create pipeline");
  }

  return pl;
}
} // namespace unknown::renderer::vulkan