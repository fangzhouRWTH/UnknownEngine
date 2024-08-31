#pragma once
#include <memory>
#include <unordered_map>
#include "keys.hpp"
#include "core/bit.hpp"
#include "core/math.hpp"

namespace unknown
{
    class Framework;
}

namespace unknown::input
{
    enum class CursorMode
    {
        Default,
        Hidden,
        Hidden_Lock,
        ENUM_MAX,
    };

    struct CursorPosition
    {
        Vec2f _previous = {0.f, 0.f};
        Vec2f _current = {0.f, 0.f};
        Vec2f Delta() { return _current - _previous; }
    };

    struct KeyEvents
    {
        BitMask<Key> pressed;
        BitMask<Key> released;
        BitMask<Key> repeat;
        BitMask<Key> down;
    };

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();
        bool Initialize();
        void PostFrame();

        void SetKey(Key key, Keystate state);
        void SetCursor(Vec2f position);
        void SetCursorMode(CursorMode mode);
        CursorMode GetCursorMode() const ;
        KeyEvents GetKeysEvents() const ;
        CursorPosition GetCursorData() const ;

    private:
        Keystore mKeystore;
        CursorMode mCursorMode;
        CursorPosition mCursor;
    };
}