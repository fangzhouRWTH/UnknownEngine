#include "input.hpp"
#include <cassert>
#include <framework/framework.hpp>

namespace unknown::input
{
    InputManager::~InputManager()
    {
    }

    InputManager::InputManager()
    {
    }

    bool InputManager::Initialize()
    {
        return true;
    }

    void InputManager::SetKey(Key key, Keystate state)
    {
        auto previous = mKeystore.keys[static_cast<u32>(key)];

        if (
            (previous.state.IsSet(input::Keystate::State::Pressed) ||
             state.state.IsSet(input::Keystate::State::Pressed) ||
             previous.state.IsSet(input::Keystate::State::Down)) &&
            (!state.state.IsSet(input::Keystate::State::Released)))
        {
            state.state.Set(input::Keystate::State::Down);
        }

        mKeystore.keys[static_cast<u32>(key)] = state;
    }

    void InputManager::SetCursor(Vec2f position)
    {
        mCursor._current = position;
        if (mCursorMode == CursorMode::Default)
            mCursor._previous = mCursor._current;
    }

    void InputManager::SetCursorMode(CursorMode mode)
    {
        mCursorMode = mode;
    }

    CursorMode InputManager::GetCursorMode() const
    {
        return mCursorMode;
    }

    void InputManager::PostFrame()
    {
        mCursor._previous = mCursor._current;

        for (auto &ks : mKeystore.keys)
            ks.state.Clear(input::Keystate::State::Released);
    }

    KeyEvents InputManager::GetKeysEvents() const
    {
        KeyEvents events;
        for (u64 i = 0; i < u64(Key::ENUM_MAX); i++)
        {
            auto k = mKeystore.keys[i];
            if (k.state.IsSet(Keystate::State::Pressed))
                events.pressed.Set(Key(i));
            if (k.state.IsSet(Keystate::State::Released))
                events.released.Set(Key(i));
            if (k.state.IsSet(Keystate::State::Repeat))
                events.repeat.Set(Key(i));
            if (k.state.IsSet(Keystate::State::Down))
                events.down.Set(Key(i));
        }
        return events;
    }

    CursorPosition InputManager::GetCursorData() const
    {
        return mCursor;
    }
}