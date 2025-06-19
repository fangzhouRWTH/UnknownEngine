#include "misc.hpp"

#include <cassert>
#include <iostream>

namespace unknown::exp
{
    Window Window::sWindowInstance = Window();
    bool Window::bInitialized = false;

    void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    void key_input_callback(GLFWwindow *window, int key, int scancode, int action,
                            int mods);
    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    void mouse_button_callback(GLFWwindow *window, int button, int action,
                               int mods);

    void Window::init(uint32_t width, uint32_t height)
    {
        if (bInitialized)
            return;

        glfwInit();
        // Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        auto pW = glfwCreateWindow(width, height, "VulkanExp", NULL, NULL);

        if (pW == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }

        sWindowInstance.spWindow = pW;
        sWindowInstance.width = width;
        sWindowInstance.height = height;

        glfwMakeContextCurrent(pW);
        glfwSetWindowUserPointer(pW, &sWindowInstance);

        glfwSetFramebufferSizeCallback(sWindowInstance.spWindow,
                                       framebuffer_size_callback);
        glfwSetCursorPosCallback(sWindowInstance.spWindow, cursor_position_callback);
        glfwSetKeyCallback(sWindowInstance.spWindow, key_input_callback);
        glfwSetMouseButtonCallback(sWindowInstance.spWindow, mouse_button_callback);

        bInitialized = true;
    }
    Window &Window::get()
    {
        assert(bInitialized);
        assert(sWindowInstance.spWindow);

        return sWindowInstance;
    }
    bool Window::shouldClose() { return glfwWindowShouldClose(spWindow); }
    void Window::update()
    {
        glfwSwapBuffers(spWindow);
        glfwPollEvents();
        mKeyEvents = mInputManager.GetKeysEvents();
        mCursorPosition = mInputManager.GetCursorData();
        if (mKeyEvents.down.IsSet(input::Key::ESCAPE))
        {
            glfwSetWindowShouldClose(spWindow, true);
        }

        if (mKeyEvents.pressed.IsSet(input::Key::C))
        {
            // mImpl->_switch_cursor_mode();
        }

        if (mKeyEvents.pressed.IsSet(input::Key::MOUSE_BUTTON_RIGHT))
        {
            //mInputManager.SetCursorMode(input::CursorMode::Hidden_Lock);
        }

        if (mKeyEvents.released.IsSet(input::Key::MOUSE_BUTTON_RIGHT))
        {
            //mInputManager.SetCursorMode(input::CursorMode::Default);
        }
        return;
    }
    void Window::postUpdate()
    {
        mInputManager.PostFrame();
    }
    void Window::terminate() { glfwTerminate(); }
} // namespace unknown::exp


/// callback
namespace unknown::exp
{
    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        Window *ptr = static_cast<Window *>(glfwGetWindowUserPointer(window));
        ptr->width = u32(width);
        ptr->height = u32(height);
    };

    void key_input_callback(GLFWwindow *window, int key, int scancode, int action,
                            int mods)
    {
        Window *ptr = static_cast<Window *>(glfwGetWindowUserPointer(window));
        input::Keystate state;
        switch (action)
        {
        case GLFW_PRESS:
            state.state.Set(input::Keystate::State::Pressed);
            break;
        case GLFW_RELEASE:
            state.state.Set(input::Keystate::State::Released);
            break;
        case GLFW_REPEAT:
            state.state.Set(input::Keystate::State::Repeat);
            break;
        default:
            break;
        }
        switch (key)
        {
        case GLFW_KEY_SPACE:
            ptr->mInputManager.SetKey(input::Key::SPACE, state);
            break;
        case GLFW_KEY_ESCAPE:
            ptr->mInputManager.SetKey(input::Key::ESCAPE, state);
            break;
        case GLFW_KEY_W:
            ptr->mInputManager.SetKey(input::Key::W, state);
            break;
        case GLFW_KEY_A:
            ptr->mInputManager.SetKey(input::Key::A, state);
            break;
        case GLFW_KEY_S:
            ptr->mInputManager.SetKey(input::Key::S, state);
            break;
        case GLFW_KEY_D:
            ptr->mInputManager.SetKey(input::Key::D, state);
            break;
        case GLFW_KEY_C:
            ptr->mInputManager.SetKey(input::Key::C, state);
            break;
            // case GLFW_MOUSE_BUTTON_RIGHT:
            //     input::InputManager::SetKey(framework,
            //     input::Key::MOUSE_BUTTON_RIGHT, state); break;
            // case GLFW_MOUSE_BUTTON_MIDDLE:
            //     input::InputManager::SetKey(framework,
            //     input::Key::MOUSE_BUTTON_MIDDLE, state); break;
            // case GLFW_MOUSE_BUTTON_LEFT:
            //     input::InputManager::SetKey(framework, input::Key::MOUSE_BUTTON_LEFT,
            //     state); break;

        default:
            break;
        }
    };

    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
    {
        Window *ptr = static_cast<Window *>(glfwGetWindowUserPointer(window));
        Vec2f newPosition = {float(xpos), float(ypos)};
        ptr->mInputManager.SetCursor(newPosition);
    };
    void mouse_button_callback(GLFWwindow *window, int button, int action,
                               int mods)
    {
        Window *ptr = static_cast<Window *>(glfwGetWindowUserPointer(window));
        input::Keystate state;
        switch (action)
        {
        case GLFW_PRESS:
            state.state.Set(input::Keystate::State::Pressed);
            break;
        case GLFW_RELEASE:
            state.state.Set(input::Keystate::State::Released);
            break;
        case GLFW_REPEAT:
            state.state.Set(input::Keystate::State::Repeat);
            break;
        default:
            break;
        }

        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            ptr->mInputManager.SetKey(input::Key::MOUSE_BUTTON_RIGHT, state);
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            ptr->mInputManager.SetKey(input::Key::MOUSE_BUTTON_MIDDLE, state);
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            ptr->mInputManager.SetKey(input::Key::MOUSE_BUTTON_LEFT, state);
            break;
        default:
            break;
        }
    };
}
