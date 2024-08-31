#include "simpleGUI.hpp"
#include "renderer/vulkan/vkCore.hpp"
#include "renderer/vulkan/vkUtils.hpp"
#include "renderer/vulkan/vkInitializer.hpp"
#include "renderer/vulkan/vkTypes.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort

#include <vector>

namespace unknown::renderer::ui
{
    void IMGUI_VULKAN_GLFW::InitializeVulkan(vulkan::VulkanCore *vkCore, void *windowPtr, bool install_callbacks)
    {
        // 1: create descriptor pool for IMGUI
        //  the size of the pool is very oversize, but it's copied from imgui demo
        //  itself.
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VkDescriptorPool imguiPool;
        VK_CHECK(vkCreateDescriptorPool(vkCore->_device, &pool_info, nullptr, &imguiPool));

        // 2: initialize imgui library

        // this initializes the core structures of imgui
        ImGui::CreateContext();

        // this initializes imgui for SDL
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow *)(windowPtr), install_callbacks);

        // this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = vkCore->_instance;
        init_info.PhysicalDevice = vkCore->_chosenGPU;
        init_info.Device = vkCore->_device;
        init_info.Queue = vkCore->_graphicsQueue;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.UseDynamicRendering = true;

        // dynamic rendering parameters for imgui to use
        init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vkCore->_swapchainImageFormat;

        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);

        ImGui_ImplVulkan_CreateFontsTexture();

        // add the destroy the imgui created structures
        vkCore->_mainDeletionQueue.push_function([=]()
                                                 {
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(vkCore->_device, imguiPool, nullptr); });
    }

    void IMGUI_VULKAN_GLFW::ImplGLFWProcessEvent()
    {
        // Imgui_implglfw_
    }

    void IMGUI_VULKAN_GLFW::NewFrame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void IMGUI_VULKAN_GLFW::VkDrawImgui(vulkan::VulkanCore* vkCore, VkCommandBuffer cmd, VkImageView targetImageView)
    {
        VkRenderingAttachmentInfo colorAttachment = vulkan::create::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = vulkan::create::rendering_info(vkCore->_swapchainExtent, &colorAttachment, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRendering(cmd);
    }

    void IMGUI_VULKAN_GLFW::Render()
    {
        // Rendering
        ImGui::Render();
    }

    void IMGUI_VULKAN_GLFW::ShutDown(vulkan::VulkanCore *vkCore)
    {
        //auto err = vkDeviceWaitIdle(vkCore->_device);
        //VK_CHECK(err);
        // ImGui_ImplVulkan_Shutdown();
        // ImGui_ImplGlfw_Shutdown();
        // ImGui::DestroyContext();
    }
}