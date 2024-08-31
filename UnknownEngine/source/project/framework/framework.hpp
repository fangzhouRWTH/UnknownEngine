#pragma once
#include "platform/type.hpp"
#include "core/input.hpp"
#include "core/bit.hpp"

#include <memory>
#include <string>

namespace unknown
{
    class Framework;

    struct FrameworkInfo
    {
        input::CursorMode cursorMode = input::CursorMode::Hidden;
        std::string name = "";
        u32 width = 800u;
        u32 height = 600u;
    };

    class FrameworkManager
    {
    public:
        static void Initialize(const FrameworkInfo &info);
        static bool RunningMain();
        static void TerminateMain();
        static void Update();
        static void PostUpdate();

        static input::KeyEvents GetKeysEvents();
        static input::CursorPosition GetCursorPosition();
        static void GetWindowSize(u32 & width, u32 & height);

        static void* GetWindowRawPointer();
    private:
        static bool bInitialized;
        static std::unique_ptr<Framework> sMainFramework;
    };
}