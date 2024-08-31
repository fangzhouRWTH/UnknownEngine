#include "framework.hpp"
#include "renderer/renderer.hpp"

#ifdef API_OPENGL
#include <glad/glad.h>
#elif defined(API_VULKAN)
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <iostream>

#include "core/input.hpp"
#include "debug/log.hpp"

// #include "renderer/gui/simpleGUI.hpp"

namespace unknown
{
    class Framework
    {
    public:
        class Impl;
        Framework(std::string name, const u32 &width, const u32 &height);
        ~Framework();

        bool ShouldClose();
        // void ProcessInput();
        void Terminate();
        void Update();
        void PostUpdate();
        void SetCursorMode(input::CursorMode mode);
        input::CursorMode GetCursorMode();

        input::KeyEvents GetKeysEvents();
        input::CursorPosition GetCursorPosition();
        void GetWindowSize(u32 &width, u32 &height);

        void cacheEvents();
        std::unique_ptr<Impl> mImpl;
        input::KeyEvents mKeyEvents;
        input::CursorPosition mCursorPosition;
        input::InputManager mInputManager;
    };

    void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    void key_input_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    bool FrameworkManager::bInitialized = false;
    std::unique_ptr<Framework> FrameworkManager::sMainFramework;

    void FrameworkManager::Initialize(const FrameworkInfo &info)
    {
        if (bInitialized)
            return;

        glfwInit();
#ifdef API_OPENGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif defined(API_VULKAN)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#endif

        sMainFramework = std::make_unique<Framework>(info.name, info.width, info.height);
        sMainFramework->SetCursorMode(info.cursorMode);

#ifdef API_OPENGL
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
#endif
        unknown::renderer::GraphicBackend::Initialize();
    }

    void FrameworkManager::TerminateMain()
    {
        return sMainFramework->Terminate();
    }

    void FrameworkManager::Update()
    {
        // sMainFramework->ProcessInput();
        sMainFramework->Update();
    }

    void FrameworkManager::PostUpdate()
    {
        sMainFramework->PostUpdate();
    }

    input::KeyEvents FrameworkManager::GetKeysEvents()
    {
        return sMainFramework->GetKeysEvents();
    }

    input::CursorPosition FrameworkManager::GetCursorPosition()
    {
        return sMainFramework->GetCursorPosition();
    }

    void FrameworkManager::GetWindowSize(u32 &width, u32 &height)
    {
        sMainFramework->GetWindowSize(width, height);
    }

    bool FrameworkManager::RunningMain()
    {
        return !sMainFramework->ShouldClose();
    }

    class Framework::Impl
    {
    public:
        Impl(std::string name, u32 width, u32 height, Framework *framework)
        {
            pWindow = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

            if (pWindow == NULL)
            {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
            }

            glfwMakeContextCurrent(pWindow);

            pFramework = framework;

            glfwSetWindowUserPointer(pWindow, this);

            glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
            glfwSetCursorPosCallback(pWindow, cursor_position_callback);
            glfwSetKeyCallback(pWindow, key_input_callback);
            glfwSetMouseButtonCallback(pWindow, mouse_button_callback);

            mWidth = width;
            mHeight = height;
        }

        bool _should_close()
        {
            return glfwWindowShouldClose(pWindow);
        }

        void _set_to_close()
        {
            glfwSetWindowShouldClose(pWindow, true);
        }

        void _terminate()
        {
            glfwTerminate();
        }

        void _update()
        {
            glfwSwapBuffers(pWindow);
            glfwPollEvents();
        }

        void _switch_cursor_mode(input::CursorMode mode)
        {
            _set_cursor_mode(mode);
        }

        void _set_cursor_mode(input::CursorMode mode)
        {
            switch (mode)
            {
            case input::CursorMode::Default:
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            case input::CursorMode::Hidden:
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                break;
            case input::CursorMode::Hidden_Lock:
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            default:
                break;
            }
        }

        Framework *pFramework;
        u32 mWidth;
        u32 mHeight;
        GLFWwindow *pWindow;
    };

    void *FrameworkManager::GetWindowRawPointer()
    {
        return (void *)sMainFramework->mImpl->pWindow;
    }

    bool Framework::ShouldClose()
    {
        return mImpl->_should_close();
    }

    void Framework::Terminate()
    {
        return mImpl->_terminate();
    }

    void Framework::Update()
    {
        mImpl->_update();
        cacheEvents();
        if (mKeyEvents.down.IsSet(input::Key::ESCAPE))
        {
            mImpl->_set_to_close();
        }

        if (mKeyEvents.pressed.IsSet(input::Key::C))
        {
            // mImpl->_switch_cursor_mode();
        }

        if (mKeyEvents.pressed.IsSet(input::Key::MOUSE_BUTTON_RIGHT))
        {
            mInputManager.SetCursorMode(input::CursorMode::Hidden_Lock);
            mImpl->_switch_cursor_mode(input::CursorMode::Hidden_Lock);
        }

        if (mKeyEvents.released.IsSet(input::Key::MOUSE_BUTTON_RIGHT))
        {
            mInputManager.SetCursorMode(input::CursorMode::Default);
            mImpl->_switch_cursor_mode(input::CursorMode::Default);
        }
        return;
    }

    void Framework::PostUpdate()
    {
        mInputManager.PostFrame();
    }

    void Framework::SetCursorMode(input::CursorMode mode)
    {
        mInputManager.SetCursorMode(mode);
        mImpl->_set_cursor_mode(mode);
    }

    input::CursorMode Framework::GetCursorMode()
    {
        return mInputManager.GetCursorMode();
    }

    input::KeyEvents Framework::GetKeysEvents()
    {
        return mKeyEvents;
    }

    input::CursorPosition Framework::GetCursorPosition()
    {
        return mCursorPosition;
    }

    void Framework::GetWindowSize(u32 &width, u32 &height)
    {
        width = mImpl->mWidth;
        height = mImpl->mHeight;
    }

    void Framework::cacheEvents()
    {
        mKeyEvents = mInputManager.GetKeysEvents();
        mCursorPosition = mInputManager.GetCursorData();
        // if(mCursorPosition.Delta().x()!=0.f||mCursorPosition.Delta().y()!=0.f)
        //     INFO_LOG("Cursor:: dx: {}   dy: {}",mCursorPosition.Delta().x(),mCursorPosition.Delta().y());
    }

    Framework::Framework(std::string name, const u32 &width, const u32 &height)
    {
        mImpl = std::make_unique<Impl>(name.c_str(), width, height, this);
    };

    Framework::~Framework(){};

    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        // #ifdef API_OPENGL
        // glViewport(0, 0, width, height);
        Framework::Impl *impl = static_cast<Framework::Impl *>(glfwGetWindowUserPointer(window));
        impl->mWidth = u32(width);
        impl->mHeight = u32(height);
        // #endif
    }

    void key_input_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        Framework *framework = static_cast<Framework::Impl *>(glfwGetWindowUserPointer(window))->pFramework;
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
            framework->mInputManager.SetKey(input::Key::SPACE, state);
            break;
        case GLFW_KEY_ESCAPE:
            framework->mInputManager.SetKey(input::Key::ESCAPE, state);
            break;
        case GLFW_KEY_W:
            framework->mInputManager.SetKey(input::Key::W, state);
            break;
        case GLFW_KEY_A:
            framework->mInputManager.SetKey(input::Key::A, state);
            break;
        case GLFW_KEY_S:
            framework->mInputManager.SetKey(input::Key::S, state);
            break;
        case GLFW_KEY_D:
            framework->mInputManager.SetKey(input::Key::D, state);
            break;
        case GLFW_KEY_C:
            framework->mInputManager.SetKey(input::Key::C, state);
            break;
            // case GLFW_MOUSE_BUTTON_RIGHT:
            //     input::InputManager::SetKey(framework, input::Key::MOUSE_BUTTON_RIGHT, state);
            //     break;
            // case GLFW_MOUSE_BUTTON_MIDDLE:
            //     input::InputManager::SetKey(framework, input::Key::MOUSE_BUTTON_MIDDLE, state);
            //     break;
            // case GLFW_MOUSE_BUTTON_LEFT:
            //     input::InputManager::SetKey(framework, input::Key::MOUSE_BUTTON_LEFT, state);
            //     break;

        default:
            break;
        }
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {

        Framework *framework = static_cast<Framework::Impl *>(glfwGetWindowUserPointer(window))->pFramework;
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
            framework->mInputManager.SetKey(input::Key::MOUSE_BUTTON_RIGHT, state);
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            framework->mInputManager.SetKey(input::Key::MOUSE_BUTTON_MIDDLE, state);
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            framework->mInputManager.SetKey(input::Key::MOUSE_BUTTON_LEFT, state);
            break;
        default:
            break;
        }
    }

    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
    {
        Framework *framework = static_cast<Framework::Impl *>(glfwGetWindowUserPointer(window))->pFramework;
        Vec2f newPosition = {float(xpos), float(ypos)};
        framework->mInputManager.SetCursor(newPosition);
    }
}