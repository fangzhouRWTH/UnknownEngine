#include "vulkan_renderer/shader.hpp"

#include <filesystem>
#include <fstream>

namespace unknown::renderer::vulkan {
bool ShaderManager::Create(const ShaderDesc &desc, ShaderModule & shaderModule) {
  std::ifstream file(desc.path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return false;
  }

  // find what the size of the file is by looking up the location of the cursor
  // because the cursor is at the end, it gives the size directly in bytes
  size_t fileSize = (size_t)file.tellg();

  // spirv expects the buffer to be on uint32, so make sure to reserve a int
  // vector big enough for the entire file
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  // put file cursor at beginning
  file.seekg(0);

  // load the entire file into the buffer
  file.read((char *)buffer.data(), fileSize);

  // now that the file is loaded into the buffer, we can close it
  file.close();

  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = nullptr;

  // codeSize has to be in bytes, so multply the ints in the buffer by size of
  // int to know the real size of the buffer
  createInfo.codeSize = buffer.size() * sizeof(uint32_t);
  createInfo.pCode = buffer.data();

  // check that the creation goes well.
  if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule.data) !=
      VK_SUCCESS) {
    return false;
  }

  shaderModule.type = desc.type;

  return true;
}

bool ShaderManager::GetShaderModule(const ShaderDesc &desc, ShaderModule & shaderModule)
{
    auto it = mModules.find(desc);
    if(it!=mModules.end())
    {
        shaderModule = it->second;
        return true;
    }

    if(!Create(desc,shaderModule))
        return false;

    mModules.insert({desc,shaderModule});
    return true;
}

void ShaderManager::Clear()
{
    for(auto s : mModules)
    {
        vkDestroyShaderModule(mDevice, s.second.data, nullptr);
    }
    mModules.clear();
}
} // namespace unknown::renderer::vulkan