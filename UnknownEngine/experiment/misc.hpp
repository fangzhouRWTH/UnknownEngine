#pragma once

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/input.hpp"

namespace unknown::exp
{
    class Window
    {
    public:
        static void init(uint32_t width, uint32_t height);
        static Window& get();
    private:
        static Window sWindowInstance;
        static bool bInitialized;

    public:
        bool shouldClose();
        void update();
        void postUpdate();
        void terminate();

        void* getRaw() {return spWindow;}

    //private:
        GLFWwindow * spWindow = nullptr;
        uint32_t width = 800u;
        uint32_t height = 600u;

        input::KeyEvents mKeyEvents;
        input::CursorPosition mCursorPosition;
        input::InputManager mInputManager;
    };
}