#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace unknown::renderer::vulkan
{
    class VulkanCore;
}

namespace unknown::renderer::ui
{
    class IMGUI_VULKAN_GLFW
    {
        public:
        //SimpleIMGUI(){}
        //~SimpleIMGUI(){}
        static void InitializeVulkan(vulkan::VulkanCore* vkCore, void* windowPtr, bool install_callbacks);
        static void ImplGLFWProcessEvent();
        static void NewFrame();
        static void VkDrawImgui(vulkan::VulkanCore* vkCore, VkCommandBuffer cmd, VkImageView targetImageView);
        static void Render();
        static void ShutDown(vulkan::VulkanCore* vkCore);
    };
}