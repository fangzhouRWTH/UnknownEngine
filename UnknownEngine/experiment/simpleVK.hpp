#pragma once

//#define VK_NO_PROTOTYPES
#include "volk.h"
#include <GLFW/glfw3.h>

#include "core/math.hpp"

#include <memory>

namespace unknown::exp
{
    struct alignas(16) InstanceData
    {
        Vec3f position;
        float scale;
    };

    struct ViewData
    {
        Mat4f view;
        Mat4f proj;
        Mat4f vp;
    };

    struct VkInitInfo
    {
        GLFWwindow* glfwWptr;
        uint32_t width;
        uint32_t height; 
    };

    class SimpleVK
    {
        public:
            bool init(VkInitInfo info);
            void shutdown();
            void draw(const ViewData & view);
            void frame();
            SimpleVK();
            ~SimpleVK();
        private:
            class Impl;
            std::unique_ptr<Impl> impl = nullptr;
    };
}