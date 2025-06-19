#include "device.hpp"
#include "VkBootstrap.h"
#include <GLFW/glfw3.h>

namespace unknown::renderer::vulkan
{
    void Device::Init(const DeviceInitInfo & info)
    {
        VK_CHECK(volkInitialize());

        vkb::InstanceBuilder builder;

        // make the vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name("Example Vulkan Application")
                            .request_validation_layers(true)
                            .use_default_debug_messenger()
                            .require_api_version(1, 3, 0)
                            .build();

        vkb::Instance vkb_inst = inst_ret.value();

        volkLoadInstance(vkb_inst);

        vkData.instance = vkb_inst.instance;
        debugMessenger = vkb_inst.debug_messenger;

        VK_CHECK(glfwCreateWindowSurface(vkData.instance, (GLFWwindow *)(info.windowPtr), NULL, &vkData.surface));

        // features
	    VkPhysicalDeviceMeshShaderFeaturesEXT featuresMesh = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
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

        std::vector<const char*> extensions = {
            "VK_EXT_mesh_shader",
            "VK_KHR_shader_draw_parameters"
        };

        vkb::PhysicalDeviceSelector selector{vkb_inst};
        //vkb::PhysicalDevice physicalDevice = 
        auto res = selector
                                                 .set_minimum_version(1, 3)
                                                 .add_required_extension_features(featuresMesh)
                                                 .set_required_features_14(features14)
                                                 .set_required_features_13(features13)
                                                 .set_required_features_12(features12)
                                                 .set_required_features(features)
                                                 .add_required_extensions(extensions)
                                                 .set_surface(vkData.surface)
                                                 .select();

        // create the final vulkan device
        vkb::DeviceBuilder deviceBuilder{res.value()};
        vkb::Device vkbDevice = deviceBuilder.build().value();

        volkLoadDevice(vkbDevice);
        //Get the VkDevice handle used in the rest of a vulkan application
        vkData.device = vkbDevice.device;
        vkData.physicalDevice = res.value().physical_device;
        properties = res.value().properties;

        vkData.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        vkData.graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        vkData.transferQueue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
        vkData.transferQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
        if(vkData.transferQueueFamily!=vkData.graphicsQueueFamily)
            vkData.hasTransferQueue = true;
        else
            vkData.useTransferQueue = false;

        vkData.computeQueue = vkbDevice.get_queue(vkb::QueueType::compute).value();
        vkData.computeQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::compute).value();
        if(vkData.computeQueueFamily!=vkData.graphicsQueueFamily)
            vkData.hasComputeQueue = true;
        else
            vkData.useComputeQueue = false;

        INFO_LOG("[Queue:Family]:-[graphic:{}]-[transfer:{}]-[compute:{}]-",
            vkData.graphicsQueueFamily,
            vkData.transferQueueFamily,
            vkData.computeQueueFamily);

        //mark
        bInit = true;
    }

    void Device::ShutDown()
    {
        if(!bInit)
            return;
        vkDestroySurfaceKHR(vkData.instance, vkData.surface, nullptr);
        vkDestroyDevice(vkData.device, nullptr);
        vkb::destroy_debug_utils_messenger(vkData.instance, debugMessenger);
        vkDestroyInstance(vkData.instance, nullptr);
    }
}