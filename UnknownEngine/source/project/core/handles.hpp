#pragma once
#include "../platform/type.hpp"
#include <cassert>

// #define _HANDLEDECL(Name)                                                     \
//     struct Name                                                               \
//     {                                                                         \
//     private:                                                                  \
//         static u32 _current;                                                  \
//         u32 _this = kInvalidHandle;                                           \
//                                                                               \
//     public:                                                                   \
//         const static u32 kInvalidHandle = 0xFFFFFFFF;                         \
//         static u32 Current() { return _current; }                             \
//         void Set(u32 h) { _this = h; }                                        \
//         u32 Get() { return _this; }                                           \
//         bool IsValid() { return _this != kInvalidHandle; }                    \
//         static Name CreateHandle()                                            \
//         {                                                                     \
//             Name _h;                                                          \
//             assert(_current < kInvalidHandle);                                \
//             _h.Set(_current == kInvalidHandle ? kInvalidHandle : _current++); \
//             return _h;                                                        \
//         }                                                                     \
//     }

// #define _HANDLEIMPL(Name) u32 Name::_current = 0u

template <typename HandleType>
struct Handle
{
private:
    static u32 _current;
    u32 _this = kInvalidHandle;

public:
    const static u32 kInvalidHandle = 0xFFFFFFFF;
    static u32 Current() { return _current; }
    void Set(u32 h) { _this = h; }
    u32 Get() { return _this; }
    bool IsValid() { return _this != kInvalidHandle; }

protected:
    static HandleType CreateHandle()
    {
        HandleType _h;
        assert(_current < kInvalidHandle);
        _h.Set(_current == kInvalidHandle ? kInvalidHandle : _current++);
        return _h;
    }
};

template <typename HandleType>
u32 Handle<HandleType>::_current = 0u;