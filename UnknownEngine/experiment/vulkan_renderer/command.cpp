#include "vulkan_renderer/command.hpp"
#include "command.hpp"
#include "vulkan_renderer/command.hpp"
#include "vulkan_renderer/defines.hpp"


namespace unknown::renderer::vulkan {
void CommandManager::Init(const CommandManagerInitDesc &desc) {
  if (bInit)
    return;

  mDevice = desc.device;

  VkCommandPoolCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.queueFamilyIndex = desc.queueFamilyIndex;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VK_CHECK(vkCreateCommandPool(desc.device, &info, nullptr, &mPool));

  bInit = true;
}

void CommandManager::Reset() {}

void CommandManager::Destroy() {
  if (!bInit)
    return;
  vkDestroyCommandPool(mDevice, mPool, nullptr);
  bInit = false;
}

CommandBuffer CommandManager::Create() {
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.commandPool = mPool;
  allocInfo.commandBufferCount = 1;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  CommandBuffer buffer;
  VK_CHECK(vkAllocateCommandBuffers(mDevice, &allocInfo, &buffer.buffer));
  mBuffers.push_back(buffer);

  return buffer;
}

void CommandBufferPool::Init(const VkDevice &device, const u32 &queueIndex) {
  assert(!bInit);
  bInit = true;

  mDevice = device;
  VkCommandPoolCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.queueFamilyIndex = queueIndex;
  //info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VK_CHECK(vkCreateCommandPool(mDevice, &info, nullptr, &mPool));
}

void CommandBufferPool::Reset() 
{
  for(auto c : mUsedCommandBuffers)
    mFreeCommandBuffers.push(c);
  
  mUsedCommandBuffers.clear();
  VK_CHECK(vkResetCommandPool(mDevice,mPool,0));
}

void CommandBufferPool::Destroy() {
  if (!bInit)
    return;
  vkDestroyCommandPool(mDevice, mPool, nullptr);
  bInit = false;
}

CommandBuffer CommandBufferPool::GetCommandBuffer()
{
  if(mFreeCommandBuffers.size()<=0)
    create();

  auto cb = mFreeCommandBuffers.front();
  mFreeCommandBuffers.pop();
  mUsedCommandBuffers.push_back(cb);
  return cb;
}

void CommandBufferPool::create() 
{ 
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.commandPool = mPool;
  allocInfo.commandBufferCount = 1;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  CommandBuffer buffer;
  VK_CHECK(vkAllocateCommandBuffers(mDevice, &allocInfo, &buffer.buffer));
  
  mFreeCommandBuffers.push(buffer);

  return;
}

} // namespace unknown::renderer::vulkan
